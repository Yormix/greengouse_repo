// =============================================================
//  GREENHOUSE REPO — Nodo Receptor Central
//  Instituto Tecnológico de Uruapan — 2025
// =============================================================
//  Responsabilidad:
//    - Conectarse al broker Mosquitto como cliente MQTT principal.
//    - Ejecutar radar de detección: escaneo WiFi (redes visibles)
//      y BLE (dispositivos cercanos) → publica resultados.
//    - Controlar actuadores y dispositivos externos propios.
//    - Reportar su estado (ping) periódicamente.
//    - Actuar como punto de referencia de conectividad para
//      Node-RED (el broker sabe si este nodo está vivo).
//
//  Topics MQTT publicados:
//    invernadero/receptor/ping               → estado y MAC/IP
//    invernadero/receptor/radar/wifi         → lista de redes detectadas
//    invernadero/receptor/radar/ble          → lista de dispositivos BLE
//    invernadero/receptor/actuador/<id>/ack  → confirmación de cambio
//    invernadero/receptor/dispositivo/<id>/ack
//
//  Topics MQTT suscritos:
//    invernadero/receptor/actuador/+         → {"id_actuador": N, "estado": true}
//    invernadero/receptor/dispositivo/+      → {"id_dispositivo": N, "senal": 1}
//    invernadero/radar/scan                  → {} (solicitud de escaneo inmediato)
//
//  Dependencias (instalar desde Arduino Library Manager):
//    - PubSubClient  by Nick O'Leary
//    - ArduinoJson   by Benoit Blanchon
//    (BLE y WiFi scan son parte del SDK del ESP32, no requieren instalación)
// =============================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "secrets.h"   // WiFi y MQTT credentials — NUNCA subir al repo

// -------------------------------------------------------------
//  CONFIGURACIÓN DEL NODO
// -------------------------------------------------------------
#define NODO_ID     0              // 0 = Receptor Central (ID fijo)
#define NODO_NOMBRE "Nodo Receptor Central"

// -------------------------------------------------------------
//  PINES — Actuadores y dispositivos del receptor central
// -------------------------------------------------------------
#define PIN_ACTUADOR_1   26        // GPIO26 — Relé 1
#define PIN_ACTUADOR_2   27        // GPIO27 — Relé 2
#define PIN_ACTUADOR_3   14        // GPIO14 — Relé 3

#define PIN_DISPOSITIVO_1  32      // GPIO32 — Puerta principal
#define PIN_DISPOSITIVO_2  33      // GPIO33 — Ventana zona norte
#define PIN_DISPOSITIVO_3  25      // GPIO25 — Escotilla

// -------------------------------------------------------------
//  TIEMPOS
// -------------------------------------------------------------
#define INTERVALO_PING_MS        30000   // Ping cada 30 s
#define INTERVALO_RADAR_WIFI_MS 120000   // Radar WiFi cada 2 min
#define INTERVALO_RADAR_BLE_MS   60000   // Radar BLE cada 1 min
#define DURACION_SCAN_BLE_S           5  // Segundos de escaneo BLE
#define TIMEOUT_RECONEXION_MS    5000

// -------------------------------------------------------------
//  OBJETOS GLOBALES
// -------------------------------------------------------------
WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);
BLEScan*     bleScan = nullptr;

unsigned long ultimoPing      = 0;
unsigned long ultimoRadarWiFi = 0;
unsigned long ultimoRadarBLE  = 0;

bool radarSolicitado = false;   // Flag para escaneo bajo demanda

