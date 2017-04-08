/**
 * Ai-Thinker RGBW Light Firmware - Configuration
 *
 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * Copyright (c) 2017 Sacha Telgenhof
 */

#define DEVICE_ID "AiLight"

// ---------------------------
// OTA (Over The Air) Updates
// ---------------------------
#define OTA_PORT 8266
#define OTA_PASSWORD "aithinker"

// ---------------------------
// WiFi
// ---------------------------
#define WIFI_SSID "<YOUR_WIFI_SSID>"
#define WIFI_PSK "<YOUR_WIFI_PSK/PASSWORD>"
#define WIFI_OUTPUT_POWER 1.0

// ---------------------------
// MQTT
// ---------------------------
#define MQTT_ENABLED true
#define MQTT_PORT 1883
#define MQTT_SERVER "<YOUR_MQTT_SERVER_ADDRESS>"
#define MQTT_USER "<YOUR_MQTT_USERNAME>"
#define MQTT_PASSWORD "<YOUR_MQTT_PASSWORD>"
#define MQTT_RECONNECT_TIME 10000
#define MQTT_QOS_LEVEL 0
#define MQTT_RETAIN false

#define MQTT_PAYLOAD_ON "ON"
#define MQTT_PAYLOAD_OFF "OFF"

#define MQTT_LIGHT_STATE_TOPIC "<YOUR_MQTT_STATE_TOPIC>"
#define MQTT_LIGHT_COMMAND_TOPIC "<YOUR_MQTT_COMMAND_TOPIC>"
