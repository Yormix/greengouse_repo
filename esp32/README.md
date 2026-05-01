# ESP32 — Greenhouse Repo

Código Arduino para los dos nodos ESP32 del sistema.

## Estructura

```
esp32/
├── nodo_colector/
│   ├── nodo_colector.ino   ← Código principal (subir a cada nodo colector)
│   └── secrets.h           ← Credenciales WiFi/MQTT (NO está en el repo)
│
└── nodo_receptor/
    ├── nodo_receptor.ino   ← Código principal (subir al nodo central)
    └── secrets.h           ← Credenciales WiFi/MQTT (NO está en el repo)
```

---

## Diferencias entre nodos

| | Nodo Colector | Nodo Receptor Central |
|---|---|---|
| **Función** | Lee sensores, publica lecturas | Radar WiFi/BLE, controla dispositivos |
| **Sensores** | DHT22 + humedad suelo + pH | Ninguno |
| **Radar** | No | Sí (WiFi scan + BLE scan) |
| **Actuadores** | Sí (relés locales) | Sí (relés centrales) |
| **NODO_ID** | 1, 2, 3… (único por unidad) | 0 (fijo) |
| **Puede haber varios** | Sí | No (solo uno) |

---

## Instalación

### 1. Instalar Arduino IDE 2.x
Descargar desde: https://www.arduino.cc/en/software

### 2. Agregar soporte para ESP32 en Arduino IDE

1. Ir a **File → Preferences**
2. En *Additional boards manager URLs* pegar:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Ir a **Tools → Board → Boards Manager**
4. Buscar `esp32` e instalar **esp32 by Espressif Systems** (versión 2.x o 3.x)

### 3. Instalar librerías requeridas

Ir a **Tools → Manage Libraries** e instalar:

| Librería | Autor | Para qué |
|---|---|---|
| `PubSubClient` | Nick O'Leary | Cliente MQTT |
| `ArduinoJson` | Benoit Blanchon | Serializar/deserializar JSON |
| `DHT sensor library` | Adafruit | Sensor DHT22 |
| `Adafruit Unified Sensor` | Adafruit | Requerida por DHT |

Las librerías BLE y WiFi ya vienen incluidas con el paquete de ESP32.

---

## Configuración antes de subir

### 1. Crear tu `secrets.h`

Dentro de la carpeta del nodo que vas a programar, crea un archivo `secrets.h` copiando la plantilla:

```cpp
// secrets.h
#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID       "TU_RED_WIFI"
#define WIFI_PASSWORD   "TU_CONTRASEÑA"
#define MQTT_BROKER     "192.168.1.100"   // IP de la PC donde corre Docker
#define MQTT_PORT       1883
#define MQTT_USER       "greenhouse"
#define MQTT_PASSWORD   "tu_password_mqtt"

#endif
```

> **Nota:** La IP del broker es la IP local de la computadora donde corre Docker con Mosquitto. No uses `localhost` en el ESP32 — esa dirección apunta a él mismo.

### 2. Ajustar el NODO_ID (solo nodo colector)

Abre `nodo_colector.ino` y cambia la línea:
```cpp
#define NODO_ID   1   // ← Número único: 1 para el primero, 2 para el segundo, etc.
```

Cada ESP32 que funcione como colector debe tener un ID diferente.

### 3. Ajustar pines

Si el cableado físico de tu nodo es diferente al default, edita las constantes `#define PIN_*` al inicio de cada archivo `.ino`.

### 4. Calibrar sensor de pH (nodo colector)

1. Conecta el módulo SEN0161 al GPIO35.
2. Abre el Monitor Serie en 115200 baudios.
3. Mide el voltaje con solución buffer pH 7.0 y actualiza `PH_VOLTAJE_NEUTRO`.
4. Mide con solución pH 4.0, calcula la diferencia de voltaje y actualiza `PH_PENDIENTE`.

### 5. Calibrar sensor de humedad de suelo (nodo colector)

1. Lee el valor ADC con el sensor en **aire seco** → actualiza `SUELO_VALOR_SECO`.
2. Lee el valor ADC con el sensor en **agua destilada** → actualiza `SUELO_VALOR_HUMEDO`.

---

## Selección de placa en Arduino IDE

- **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
- **Tools → Port** → seleccionar el puerto COM donde está conectado el ESP32
- **Tools → Upload Speed** → 115200 (o 921600 si quieres más velocidad)
- **Tools → Flash Size** → 4MB (default)

---

