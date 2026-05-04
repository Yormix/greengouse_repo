// =============================================================
//  secrets.h — Nodo Receptor Central
//  Greenhouse Repo | Instituto Tecnológico de Uruapan — 2025
// =============================================================
//
//  ██ ESTE ARCHIVO NO SE SUBE AL REPOSITORIO ██
//  Está listado en .gitignore: esp32/**/secrets.h
//
//  Cada integrante del equipo crea su propia copia local
//  con los datos reales del entorno donde va a probar.
//
//  Cómo usarlo:
//    1. Este archivo ya existe en el repo como plantilla.
//    2. Rellena los valores de tu entorno.
//    3. Nunca hagas "git add secrets.h".
//
// =============================================================

#ifndef SECRETS_H
#define SECRETS_H

// -------------------------------------------------------------
//  RED WiFi
//  La red a la que se conectará el ESP32.
//  El ESP32 y la PC con Docker deben estar en la misma red local.
// -------------------------------------------------------------
#define WIFI_SSID       "NOMBRE_DE_TU_RED"      // Sensible a mayúsculas
#define WIFI_PASSWORD   "CONTRASEÑA_DE_TU_RED"

// -------------------------------------------------------------
//  BROKER MQTT (Mosquitto corriendo en Docker)
//
//  MQTT_BROKER: IP local de la PC donde corre Docker.
//    - Windows → abre CMD → ipconfig → busca "Dirección IPv4"
//    - Mac/Linux → ip a  o  ifconfig
//    - Ejemplo típico: "192.168.1.100"
//
//  ¡NO uses "localhost" ni "127.0.0.1"!
//  Esas direcciones apuntan al propio ESP32, no a tu PC.
//
//  MQTT_PORT: 1883 es el puerto estándar. Solo cambia si
//    modificaste el docker-compose.yml.
//
//  MQTT_USER / MQTT_PASSWORD: deben coincidir con las
//    credenciales del archivo passwd de Mosquitto.
//    Si usas allow_anonymous true (solo desarrollo),
//    puedes dejar ambos como cadena vacía: "".
// -------------------------------------------------------------
#define MQTT_BROKER     "192.168.X.X"   // ← Reemplazar con la IP real de tu PC
#define MQTT_PORT       1883
#define MQTT_USER       "greenhouse"
#define MQTT_PASSWORD   "reemplazar_con_password_real"

#endif // SECRETS_H