// =============================================================
//  SETUP
// =============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n== Greenhouse Repo — Nodo Receptor Central ==");

  // Configurar pines de salida
  int pinesActuadores[]  = {PIN_ACTUADOR_1,    PIN_ACTUADOR_2,    PIN_ACTUADOR_3};
  int pinesDispositivos[] = {PIN_DISPOSITIVO_1, PIN_DISPOSITIVO_2, PIN_DISPOSITIVO_3};

  for (int pin : pinesActuadores)   { pinMode(pin, OUTPUT); digitalWrite(pin, LOW); }
  for (int pin : pinesDispositivos) { pinMode(pin, OUTPUT); digitalWrite(pin, LOW); }

  // Inicializar BLE (solo modo escaneo, sin broadcasting)
  BLEDevice::init(NODO_NOMBRE);
  bleScan = BLEDevice::getScan();
  bleScan->setActiveScan(true);    // Solicita información adicional a los dispositivos
  bleScan->setInterval(100);
  bleScan->setWindow(99);

  conectarWiFi();

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(callbackMQTT);
  mqtt.setBufferSize(2048);        // Buffer más grande para payloads de radar

  conectarMQTT();
}

// =============================================================
//  LOOP PRINCIPAL
// =============================================================
void loop() {
  if (!mqtt.connected()) {
    conectarMQTT();
  }
  mqtt.loop();

  unsigned long ahora = millis();

  if (ahora - ultimoPing >= INTERVALO_PING_MS) {
    ultimoPing = ahora;
    publicarPing();
  }

  if (ahora - ultimoRadarWiFi >= INTERVALO_RADAR_WIFI_MS || radarSolicitado) {
    ultimoRadarWiFi = ahora;
    ejecutarRadarWiFi();
  }

  if (ahora - ultimoRadarBLE >= INTERVALO_RADAR_BLE_MS || radarSolicitado) {
    ultimoRadarBLE = ahora;
    ejecutarRadarBLE();
    radarSolicitado = false;
  }
}

// =============================================================
//  CONEXIÓN WiFi
// =============================================================
void conectarWiFi() {
  Serial.printf("Conectando a WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado.");
    Serial.printf("IP: %s  MAC: %s\n",
                  WiFi.localIP().toString().c_str(),
                  WiFi.macAddress().c_str());
  } else {
    Serial.println("\nError WiFi. Reiniciando...");
    delay(3000);
    ESP.restart();
  }
}

// =============================================================
//  CONEXIÓN MQTT
// =============================================================
void conectarMQTT() {
  Serial.printf("Conectando a MQTT %s:%d\n", MQTT_BROKER, MQTT_PORT);

  while (!mqtt.connected()) {
    bool ok = mqtt.connect("greenhouse-receptor", MQTT_USER, MQTT_PASSWORD);
    if (ok) {
      Serial.println("MQTT conectado.");
      mqtt.subscribe("invernadero/receptor/actuador/+");
      mqtt.subscribe("invernadero/receptor/dispositivo/+");
      mqtt.subscribe("invernadero/radar/scan");
      Serial.println("Suscripciones activas.");
    } else {
      Serial.printf("Error MQTT rc=%d. Reintento en %d ms...\n",
                    mqtt.state(), TIMEOUT_RECONEXION_MS);
      delay(TIMEOUT_RECONEXION_MS);
    }
  }
}