## Topics MQTT del sistema

### Nodo Colector (NODO_ID = N)

| Topic | Dirección | Descripción |
|---|---|---|
| `invernadero/nodoN/temperatura` | Publica | Temperatura ambiente °C |
| `invernadero/nodoN/humedad_amb` | Publica | Humedad relativa % |
| `invernadero/nodoN/humedad_suelo` | Publica | Humedad suelo % |
| `invernadero/nodoN/ph` | Publica | pH del suelo |
| `invernadero/nodoN/ping` | Publica | Estado del nodo (MAC + IP) |
| `invernadero/nodoN/actuador/+` | Suscribe | Comando encender/apagar actuador |
| `invernadero/nodoN/dispositivo/+` | Suscribe | Señal 0/1 a dispositivo externo |

### Nodo Receptor Central

| Topic | Dirección | Descripción |
|---|---|---|
| `invernadero/receptor/ping` | Publica | Estado del nodo (MAC + IP + RSSI) |
| `invernadero/receptor/radar/wifi` | Publica | Redes WiFi detectadas (MAC, SSID, RSSI) |
| `invernadero/receptor/radar/ble` | Publica | Dispositivos BLE (MAC, nombre, distancia) |
| `invernadero/receptor/actuador/+` | Suscribe | Comando de actuador |
| `invernadero/receptor/dispositivo/+` | Suscribe | Señal a dispositivo externo |
| `invernadero/radar/scan` | Suscribe | Solicita escaneo de radar inmediato |

---

## Formato JSON de los mensajes

### Lectura de sensor
```json
{
  "valor": 23.5,
  "unidad": "°C",
  "estado": "normal"
}
```
`estado` puede ser: `"normal"` | `"alerta"` | `"critico"`

### Ping de nodo
```json
{
  "mac": "AA:BB:CC:DD:EE:FF",
  "ip": "192.168.1.45"
}
```

### Radar BLE
```json
{
  "dispositivos": [
    {"mac": "AA:BB:CC:DD:EE:FF", "nombre": "ESP32-Sensor", "rssi": -72, "distancia_cm": 85.3}
  ],
  "total": 1
}
```

### Radar WiFi
```json
{
  "dispositivos": [
    {"mac": "AA:BB:CC:DD:EE:FF", "ssid": "RedInvernadero", "rssi": -65, "distancia_cm": null}
  ],
  "total": 1
}
```

### Comando de actuador
```json
{
  "id_actuador": 1,
  "estado": true
}
```

### Señal a dispositivo externo
```json
{
  "id_dispositivo": 2,
  "senal": 1
}
```

---

## Diagrama de pines (referencia)

### Nodo Colector
```
GPIO4  → DHT22 (DATA)
GPIO34 → Sensor humedad suelo (analógico)
GPIO35 → Módulo pH SEN0161 (analógico)
GPIO26 → Relé 1 / Actuador 1
GPIO27 → Relé 2 / Actuador 2
GPIO32 → Dispositivo externo 1
```

### Nodo Receptor Central
```
GPIO26 → Relé 1 / Actuador 1
GPIO27 → Relé 2 / Actuador 2
GPIO14 → Relé 3 / Actuador 3
GPIO32 → Dispositivo externo 1 (puerta principal)
GPIO33 → Dispositivo externo 2 (ventana zona norte)
GPIO25 → Dispositivo externo 3 (escotilla)
```

---

## Problemas comunes

| Problema | Causa probable | Solución |
|---|---|---|
| `WiFi no conecta` | Credenciales incorrectas | Verificar `secrets.h`, SSID sensible a mayúsculas |
| `MQTT rc=-2` | Broker no alcanzable | Verificar IP del broker, que Docker esté corriendo |
| `MQTT rc=-4` | Credenciales MQTT incorrectas | Verificar `MQTT_USER` y `MQTT_PASSWORD` |
| `DHT22 no responde` | Cableado o resistencia pull-up | Agregar resistencia 4.7kΩ entre DATA y 3.3V |
| `pH siempre en 7.0` | Sin calibración | Ejecutar calibración con soluciones buffer |
| `BLE scan bloquea WiFi` | Limitación del radio ESP32 | Normal, el scan BLE es breve (5 s) y luego se reanuda MQTT |
| `Mensajes cortados en MQTT` | Buffer insuficiente | Ya está en 2048; si persiste, reducir el número de dispositivos en el JSON |

---

Instituto Tecnológico de Uruapan — Greenhouse Repo — 2025
