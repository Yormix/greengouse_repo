// =============================================================
//  GREENHOUSE REPO — Nodo Colector
//  Instituto Tecnológico de Uruapan — 2025
// =============================================================
//  Responsabilidad:
//    - Leer sensores: temperatura, humedad ambiente (DHT22),
//      humedad de suelo (resistivo/capacitivo) y pH (SEN0161).
//    - Publicar lecturas al broker Mosquitto vía WiFi + MQTT.
//    - Reportar su propio estado (ping) al backend.
//    - Escuchar comandos del broker para encender/apagar
//      actuadores conectados a este nodo.
//
//  Topics MQTT publicados:
//    invernadero/nodo<ID>/temperatura   → {"valor": 23.5, "unidad": "°C",  "estado": "normal"}
//    invernadero/nodo<ID>/humedad_amb   → {"valor": 65.2, "unidad": "%",   "estado": "normal"}
//    invernadero/nodo<ID>/humedad_suelo → {"valor": 42.0, "unidad": "%",   "estado": "alerta"}
//    invernadero/nodo<ID>/ph            → {"valor": 6.8,  "unidad": "pH",  "estado": "normal"}
//    invernadero/nodo<ID>/ping          → {"mac": "XX:XX:XX:XX:XX:XX", "ip": "192.168.1.x"}
//
//  Topics MQTT suscritos:
//    invernadero/nodo<ID>/actuador/+    → {"id_actuador": 1, "estado": true}
//    invernadero/nodo<ID>/dispositivo/+ → {"id_dispositivo": 2, "senal": 1}
//
//  Dependencias (instalar desde Arduino Library Manager):
//    - PubSubClient  by Nick O'Leary     (MQTT)
//    - ArduinoJson   by Benoit Blanchon  (JSON)
//    - DHT sensor library by Adafruit    (DHT22)
//    - Adafruit Unified Sensor           (requerida por DHT)
// =============================================================

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "secrets.h"   // WiFi y MQTT credentials — NUNCA subir al repo

// -------------------------------------------------------------
//  CONFIGURACIÓN DEL NODO — Cambiar por cada unidad física
// -------------------------------------------------------------
#define NODO_ID          1          // Número único de este nodo (1, 2, 3...)
#define NODO_NOMBRE      "Nodo Colector 1"

// -------------------------------------------------------------
//  PINES
// -------------------------------------------------------------
#define PIN_DHT          4          // GPIO4 — DHT22 (temp + humedad ambiente)
#define PIN_HUMEDAD_SUELO 34        // GPIO34 — Sensor resistivo/capacitivo (ADC)
#define PIN_PH           35         // GPIO35 — Módulo pH SEN0161 (ADC)

// Actuadores — ajustar según cableado real del nodo
#define PIN_ACTUADOR_1   26         // GPIO26 — Relé 1 (ej. bomba de riego)
#define PIN_ACTUADOR_2   27         // GPIO27 — Relé 2 (ej. ventilador)

// Dispositivos externos (señal digital 0/1 a través de GPIO)
#define PIN_DISPOSITIVO_1  32       // GPIO32 — Puerta / ventana / escotilla

// -------------------------------------------------------------
//  RANGOS DE CLASIFICACIÓN (deben coincidir con Node-RED)
//  normal / alerta / critico → tabla LECTURA_SENSOR.estado
// -------------------------------------------------------------
#define TEMP_MIN_NORMAL    15.0
#define TEMP_MAX_NORMAL    35.0
#define TEMP_MAX_ALERTA    40.0

#define HUM_AMB_MIN_NORMAL  40.0
#define HUM_AMB_MAX_NORMAL  80.0

#define HUM_SUELO_MIN_NORMAL 40.0
#define HUM_SUELO_MAX_NORMAL 80.0

#define PH_MIN_NORMAL      5.5
#define PH_MAX_NORMAL      7.5
#define PH_MIN_ALERTA      4.5
#define PH_MAX_ALERTA      8.5

// -------------------------------------------------------------
//  CALIBRACIÓN DEL SENSOR DE pH  (SEN0161 en 3.3V)
//  Ajustar con soluciones buffer 4.0 y 7.0
// -------------------------------------------------------------
#define PH_VOLTAJE_NEUTRO  1.65     // Voltaje en pH 7.0 (ajustar con buffer)
#define PH_PENDIENTE      -5.0      // mV por unidad de pH (ajustar con calibración)