// =============================================================
//  CALLBACK — Mensajes recibidos
// =============================================================
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  char mensaje[512];
  length = min(length, (unsigned int)(sizeof(mensaje) - 1));
  memcpy(mensaje, payload, length);
  mensaje[length] = '\0';

  Serial.printf("Rx [%s]: %s\n", topic, mensaje);

  String topicStr = String(topic);

  // --- Solicitud de radar inmediato ---
  if (topicStr == "invernadero/radar/scan") {
    radarSolicitado = true;
    Serial.println("Escaneo de radar solicitado.");
    return;
  }

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, mensaje) != DeserializationError::Ok) {
    Serial.println("Error: JSON inválido.");
    return;
  }

  // --- Comando de actuador ---
  if (topicStr.indexOf("/actuador/") >= 0) {
    int  id     = doc["id_actuador"] | 0;
    bool estado = doc["estado"]      | false;

    Serial.printf("Actuador %d -> %s\n", id, estado ? "ON" : "OFF");

    switch (id) {
      case 1: digitalWrite(PIN_ACTUADOR_1, estado ? HIGH : LOW); break;
      case 2: digitalWrite(PIN_ACTUADOR_2, estado ? HIGH : LOW); break;
      case 3: digitalWrite(PIN_ACTUADOR_3, estado ? HIGH : LOW); break;
      default: Serial.printf("Actuador %d sin mapeo.\n", id); return;
    }

    // Publicar ACK de confirmación
    char ackTopic[60];
    snprintf(ackTopic, sizeof(ackTopic), "invernadero/receptor/actuador/%d/ack", id);
    StaticJsonDocument<128> ack;
    ack["id_actuador"] = id;
    ack["estado"]      = estado;
    ack["confirmado"]  = true;
    char ackBuf[128];
    serializeJson(ack, ackBuf);
    mqtt.publish(ackTopic, ackBuf);
  }

  // --- Comando de dispositivo externo ---
  else if (topicStr.indexOf("/dispositivo/") >= 0) {
    int id    = doc["id_dispositivo"] | 0;
    int senal = doc["senal"]          | 0;

    Serial.printf("Dispositivo %d -> señal %d\n", id, senal);

    switch (id) {
      case 1: digitalWrite(PIN_DISPOSITIVO_1, senal == 1 ? HIGH : LOW); break;
      case 2: digitalWrite(PIN_DISPOSITIVO_2, senal == 1 ? HIGH : LOW); break;
      case 3: digitalWrite(PIN_DISPOSITIVO_3, senal == 1 ? HIGH : LOW); break;
      default: Serial.printf("Dispositivo %d sin mapeo.\n", id); return;
    }

    // ACK
    char ackTopic[64];
    snprintf(ackTopic, sizeof(ackTopic), "invernadero/receptor/dispositivo/%d/ack", id);
    StaticJsonDocument<128> ack;
    ack["id_dispositivo"] = id;
    ack["senal"]          = senal;
    ack["confirmado"]     = true;
    char ackBuf[128];
    serializeJson(ack, ackBuf);
    mqtt.publish(ackTopic, ackBuf);
  }
}

// =============================================================
//  PING DE ESTADO
// =============================================================
void publicarPing() {
  StaticJsonDocument<128> doc;
  doc["mac"]       = WiFi.macAddress();
  doc["ip"]        = WiFi.localIP().toString();
  doc["rssi_wifi"] = WiFi.RSSI();

  char buffer[128];
  serializeJson(doc, buffer);
  mqtt.publish("invernadero/receptor/ping", buffer, false);
  Serial.printf("Ping: %s\n", buffer);
}

// =============================================================
//  RADAR WiFi — Escaneo de redes visibles
//  Publica en: invernadero/receptor/radar/wifi
//  Tabla destino: RADAR_DETECCION (via Node-RED)
//
//  Payload:
//  {
//    "dispositivos": [
//      {"mac": "AA:BB:CC:DD:EE:FF", "ssid": "MiRed", "rssi": -65, "distancia_cm": null},
//      ...
//    ],
//    "total": 3
//  }
//
//  Nota: WiFi scan no provee distancia confiable (sin triangulación).
//        distancia_cm se envía null para que Node-RED lo registre así.
// =============================================================
void ejecutarRadarWiFi() {
  Serial.println("Iniciando radar WiFi...");

  // Desconectar temporalmente para escaneo pasivo
  // WiFi.scanNetworks es síncrono; no afecta la conexión MQTT establecida
  int n = WiFi.scanNetworks(false, true);   // async=false, show_hidden=true

  if (n == WIFI_SCAN_FAILED || n == 0) {
    Serial.printf("Radar WiFi: %d redes encontradas.\n", n);
    return;
  }

  // Construir JSON con hasta 20 dispositivos para no exceder el buffer MQTT
  DynamicJsonDocument doc(2048);
  JsonArray lista = doc.createNestedArray("dispositivos");

  int maxDisp = min(n, 20);
  for (int i = 0; i < maxDisp; i++) {
    JsonObject disp = lista.createNestedObject();
    disp["mac"]          = WiFi.BSSIDstr(i);
    disp["ssid"]         = WiFi.SSID(i);
    disp["rssi"]         = WiFi.RSSI(i);
    disp["distancia_cm"] = (char*)nullptr;   // null — sin cálculo de distancia
  }
  doc["total"] = n;

  char buffer[2048];
  serializeJson(doc, buffer);
  mqtt.publish("invernadero/receptor/radar/wifi", buffer, false);
  Serial.printf("Radar WiFi: %d redes publicadas.\n", maxDisp);

  WiFi.scanDelete();   // Liberar memoria del scan
}

