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

#include "main.h"

/**
 * @brief Determines the ID of this Ai-Thinker RGBW Light
 *
 * @return the unique identifier of this Ai-Thinker RGBW Light
 */
const char *getDeviceID() {
  char *identifier = new char[30];
  strcpy(identifier, HOSTNAME);
  strcat_P(identifier, PSTR("-"));

  char cidBuf[7];
  sprintf(cidBuf, "%06X", ESP.getChipId());
  strcat(identifier, cidBuf);

  return identifier;
}

/**
 * @brief Loads the factory defaults for this Ai-Thinker RGBW Light
 *
 * If you like to change 'your' factory defaults, please change the appropriate
 * settings in your config.h file.
 *
 * @return void
 */
void loadFactoryDefaults() {
  // Light defaults
  cfg.is_on = LIGHT_STATE;
  cfg.brightness = LIGHT_BRIGHTNESS;
  cfg.color_temp = LIGHT_COLOR_TEMPERATURE;
  cfg.color = {LIGHT_COLOR_RED, LIGHT_COLOR_GREEN, LIGHT_COLOR_BLUE,
               LIGHT_COLOR_WHITE};

  // Device defaults
  strcpy(cfg.hostname, getDeviceID());
  cfg.mqtt_port = MQTT_PORT;
  strcpy(cfg.mqtt_server, MQTT_SERVER);
  strcpy(cfg.mqtt_user, MQTT_USER);
  strcpy(cfg.mqtt_password, MQTT_PASSWORD);
  strcpy(cfg.mqtt_state_topic, getDeviceID());

  char *cmd_topic = new char[64];
  sprintf_P(cmd_topic, PSTR("%s/set"), getDeviceID());
  strcpy(cfg.mqtt_command_topic, cmd_topic);

  strcpy(cfg.wifi_ssid, WIFI_SSID);
  strcpy(cfg.wifi_psk, WIFI_PSK);

  EEPROM_write(cfg);
}

/**
 * @brief Bootstrap/Initialization
 */
void setup() {
  EEPROM.begin(SPI_FLASH_SEC_SIZE);
  EEPROM_read(cfg);

// Serial Port Initialization
#ifdef DEBUG
  Serial.begin(115200);
  DEBUGLOG("\n");
  DEBUGLOG("\n");

  DEBUGLOG("Welcome to %s!\n", APP_NAME);
  DEBUGLOG("Firmware Version : %s\n", APP_VERSION);
  DEBUGLOG("Firmware Build   : %s - %s\n", __DATE__, __TIME__);
  DEBUGLOG("Device Name      : %s\n", cfg.hostname);
  DEBUGLOG("\n");
#endif

  setupLight();
  setupWiFi();
  setupOTA();
  setupMQTT();
  setupWeb();

#ifdef DEBUG
  wifiStatus();
#endif

  sendState(); // Notify subscribers about current state
}

/**
 * @brief Main loop
 */
void loop() {
  loopMQTT();
  loopOTA();
  loopLight();
}
