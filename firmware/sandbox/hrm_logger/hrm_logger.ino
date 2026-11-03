// hrm_logger.ino -- prise en main de NimBLE-Arduino 2.x avec une ceinture cardio
//
// Fonctionnement :
//   1. scan BLE à la recherche d'un appareil qui annonce le service 0x180D
//   2. connexion
//   3. abonnement aux notifications de la caractéristique 0x2A37
//   4. décodage du BPM, du contact capteur, de l'énergie et des intervalles RR
//   5. reconnexion automatique en cas de perte de la ceinture
//   6. affichage du BPM sur un OLED SSD1306 0,96" 128x64 (I2C)
//
// Bibliothèques : NimBLE-Arduino (h2zero) version 2.x (l'API 1.4 est différente),
//                 Adafruit SSD1306 (+ Adafruit GFX, installée avec).
// Sur ESP32-S3 en USB natif : activer « USB CDC On Boot » pour voir Serial.
//
// Câblage OLED (suivre les inscriptions du module, l'ordre GND/VCC varie) :
//   GND -> GND,  VCC -> 3V3 (pas 5 V),  SCL -> OLED_SCL,  SDA -> OLED_SDA


#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Broches I2C par défaut du core Arduino pour chaque cible
#if CONFIG_IDF_TARGET_ESP32S3
static const int OLED_SDA = 8;
static const int OLED_SCL = 9;
#else
static const int OLED_SDA = 21;
static const int OLED_SCL = 22;
#endif

static Adafruit_SSD1306 display(128, 64, &Wire, -1);
static bool displayOk = false;

// Partagé entre la tâche NimBLE (callbacks) et loop() : on n'écrit jamais
// sur l'I2C depuis un callback BLE, loop() rafraîchit l'écran.
enum LinkState : uint8_t { LINK_SCANNING, LINK_CONNECTED };
static volatile LinkState linkState = LINK_SCANNING;
static volatile uint16_t lastBpm = 0;
static volatile bool lastContact = true;
static volatile uint32_t lastHrMs = 0;

static const NimBLEUUID HR_SERVICE_UUID((uint16_t)0x180D);
static const NimBLEUUID HR_MEASUREMENT_UUID((uint16_t)0x2A37);
static const NimBLEUUID BATTERY_SERVICE_UUID((uint16_t)0x180F);
static const NimBLEUUID BATTERY_LEVEL_UUID((uint16_t)0x2A19);

// Optionnel : forcer une ceinture précise (adresse MAC affichée au scan).
// Laisser vide pour se connecter à la première ceinture trouvée.
static const char *TARGET_ADDRESS = "";

static const uint32_t SCAN_TIME_MS = 10000;

static const NimBLEAdvertisedDevice *foundDevice = nullptr;
static volatile bool doConnect = false;
static NimBLEClient *client = nullptr;

// ---------------------------------------------------------------------------
// Décodage de la caractéristique Heart Rate Measurement (0x2A37)
//
// Octet 0 = flags :
//   bit 0    : format du BPM (0 = uint8, 1 = uint16)
//   bits 1-2 : état du contact capteur (0b10 = pas de contact, 0b11 = contact)
//   bit 3    : champ "Energy Expended" présent (uint16, en kJ)
//   bit 4    : un ou plusieurs intervalles RR présents (uint16, unité 1/1024 s)
// ---------------------------------------------------------------------------
static void parseHeartRate(const uint8_t *data, size_t len) {
  if (len < 2) return;

  const uint8_t flags = data[0];
  size_t i = 1;

  uint16_t bpm;
  if (flags & 0x01) {
    if (len < 3) return;
    bpm = data[i] | (data[i + 1] << 8);
    i += 2;
  } else {
    bpm = data[i];
    i += 1;
  }

  const bool contactSupported = flags & 0x04;
  const bool contactDetected = flags & 0x02;

  lastBpm = bpm;
  lastContact = !contactSupported || contactDetected;
  lastHrMs = millis();

  int energy = -1;
  if (flags & 0x08) {
    if (i + 1 < len) energy = data[i] | (data[i + 1] << 8);
    i += 2;
  }

  Serial.printf("[%lu ms] BPM=%u", millis(), bpm);
  if (contactSupported) Serial.printf(" contact=%s", contactDetected ? "oui" : "NON");
  if (energy >= 0) Serial.printf(" energie=%d kJ", energy);

  if (flags & 0x10) {
    Serial.print(" RR(ms)=");
    bool first = true;
    while (i + 1 < len) {
      const uint16_t rrRaw = data[i] | (data[i + 1] << 8);  // en 1/1024 s
      const float rrMs = rrRaw * 1000.0f / 1024.0f;
      Serial.printf("%s%.1f", first ? "" : ",", rrMs);
      first = false;
      i += 2;
    }
  }
  Serial.println();
}

static void onHeartRateNotify(NimBLERemoteCharacteristic *chr, uint8_t *data,
                              size_t len, bool isNotify) {
  parseHeartRate(data, len);
}

// ---------------------------------------------------------------------------
// Callbacks du client (déconnexion -> on relance le scan)
// ---------------------------------------------------------------------------
class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient *c) override {
    Serial.printf("Connecté à %s\n", c->getPeerAddress().toString().c_str());
    linkState = LINK_CONNECTED;
  }
  void onDisconnect(NimBLEClient *c, int reason) override {
    Serial.printf("Déconnecté (raison %d), reprise du scan...\n", reason);
    linkState = LINK_SCANNING;
    NimBLEDevice::getScan()->start(SCAN_TIME_MS, false, true);
  }
};
static ClientCallbacks clientCallbacks;

// ---------------------------------------------------------------------------
// Callbacks du scan
// ---------------------------------------------------------------------------
class ScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice *dev) override {
    if (!dev->isAdvertisingService(HR_SERVICE_UUID)) return;