// -------------------------------------------------------------
//  CALIBRACIÓN HUMEDAD DE SUELO (valores ADC)
//  Medir: sensor en aire → valor_seco; sensor en agua → valor_humedo
// -------------------------------------------------------------
#define SUELO_VALOR_SECO   3500     // ADC cuando el sensor está completamente seco
#define SUELO_VALOR_HUMEDO 1200     // ADC cuando el sensor está en agua

// -------------------------------------------------------------
//  TIEMPOS
// -------------------------------------------------------------
#define INTERVALO_LECTURA_MS   10000   // Publicar sensores cada 10 segundos
#define INTERVALO_PING_MS      30000   // Ping de estado cada 30 segundos
#define TIMEOUT_RECONEXION_MS   5000   // Espera entre reintentos de conexión

// -------------------------------------------------------------
//  OBJETOS GLOBALES
// -------------------------------------------------------------
DHT dht(PIN_DHT, DHT22);

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

// Topics base (se construyen con el NODO_ID)
char topicTemp[50];
char topicHumAmb[50];
char topicHumSuelo[50];
char topicPH[50];
char topicPing[50];
char topicSubActuador[50];
char topicSubDispositivo[50];

unsigned long ultimaLectura  = 0;
unsigned long ultimoPing     = 0;

// =============================================================
//  SETUP
// =============================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n== Greenhouse Repo — Nodo Colector ==");
  Serial.printf("   Nodo ID   : %d\n", NODO_ID);
  Serial.printf("   Nodo Nombre: %s\n", NODO_NOMBRE);

  // Construir topics con el ID de este nodo
  snprintf(topicTemp,          sizeof(topicTemp),          "invernadero/nodo%d/temperatura",   NODO_ID);
  snprintf(topicHumAmb,        sizeof(topicHumAmb),        "invernadero/nodo%d/humedad_amb",   NODO_ID);
  snprintf(topicHumSuelo,      sizeof(topicHumSuelo),      "invernadero/nodo%d/humedad_suelo", NODO_ID);
  snprintf(topicPH,            sizeof(topicPH),            "invernadero/nodo%d/ph",            NODO_ID);
  snprintf(topicPing,          sizeof(topicPing),          "invernadero/nodo%d/ping",          NODO_ID);
  snprintf(topicSubActuador,   sizeof(topicSubActuador),   "invernadero/nodo%d/actuador/+",    NODO_ID);
  snprintf(topicSubDispositivo,sizeof(topicSubDispositivo),"invernadero/nodo%d/dispositivo/+", NODO_ID);

  // Configurar pines de salida (actuadores y dispositivos)
  pinMode(PIN_ACTUADOR_1,    OUTPUT);
  pinMode(PIN_ACTUADOR_2,    OUTPUT);
  pinMode(PIN_DISPOSITIVO_1, OUTPUT);
  digitalWrite(PIN_ACTUADOR_1,    LOW);  // Apagado por defecto
  digitalWrite(PIN_ACTUADOR_2,    LOW);
  digitalWrite(PIN_DISPOSITIVO_1, LOW);

  dht.begin();

  conectarWiFi();

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(callbackMQTT);
  mqtt.setBufferSize(512);

  conectarMQTT();
}

// =============================================================
//  LOOP PRINCIPAL
// =============================================================
void loop() {
  // Mantener conexión MQTT activa
  if (!mqtt.connected()) {
    conectarMQTT();
  }
  mqtt.loop();

  unsigned long ahora = millis();

  // Publicar lecturas de sensores
  if (ahora - ultimaLectura >= INTERVALO_LECTURA_MS) {
    ultimaLectura = ahora;
    publicarSensores();
  }

  // Publicar ping de estado
  if (ahora - ultimoPing >= INTERVALO_PING_MS) {
    ultimoPing = ahora;
    publicarPing();
  }
}