// =============================================================
//  RADAR BLE — Escaneo de dispositivos Bluetooth Low Energy
//  Publica en: invernadero/receptor/radar/ble
//  Tabla destino: RADAR_DETECCION (via Node-RED)
//
//  Payload:
//  {
//    "dispositivos": [
//      {"mac": "AA:BB:CC:DD:EE:FF", "nombre": "ESP32-Sensor", "rssi": -72, "distancia_cm": 85.3},
//      ...
//    ],
//    "total": 2
//  }
//
//  Distancia estimada mediante fórmula log-distance path loss:
//    distancia = 10 ^ ((txPower - RSSI) / (10 * n))
//  donde txPower ≈ -59 dBm a 1 metro (constante empírica para ESP32)
//  y n = 2.0 (exponente de pérdida de trayectoria en espacio libre).
//  El resultado tiene margen de error de ±30% en interiores.
// =============================================================
void ejecutarRadarBLE() {
  Serial.println("Iniciando radar BLE...");

  BLEScanResults resultados = bleScan->start(DURACION_SCAN_BLE_S, false);
  int n = resultados.getCount();

  if (n == 0) {
    Serial.println("Radar BLE: ningún dispositivo encontrado.");
    bleScan->clearResults();
    return;
  }

  DynamicJsonDocument doc(2048);
  JsonArray lista = doc.createNestedArray("dispositivos");

  int maxDisp = min(n, 20);
  for (int i = 0; i < maxDisp; i++) {
    BLEAdvertisedDevice dispositivo = resultados.getDevice(i);

    float rssi       = dispositivo.getRSSI();
    float distanciaCm = estimarDistanciaBLE(rssi);

    JsonObject disp = lista.createNestedObject();
    disp["mac"]          = dispositivo.getAddress().toString().c_str();
    disp["nombre"]       = dispositivo.haveName() ? dispositivo.getName().c_str() : "";
    disp["rssi"]         = (int)rssi;
    disp["distancia_cm"] = round(distanciaCm * 10.0) / 10.0;  // 1 decimal
  }
  doc["total"] = n;

  char buffer[2048];
  serializeJson(doc, buffer);
  mqtt.publish("invernadero/receptor/radar/ble", buffer, false);
  Serial.printf("Radar BLE: %d dispositivos publicados.\n", maxDisp);

  bleScan->clearResults();
}

// =============================================================
//  ESTIMACIÓN DE DISTANCIA BLE
//  Fórmula log-distance path loss.
//  txPower: potencia de transmisión a 1 metro en dBm.
//  n:       exponente de pérdida (2.0 libre, 3.0-4.0 en interiores).
// =============================================================
float estimarDistanciaBLE(float rssi) {
  const float txPower = -59.0;   // dBm a 1 metro (ESP32 típico)
  const float n       = 2.5;     // Exponente para interior con obstáculos

  if (rssi >= 0) return 0.0;     // Valor inválido

  float distanciaMetros = pow(10.0, (txPower - rssi) / (10.0 * n));
  return distanciaMetros * 100.0;   // Convertir a centímetros
}
