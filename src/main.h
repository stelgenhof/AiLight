/**
 * Ai-Thinker RGBW Light Firmware
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.

 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */

#define APP_NAME "AiLight"
#define APP_VERSION "0.3.0"
#define APP_AUTHOR "stelgenhof@gmail.com"

#include "config.h"

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
#include <WiFiUdp.h>
#include <vector>

extern "C" {
#include "spi_flash.h"
}

#include "html.gz.h"

#define EEPROM_START_ADDRESS 0
#define INIT_HASH 0x4B

// Key names as used internally and in Home Assistant
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
#define KEY_HOSTNAME "hostname"
#define KEY_GAMMA_CORRECTION "gamma"
#define KEY_WIFI_SSID "wifi_ssid"
#define KEY_WIFI_PSK "wifi_psk"
#define KEY_MQTT_SERVER "mqtt_server"
#define KEY_MQTT_PORT "mqtt_port"
#define KEY_MQTT_USER "mqtt_user"
#define KEY_MQTT_PASSWORD "mqtt_password"
#define KEY_MQTT_STATE_TOPIC "mqtt_state_topic"
#define KEY_MQTT_COMMAND_TOPIC "mqtt_command_topic"
#define KEY_MQTT_LWT_TOPIC "mqtt_lwt_topic"

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
} cfg;

#define SerialPrint(format, ...)                                               \
  StreamPrint_progmem(Serial, PSTR(format), ##__VA_ARGS__)
#define StreamPrint(stream, format, ...)                                       \
  StreamPrint_progmem(stream, PSTR(format), ##__VA_ARGS__)

#ifdef DEBUG
#define DEBUGLOG(...) SerialPrint(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

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
  ptr = &formatString[strlen(formatString) + 1]; // our result buffer...

  va_list args;
  va_start(args, format);
  vsnprintf(ptr, sizeof(formatString) - 1 - strlen(formatString), formatString,
            args);
  va_end(args);
  formatString[sizeof(formatString) - 1] = '\0';

  out.print(ptr);
}

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
  int ee = EEPROM_START_ADDRESS;
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
  int ee = EEPROM_START_ADDRESS;
  byte *p = (byte *)(void *)&value;
  for (uint16_t i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
}