// =============================================================
//  CONEXIÓN WiFi
// =============================================================
void conectarWiFi() {
  Serial.printf("Conectando a WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado.");
    Serial.printf("IP local: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("MAC: %s\n",      WiFi.macAddress().c_str());
  } else {
    Serial.println("\nError: no se pudo conectar al WiFi. Reiniciando...");
    delay(3000);
    ESP.restart();
  }
}

// =============================================================
//  CONEXIÓN MQTT
// =============================================================
void conectarMQTT() {
  char clientId[30];
  snprintf(clientId, sizeof(clientId), "greenhouse-nodo%d", NODO_ID);

  Serial.printf("Conectando a MQTT broker %s:%d\n", MQTT_BROKER, MQTT_PORT);

  while (!mqtt.connected()) {
    bool conectado = mqtt.connect(clientId, MQTT_USER, MQTT_PASSWORD);

    if (conectado) {
      Serial.println("MQTT conectado.");

      // Suscribirse a comandos de actuadores y dispositivos
      mqtt.subscribe(topicSubActuador);
      mqtt.subscribe(topicSubDispositivo);

      Serial.printf("Suscrito a: %s\n", topicSubActuador);
      Serial.printf("Suscrito a: %s\n", topicSubDispositivo);
    } else {
      Serial.printf("Error MQTT (rc=%d). Reintentando en %d ms...\n",
                    mqtt.state(), TIMEOUT_RECONEXION_MS);
      delay(TIMEOUT_RECONEXION_MS);
    }
  }
}

// =============================================================
//  CALLBACK — Mensajes recibidos del broker
// =============================================================
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  // Convertir payload a string
  char mensaje[256];
  length = min(length, (unsigned int)(sizeof(mensaje) - 1));
  memcpy(mensaje, payload, length);
  mensaje[length] = '\0';

  Serial.printf("Mensaje recibido [%s]: %s\n", topic, mensaje);

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, mensaje);
  if (err) {
    Serial.printf("Error JSON: %s\n", err.c_str());
    return;
  }

  String topicStr = String(topic);

  // --- Comando de actuador ---
  // Topic: invernadero/nodoX/actuador/<id_actuador>
  if (topicStr.indexOf("/actuador/") >= 0) {
    bool estado = doc["estado"] | false;
    int  id     = doc["id_actuador"] | 0;

    Serial.printf("Actuador %d -> %s\n", id, estado ? "ENCENDIDO" : "APAGADO");

    // Mapear id_actuador al GPIO correspondiente
    // Ajustar este switch según la instalación física del nodo
    switch (id) {
      case 1: digitalWrite(PIN_ACTUADOR_1, estado ? HIGH : LOW); break;
      case 2: digitalWrite(PIN_ACTUADOR_2, estado ? HIGH : LOW); break;
      default:
        Serial.printf("Actuador %d no mapeado en este nodo.\n", id);
    }
  }

  // --- Comando de dispositivo externo ---
  // Topic: invernadero/nodoX/dispositivo/<id_dispositivo>
  else if (topicStr.indexOf("/dispositivo/") >= 0) {
    int senal = doc["senal"] | 0;   // 0 = cerrar/apagar, 1 = abrir/encender
    int id    = doc["id_dispositivo"] | 0;

    Serial.printf("Dispositivo %d -> señal %d\n", id, senal);

    switch (id) {
      case 1: digitalWrite(PIN_DISPOSITIVO_1, senal == 1 ? HIGH : LOW); break;
      default:
        Serial.printf("Dispositivo %d no mapeado en este nodo.\n", id);
    }
  }
}

// =============================================================
//  PUBLICAR LECTURAS DE SENSORES
// =============================================================
void publicarSensores() {
  // --- DHT22: temperatura y humedad ambiente ---
  float temperatura = dht.readTemperature();
  float humedadAmb  = dht.readHumidity();

  if (isnan(temperatura) || isnan(humedadAmb)) {
    Serial.println("Error: DHT22 no responde. Verificar cableado.");
  } else {
    publicarLectura(topicTemp,     temperatura, "°C",  clasificarTemperatura(temperatura));
    publicarLectura(topicHumAmb,   humedadAmb,  "%",   clasificarHumedadAmb(humedadAmb));
  }

  // --- Humedad de suelo (ADC analógico) ---
  int adcSuelo      = analogRead(PIN_HUMEDAD_SUELO);
  float humedadSuelo = mapearHumedadSuelo(adcSuelo);
  publicarLectura(topicHumSuelo, humedadSuelo, "%", clasificarHumedadSuelo(humedadSuelo));

  // --- pH (ADC analógico) ---
  float voltajePH = leerVoltajePH();
  float ph        = calcularPH(voltajePH);
  publicarLectura(topicPH, ph, "pH", clasificarPH(ph));
}

