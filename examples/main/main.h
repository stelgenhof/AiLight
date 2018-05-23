/**
 * Ai-Thinker RGBW Light Firmware
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * Created by Sacha Telgenhof <me at sachatelgenhof dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2018 Sacha Telgenhof
 */

#define APP_NAME "AiLight"
#define APP_VERSION "0.6.0-dev"
#define APP_AUTHOR "me@sachatelgenhof.com"

#define DEVICE_MANUFACTURER "Ai-Thinker"
#define DEVICE_MODEL "RGBW Light"

// Power Up Modes
#define POWERUP_OFF 0
#define POWERUP_ON 1
#define POWERUP_SAME 2

#include "config.h"

// Fallback
#ifndef MQTT_HOMEASSISTANT_DISCOVERY_ENABLED
#define MQTT_HOMEASSISTANT_DISCOVERY_ENABLED false
#endif

#ifndef MQTT_HOMEASSISTANT_DISCOVERY_PREFIX
#define MQTT_HOMEASSISTANT_DISCOVERY_PREFIX "homeassistant"
#endif

#ifndef REST_API_ENABLED
#define REST_API_ENABLED false
#endif

#ifndef POWERUP_MODE
#define POWERUP_MODE POWERUP_OFF
#endif

#include "AiLight.hpp"
#include "ArduinoOTA.h"
#include <ArduinoJson.h>
#include <AsyncMqttClient.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Hash.h>
#include <Ticker.h>
#include <WiFiUdp.h>
#include <vector>

extern "C" {
#include "spi_flash.h"
}

#include "html.gz.h"

#define EEPROM_START_ADDRESS 0
#define INIT_HASH 0x5F
static const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);
#define RECONNECT_TIME 10

// Key names as used internally and in the WebUI
#define KEY_SETTINGS "s"
#define KEY_DEVICE "d"

#define KEY_STATE "state"
#define KEY_BRIGHTNESS "brightness"
#define KEY_WHITE "white_value"
#define KEY_COLORTEMP "color_temp"
#define KEY_FLASH "flash"
#define KEY_COLOR "color"
#define KEY_COLOR_R "r"
#define KEY_COLOR_G "g"
#define KEY_COLOR_B "b"
#define KEY_GAMMA_CORRECTION "gamma"
#define KEY_TRANSITION "transition"

#define KEY_HOSTNAME "hostname"
#define KEY_WIFI_SSID "wifi_ssid"
#define KEY_WIFI_PSK "wifi_psk"
#define KEY_MQTT_SERVER "mqtt_server"
#define KEY_MQTT_PORT "mqtt_port"
#define KEY_MQTT_USER "mqtt_user"
#define KEY_MQTT_PASSWORD "mqtt_password"
#define KEY_MQTT_STATE_TOPIC "mqtt_state_topic"
#define KEY_MQTT_COMMAND_TOPIC "mqtt_command_topic"
#define KEY_MQTT_LWT_TOPIC "mqtt_lwt_topic"
#define KEY_MQTT_HA_USE_DISCOVERY "switch_ha_discovery"
#define KEY_MQTT_HA_IS_DISCOVERED "mqtt_ha_is_discovered"
#define KEY_MQTT_HA_DISCOVERY_PREFIX "mqtt_ha_discovery_prefix"
#define KEY_REST_API_ENABLED "switch_rest_api"
#define KEY_REST_API_KEY "api_key"
#define KEY_POWERUP_MODE "powerup_mode"

// MQTT Event type definitions
#define MQTT_EVENT_CONNECT 0
#define MQTT_EVENT_DISCONNECT 1
#define MQTT_EVENT_MESSAGE 2

// HTTP
#define HTTP_WEB_INDEX "index.html"
#define HTTP_API_ROOT "api"
#define HTTP_HEADER_APIKEY "API-Key"
#define HTTP_HEADER_SERVER "Server"
#define HTTP_HEADER_CONTENTTYPE "Content-Type"
#define HTTP_HEADER_ALLOW "Allow"
#define HTTP_MIMETYPE_HTML "text/html"
#define HTTP_MIMETYPE_JSON "application/json"

const char *SERVER_SIGNATURE = APP_NAME "/" APP_VERSION;

const char *HTTP_ROUTE_INDEX = "/" HTTP_WEB_INDEX;
const char *HTTP_APIROUTE_ROOT = "/" HTTP_API_ROOT;
const char *HTTP_APIROUTE_ABOUT = "/" HTTP_API_ROOT "/about";
const char *HTTP_APIROUTE_LIGHT = "/" HTTP_API_ROOT "/light";

AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");
AsyncWebServer *server;
AsyncMqttClient mqtt;
std::vector<void (*)(uint8_t, const char *, const char *)> _mqtt_callbacks;
Ticker wifiReconnectTimer;
Ticker mqttReconnectTimer;

// Configuration structure that gets stored to the EEPROM
struct config_t {
  uint8_t ic;                 // Initialization check
  bool is_on;                 // Operational state (true == on)
  uint8_t brightness;         // Brightness level
  uint8_t color_temp;         // Colour temperature
  Color color;                // RGBW channel levels
  uint16_t mqtt_port;         // MQTT Broker port
  char hostname[128];         // Hostname/Identifier
  char wifi_ssid[32];         // WiFi SSID
  char wifi_psk[63];          // WiFi Passphrase Key
  char mqtt_server[128];      // Server/hostname of the MQTT Broker
  char mqtt_user[64];         // Username used for connecting to the MQTT Broker
  char mqtt_password[64];     // Password used for connecting to the MQTT Broker
  char mqtt_state_topic[128]; // MQTT Topic for publishing the state
  char mqtt_command_topic[128]; // MQTT Topic for receiving commands
  char mqtt_lwt_topic[128]; // MQTT Topic for publising Last Will and Testament
  bool gamma;               // Gamma Correction enabled or not
  bool mqtt_ha_use_discovery; // Home Assistant MQTT discovery enabled or not
  bool mqtt_ha_is_discovered; // Has this device already been discovered or not
  char mqtt_ha_disc_prefix[32]; // MQTT Discovery prefix for Home Assistant
  bool api;                     // REST API enabled or not
  char api_key[32];             // API Key
  uint8_t powerup_mode;         // Power Up Mode
} cfg;

// Globals for flash
bool flash = false;
bool startFlash = false;
uint16_t flashLength = 0;
uint32_t flashStartTime = 0;
Color flashColor;
uint8_t flashBrightness = 0;

// Globals for current state
Color currentColor;
uint8_t currentBrightness;
bool currentState;

// Globals for transition/fade
bool state = false;
uint16_t transitionTime = 0;
uint32_t startTransTime = 0;
int stepR, stepG, stepB, stepW, stepBrightness = 0;
uint16_t stepCount = 0;
Color transColor;
uint8_t transBrightness = 0;

#ifdef DEBUG
#define SerialPrint(format, ...)                                               \
  StreamPrint_progmem(Serial, PSTR(format), ##__VA_ARGS__)
#define StreamPrint(stream, format, ...)                                       \
  StreamPrint_progmem(stream, PSTR(format), ##__VA_ARGS__)
#endif

#ifdef DEBUG
#define DEBUGLOG(...) SerialPrint(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

#ifdef DEBUG
/**
 * @brief A program memory version of printf
 *
 * Copy of format string and result share a buffer so as to avoid too much
 * memory use.
 *
 * Credits:  David Pankhurst
 * Source: http://www.utopiamechanicus.com/article/low-memory-serial-print/
 *
 * @param  out the output object (e.g. Serial)
 * @param  format the format string (as used in the printf function and alike)
 * @return void
 */
void StreamPrint_progmem(Print &out, PGM_P format, ...) {
  char formatString[128], *ptr;

  // Copy in from program mem
  strncpy_P(formatString, format, sizeof(formatString));

  // null terminate - leave last char since we might need it in worst case for
  // results \0
  formatString[sizeof(formatString) - 2] = '\0';
  ptr = &formatString[os_strlen(formatString) + 1]; // our result buffer...

  va_list args;
  va_start(args, format);
  vsnprintf(ptr, sizeof(formatString) - 1 - os_strlen(formatString),
            formatString, args);
  va_end(args);
  formatString[sizeof(formatString) - 1] = '\0';

  out.print(ptr);
}
#endif

/**
 * @brief Template to allow any type of data to be written to the EEPROM
 *
 * Although this template allows any type of data, it will primarily be used to
 * save the config_t struct.
 *
 * @param  value the data to be written to the EEPROM
 *
 * @return void
 */
template <class T> void EEPROM_write(const T &value) {
  uint16_t ee = EEPROM_START_ADDRESS;
  const byte *p = (const byte *)(const void *)&value;
  for (uint16_t i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);

  EEPROM.commit();
}

/**
 * @brief Template to allow any type of data to be read from the EEPROM
 *
 * Although this template allows any type of data, it will primarily be used to
 * read the EEPROM contents into the config_t struct.
 *
 * @param  value the data to be loaded into from the EEPROM
 *
 * @return void
 */
template <class T> void EEPROM_read(T &value) {
  uint16_t ee = EEPROM_START_ADDRESS;
  byte *p = (byte *)(void *)&value;
  for (uint16_t i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
}