    Serial.printf("Ceinture trouvée : %s  nom=\"%s\"  RSSI=%d\n",
                  dev->getAddress().toString().c_str(),
                  dev->getName().c_str(), dev->getRSSI());

    if (strlen(TARGET_ADDRESS) > 0 &&
        strcasecmp(dev->getAddress().toString().c_str(), TARGET_ADDRESS) != 0) {
      return;
    }

    // On ne se connecte jamais depuis le callback : on arrête le scan
    // et on laisse loop() faire la connexion.
    // doConnect est positionné AVANT stop() : si stop() appelle onScanEnd
    // immédiatement, celui-ci ne doit pas relancer un scan (qui viderait les
    // résultats et rendrait foundDevice invalide).
    foundDevice = dev;
    doConnect = true;
    NimBLEDevice::getScan()->stop();
  }

  void onScanEnd(const NimBLEScanResults &results, int reason) override {
    if (!doConnect && (client == nullptr || !client->isConnected())) {
      Serial.println("Aucune ceinture trouvée, nouveau scan...");
      NimBLEDevice::getScan()->start(SCAN_TIME_MS, false, true);
    }
  }
};
static ScanCallbacks scanCallbacks;


// ---------------------------------------------------------------------------
// Connexion + abonnement
// ---------------------------------------------------------------------------
static bool connectToSensor() {
  if (client == nullptr) {
    client = NimBLEDevice::createClient();
    client->setClientCallbacks(&clientCallbacks, false);
    client->setConnectionParams(12, 12, 0, 150);  // 15 ms, timeout 1,5 s
    client->setConnectTimeout(5000);
  }

  if (!client->connect(foundDevice)) {
    Serial.println("Échec de connexion");
    return false;
  }

  NimBLERemoteService *hrService = client->getService(HR_SERVICE_UUID);
  if (hrService == nullptr) {
    Serial.println("Service 0x180D absent");
    client->disconnect();
    return false;
  }

  NimBLERemoteCharacteristic *hrChar = hrService->getCharacteristic(HR_MEASUREMENT_UUID);
  if (hrChar == nullptr || !hrChar->canNotify()) {
    Serial.println("Caractéristique 0x2A37 absente ou sans notification");
    client->disconnect();
    return false;
  }

  if (!hrChar->subscribe(true, onHeartRateNotify)) {
    Serial.println("Échec de l'abonnement aux notifications");
    client->disconnect();
    return false;
  }

  // Bonus : niveau de batterie si la ceinture l'expose
  NimBLERemoteService *batService = client->getService(BATTERY_SERVICE_UUID);
  if (batService != nullptr) {
    NimBLERemoteCharacteristic *batChar = batService->getCharacteristic(BATTERY_LEVEL_UUID);
    if (batChar != nullptr && batChar->canRead()) {
      Serial.printf("Batterie ceinture : %u %%\n", batChar->readValue<uint8_t>());
    }
  }

  Serial.println("Abonné aux mesures cardio.");
  return true;
}


// ---------------------------------------------------------------------------
// Écran OLED
// ---------------------------------------------------------------------------

// Liste les périphériques I2C présents et renvoie l'adresse de l'écran
// (0x3C ou 0x3D selon le module), 0 si absent.
static uint8_t scanI2C() {
  uint8_t oledAddr = 0;
  Serial.printf("Scan I2C (SDA=%d, SCL=%d) :", OLED_SDA, OLED_SCL);
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf(" 0x%02X", addr);
      if ((addr == 0x3C || addr == 0x3D) && oledAddr == 0) oledAddr = addr;
    }
  }
  Serial.println(oledAddr ? "" : "  -> aucun écran en 0x3C/0x3D, vérifier le câblage");
  return oledAddr;
}

static void setupDisplay() {
  Wire.begin(OLED_SDA, OLED_SCL);
  const uint8_t addr = scanI2C();
  if (addr == 0) return;
  if (!display.begin(SSD1306_SWITCHCAPVCC, addr)) {
    Serial.println("Échec d'initialisation SSD1306 (mémoire ?)");
    return;
  }
  displayOk = true;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
}

static void updateDisplay() {
  if (!displayOk) return;

  const bool connected = linkState == LINK_CONNECTED;
  const bool fresh = connected && lastHrMs != 0 && millis() - lastHrMs < 3000;

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(connected ? "Ceinture connectee" : "Recherche...");

  display.setTextSize(4);  // caractères de 24x32 px
  display.setCursor(0, 20);
  if (fresh) display.printf("%3u", lastBpm);
  else display.print("---");

  display.setTextSize(1);
  display.setCursor(80, 44);
  display.print("bpm");

  if (fresh && !lastContact) {
    display.setCursor(0, 56);
    display.print("pas de contact");
  }

  display.display();
}


void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== ESP32 - lecteur de ceinture cardio BLE ===");

  setupDisplay();

  NimBLEDevice::init("ESP32-HRM");
  NimBLEDevice::setPower(9);  // dBm, portée maximale

  NimBLEScan *scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(&scanCallbacks, false);
  scan->setActiveScan(true);   // récupère aussi le nom de l'appareil
  scan->setInterval(100);
  scan->setWindow(99);
  scan->start(SCAN_TIME_MS, false, true);
  Serial.println("Scan en cours... (mettez la ceinture, électrodes humidifiées)");
}


void loop() {
  if (doConnect) {
    doConnect = false;
    if (!connectToSensor()) {
      NimBLEDevice::getScan()->start(SCAN_TIME_MS, false, true);
    }
  }

  static uint32_t lastDrawMs = 0;
  if (millis() - lastDrawMs >= 250) {
    lastDrawMs = millis();
    updateDisplay();
  }
  delay(10);
}