// Construye el JSON y publica en el topic correspondiente
void publicarLectura(const char* topic, float valor, const char* unidad, const char* estado) {
  StaticJsonDocument<128> doc;
  doc["valor"]  = round(valor * 100.0) / 100.0;  // 2 decimales
  doc["unidad"] = unidad;
  doc["estado"] = estado;

  char buffer[128];
  serializeJson(doc, buffer);

  bool ok = mqtt.publish(topic, buffer, true);  // retain=true
  Serial.printf("Pub %s -> %s [%s]\n", topic, buffer, ok ? "OK" : "ERROR");
}

// =============================================================
//  PUBLICAR PING DE ESTADO
// =============================================================
void publicarPing() {
  StaticJsonDocument<128> doc;
  doc["mac"] = WiFi.macAddress();
  doc["ip"]  = WiFi.localIP().toString();

  char buffer[128];
  serializeJson(doc, buffer);

  mqtt.publish(topicPing, buffer, false);
  Serial.printf("Ping publicado: %s\n", buffer);
}

// =============================================================
//  LECTURA Y CÁLCULO DE pH
// =============================================================
float leerVoltajePH() {
  // Promedio de 10 muestras para reducir ruido ADC
  long suma = 0;
  for (int i = 0; i < 10; i++) {
    suma += analogRead(PIN_PH);
    delay(10);
  }
  float adc = suma / 10.0;
  // ESP32: ADC de 12 bits (0-4095) con referencia de 3.3V
  return (adc / 4095.0) * 3.3;
}

float calcularPH(float voltaje) {
  // Fórmula lineal de calibración: pH = 7 + (Vneutro - V) / pendiente
  // Ajustar PH_VOLTAJE_NEUTRO y PH_PENDIENTE con soluciones buffer reales
  float ph = 7.0 + (PH_VOLTAJE_NEUTRO - voltaje) / PH_PENDIENTE * 1000.0;
  // Limitar a rango físico razonable
  return constrain(ph, 0.0, 14.0);
}

// =============================================================
//  MAPEO DE HUMEDAD DE SUELO (ADC → porcentaje)
// =============================================================
float mapearHumedadSuelo(int adcValor) {
  // Inversamente proporcional: más humedad = menor resistencia = menor ADC
  float porcentaje = map(adcValor, SUELO_VALOR_SECO, SUELO_VALOR_HUMEDO, 0, 100);
  return constrain(porcentaje, 0.0, 100.0);
}

// =============================================================
//  CLASIFICACIÓN DE ESTADOS
//  Retorna: "normal" | "alerta" | "critico"
//  Debe coincidir con la lógica de Node-RED
// =============================================================
const char* clasificarTemperatura(float v) {
  if (v >= TEMP_MIN_NORMAL && v <= TEMP_MAX_NORMAL) return "normal";
  if (v > TEMP_MAX_NORMAL && v <= TEMP_MAX_ALERTA)  return "alerta";
  return "critico";
}

const char* clasificarHumedadAmb(float v) {
  if (v >= HUM_AMB_MIN_NORMAL && v <= HUM_AMB_MAX_NORMAL) return "normal";
  if (v > HUM_AMB_MAX_NORMAL && v <= 90.0)                return "alerta";
  return "critico";
}

const char* clasificarHumedadSuelo(float v) {
  if (v >= HUM_SUELO_MIN_NORMAL && v <= HUM_SUELO_MAX_NORMAL) return "normal";
  if (v > 20.0 && v < HUM_SUELO_MIN_NORMAL)                   return "alerta";
  return "critico";
}

const char* clasificarPH(float v) {
  if (v >= PH_MIN_NORMAL && v <= PH_MAX_NORMAL) return "normal";
  if (v >= PH_MIN_ALERTA && v <= PH_MAX_ALERTA) return "alerta";
  return "critico";
}
