/*
  cda_logger.ino
  --------------------------------------------------------------
  Plateforme  : ESP32 (Arduino core)
  Bibliothèque: NimBLE-Arduino (h2zero) -- à installer via le
                Gestionnaire de bibliothèques Arduino IDE
  Rôle        :
    1. Lit en continu 2 capteurs de pression différentielle
       Sensirion SDP810/SDP811 (I2C) :
         - SDP810 (0x25) : PC - Patm  -> pression dynamique
         - SDP811 (0x26) : PG - PD    -> angle de lacet
    2. Se connecte en BLE (rôle "central") à :
         - un capteur de puissance  (Cycling Power Service, 0x1818)
         - un capteur de vitesse    (CSC Service,           0x1816)
    3. Logue tout sur carte SD, avec horodatage, en CSV.

  À VÉRIFIER / ADAPTER avant utilisation :
    - Les commandes I2C du SDP8xx (TRIGGER_CMD ci-dessous) sont
      reconstituées à partir de la datasheet Sensirion et de
      retours d'expérience communautaires. Confirme-les à la
      section 5.3 de la datasheet SDP8xx-Digital pour ta révision
      exacte de silicium avant de faire confiance aux mesures.
    - WHEEL_CIRCUMFERENCE_MM : mesure-la toi-même par rollout avec
      tes pneus (poids du cycliste sur le vélo), plutôt que de
      prendre une valeur de table -- l'erreur sur la circonférence
      se répercute directement sur la vitesse air et donc sur le SCx.
    - Brochage SD (SPI) et I2C ci-dessous correspond aux pins par
      défaut ESP32 (VSPI). Adapte SD_CS_PIN à ton câblage.
--------------------------------------------------------------*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <NimBLEDevice.h>

// ---------------------------------------------------------------
// Configuration matérielle
// ---------------------------------------------------------------
static const uint8_t I2C_SDA_PIN   = 21;
static const uint8_t I2C_SCL_PIN   = 22;
static const uint8_t SD_CS_PIN     = 5;     // adapte à ton câblage

static const uint8_t SDP_ADDR_DYN  = 0x25;  // SDP810-500Pa : PC - Patm
static const uint8_t SDP_ADDR_YAW  = 0x26;  // SDP811-500Pa : PG - PD

static const float WHEEL_CIRCUMFERENCE_MM = 2105.0f; // À MESURER (rollout)

static const uint32_t SAMPLE_PERIOD_MS = 50;  // 20 Hz

// ---------------------------------------------------------------
// UUIDs Bluetooth SIG standard
// ---------------------------------------------------------------
static const NimBLEUUID SVC_POWER   ((uint16_t)0x1818);
static const NimBLEUUID CHR_POWER_MEAS((uint16_t)0x2A63);
static const NimBLEUUID SVC_CSC     ((uint16_t)0x1816);
static const NimBLEUUID CHR_CSC_MEAS((uint16_t)0x2A5B);

// ---------------------------------------------------------------
// État partagé (mis à jour par les callbacks BLE, lu dans loop())
// ---------------------------------------------------------------
volatile int16_t  g_power_w        = 0;
volatile float     g_speed_kmh      = 0.0f;
volatile bool       g_power_connected = false;
volatile bool       g_speed_connected = false;

// mémoire pour le calcul de vitesse à partir du CSC
static uint32_t s_prevWheelRevs = 0;
static uint16_t s_prevWheelTime = 0; // unités de 1/1024 s
static bool     s_haveCscBaseline = false;

File g_logFile;

// =================================================================
//  Driver minimal SDP810/SDP811 (I2C)
// =================================================================

// CRC-8 Sensirion : poly 0x31, init 0xFF, pas de XOR final
uint8_t sensirionCrc8(const uint8_t *data, uint8_t len) {
  uint8_t crc = 0xFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; b++) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

// Déclenche une mesure "triggered, clock stretching enabled, sans moyennage"
bool sdpTrigger(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x37);
  Wire.write(0x2D);
  return (Wire.endTransmission() == 0);
}

// Lit 9 octets : DP(2)+CRC, Temp(2)+CRC, ScaleFactor(2)+CRC
// Retourne true si la lecture + les CRC sont valides.
bool sdpRead(uint8_t addr, float &dp_pa, float &temp_c) {
  uint8_t buf[9];
  if (Wire.requestFrom((int)addr, 9) != 9) return false;
  for (uint8_t i = 0; i < 9; i++) buf[i] = Wire.read();

  if (sensirionCrc8(&buf[0], 2) != buf[2]) return false;
  if (sensirionCrc8(&buf[3], 2) != buf[5]) return false;
  if (sensirionCrc8(&buf[6], 2) != buf[8]) return false;

  int16_t rawDp     = (int16_t)((buf[0] << 8) | buf[1]);
  int16_t rawTemp   = (int16_t)((buf[3] << 8) | buf[4]);
  int16_t rawScale  = (int16_t)((buf[6] << 8) | buf[7]);
  if (rawScale == 0) return false;

  dp_pa   = (float)rawDp / (float)rawScale;
  temp_c  = (float)rawTemp / 200.0f;
  return true;
}

// Lit une mesure complète : déclenche puis lit après un court délai.
// (Le capteur gère le clock-stretching, mais on ajoute une marge
//  pour rester robuste face aux implémentations Wire imparfaites.)
bool sdpMeasure(uint8_t addr, float &dp_pa) {
  float t;
  if (!sdpTrigger(addr)) return false;
  delay(10); // la datasheet indique quelques ms ; marge de sécurité
  return sdpRead(addr, dp_pa, t);
}

// =================================================================
//  BLE : callbacks de notification
// =================================================================

void powerNotifyCB(NimBLERemoteCharacteristic *chr, uint8_t *data, size_t len, bool isNotify) {
  // Cycling Power Measurement : Flags(2) + Instantaneous Power (sint16, W)
  // La puissance instantanée est toujours aux octets [2:3], quels
  // que soient les champs optionnels activés par les flags.
  if (len < 4) return;
  int16_t power = (int16_t)(data[2] | (data[3] << 8));
  g_power_w = power;
}

void cscNotifyCB(NimBLERemoteCharacteristic *chr, uint8_t *data, size_t len, bool isNotify) {
  // CSC Measurement : Flags(1) puis, si bit0 set :
  //   Cumulative Wheel Revolutions (uint32) + Last Wheel Event Time (uint16, 1/1024 s)
  if (len < 1) return;
  uint8_t flags = data[0];
  if (!(flags & 0x01) || len < 7) return; // pas de données roue dans ce paquet

  uint32_t wheelRevs = (uint32_t)data[1] | ((uint32_t)data[2] << 8) |
                        ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 24);
  uint16_t wheelTime = (uint16_t)data[5] | ((uint16_t)data[6] << 8);

  if (!s_haveCscBaseline) {
    s_prevWheelRevs = wheelRevs;
    s_prevWheelTime = wheelTime;
    s_haveCscBaseline = true;
    return;
  }

  uint32_t dRevs = wheelRevs - s_prevWheelRevs;      // wraparound OK (uint32)
  uint16_t dTime = wheelTime - s_prevWheelTime;      // wraparound OK (uint16)
  s_prevWheelRevs = wheelRevs;
  s_prevWheelTime = wheelTime;

  if (dRevs == 0 || dTime == 0) return; // roue à l'arrêt ou paquet dupliqué

  float dt_s = (float)dTime / 1024.0f;
  float dist_m = (float)dRevs * (WHEEL_CIRCUMFERENCE_MM / 1000.0f);
  g_speed_kmh = (dist_m / dt_s) * 3.6f;
}

// =================================================================
//  BLE : connexion à un service donné
// =================================================================

bool connectToService(const NimBLEAdvertisedDevice *device,
                       const NimBLEUUID &svcUuid,
                       const NimBLEUUID &chrUuid,
                       NimBLERemoteCharacteristic::notify_callback cb) {
  NimBLEClient *client = NimBLEDevice::createClient();
  if (!client->connect(device)) return false;

  NimBLERemoteService *svc = client->getService(svcUuid);
  if (!svc) { client->disconnect(); return false; }

  NimBLERemoteCharacteristic *chr = svc->getCharacteristic(chrUuid);
  if (!chr || !chr->canNotify()) { client->disconnect(); return false; }

  chr->subscribe(true, cb);
  return true;
}

void scanAndConnect() {
  Serial.println("Scan BLE (10 s) pour capteur de puissance + capteur de vitesse...");
  NimBLEScan *scan = NimBLEDevice::getScan();
  scan->setActiveScan(true);
  NimBLEScanResults results = scan->getResults(10 * 1000, false);

  const NimBLEAdvertisedDevice *powerDev = nullptr;
  const NimBLEAdvertisedDevice *cscDev   = nullptr;

  for (int i = 0; i < results.getCount(); i++) {
    const NimBLEAdvertisedDevice *dev = results.getDevice(i);
    if (!powerDev && dev->isAdvertisingService(SVC_POWER)) powerDev = dev;
    if (!cscDev   && dev->isAdvertisingService(SVC_CSC))   cscDev   = dev;
  }

  if (powerDev) {
    g_power_connected = connectToService(powerDev, SVC_POWER, CHR_POWER_MEAS, powerNotifyCB);
    Serial.println(g_power_connected ? "Capteur de puissance connecté."
                                      : "Échec connexion capteur de puissance.");
  } else {
    Serial.println("Aucun capteur de puissance (0x1818) trouvé.");
  }

  if (cscDev) {
    g_speed_connected = connectToService(cscDev, SVC_CSC, CHR_CSC_MEAS, cscNotifyCB);
    Serial.println(g_speed_connected ? "Capteur de vitesse connecté."
                                      : "Échec connexion capteur de vitesse.");
  } else {
    Serial.println("Aucun capteur de vitesse (0x1816) trouvé.");
  }
}

// =================================================================
//  Setup / Loop
// =================================================================

void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000); // 100 kHz, prudent pour du câblage déporté par tube+fil

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Erreur: carte SD non détectée.");
  } else {
    g_logFile = SD.open("/cda_log.csv", FILE_WRITE);
    if (g_logFile) {
      g_logFile.println("millis_ms,dp_dyn_pa,dp_yaw_pa,power_w,speed_kmh");
      g_logFile.flush();
    }
  }

  NimBLEDevice::init("CdA-Logger");
  scanAndConnect();
}

void loop() {
  static uint32_t lastSample = 0;
  uint32_t now = millis();
  if (now - lastSample < SAMPLE_PERIOD_MS) return;
  lastSample = now;

  float dpDyn = NAN, dpYaw = NAN;
  bool okDyn = sdpMeasure(SDP_ADDR_DYN, dpDyn);
  bool okYaw = sdpMeasure(SDP_ADDR_YAW, dpYaw);

  if (!okDyn) Serial.println("Lecture SDP dynamique invalide (CRC ou I2C).");
  if (!okYaw) Serial.println("Lecture SDP lacet invalide (CRC ou I2C).");

  if (g_logFile) {
    g_logFile.printf("%lu,%.3f,%.3f,%d,%.2f\n",
                      now, okDyn ? dpDyn : NAN, okYaw ? dpYaw : NAN,
                      (int)g_power_w, (float)g_speed_kmh);
    // flush périodique pour limiter l'usure de la carte SD
    static uint8_t flushCounter = 0;
    if (++flushCounter >= 20) { g_logFile.flush(); flushCounter = 0; }
  }
}
