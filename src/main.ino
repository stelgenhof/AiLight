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

#include "main.h"

/**
 * @brief Determines the ID of this Ai-Thinker RGBW Light
 *
 * @return the unique identifier of this Ai-Thinker RGBW Light
 */
const char *getDeviceID() {
  char *identifier = new char[30];
  os_strcpy(identifier, HOSTNAME);
  strcat_P(identifier, PSTR("-"));

  char cidBuf[7];
  sprintf(cidBuf, "%06X", ESP.getChipId());
  os_strcat(identifier, cidBuf);

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
  // Clear EEPROM space
  for (uint16_t i = 0; i < SPI_FLASH_SEC_SIZE; i++) {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();

  // Device defaults
  cfg.ic = INIT_HASH;
  cfg.is_on = LIGHT_STATE;
  cfg.brightness = LIGHT_BRIGHTNESS;
  cfg.color_temp = LIGHT_COLOR_TEMPERATURE;
  cfg.color = {LIGHT_COLOR_RED, LIGHT_COLOR_GREEN, LIGHT_COLOR_BLUE,
               LIGHT_COLOR_WHITE};

  // Configuration defaults
  os_strcpy(cfg.hostname, getDeviceID());
  cfg.mqtt_port = MQTT_PORT;
  os_strcpy(cfg.mqtt_server, MQTT_SERVER);
  os_strcpy(cfg.mqtt_user, MQTT_USER);
  os_strcpy(cfg.mqtt_password, MQTT_PASSWORD);
  os_strcpy(cfg.mqtt_state_topic, getDeviceID());

  // MQTT Topics
  char *cmd_topic = new char[128];
  sprintf_P(cmd_topic, PSTR("%s/set"), getDeviceID());
  os_strcpy(cfg.mqtt_command_topic, cmd_topic);

  char *lwt_topic = new char[128];
  sprintf_P(lwt_topic, PSTR("%s/status"), getDeviceID());
  os_strcpy(cfg.mqtt_lwt_topic, lwt_topic);

  cfg.mqtt_ha_use_discovery = MQTT_HOMEASSISTANT_DISCOVERY_ENABLED;
  cfg.mqtt_ha_is_discovered = false;
  os_strcpy(cfg.mqtt_ha_disc_prefix, MQTT_HOMEASSISTANT_DISCOVERY_PREFIX);

  os_strcpy(cfg.wifi_ssid, WIFI_SSID);
  os_strcpy(cfg.wifi_psk, WIFI_PSK);

  // REST API
  cfg.api = REST_API_ENABLED;
  os_strcpy(cfg.api_key, ADMIN_PASSWORD);

  cfg.powerup_mode = POWERUP_MODE;

  EEPROM_write(cfg);
}

/**
 * @brief Bootstrap/Initialization
 */
void setup() {
  EEPROM.begin(SPI_FLASH_SEC_SIZE);
  EEPROM_read(cfg);
  if (cfg.ic != INIT_HASH) {
    loadFactoryDefaults();
  }

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
  setupMQTT();
  setupWiFi();
  setupOTA();
  setupWeb();

  sendState(); // Notify subscribers about current state
}

/**
 * @brief Main loop
 */
void loop() {
  loopOTA();
  loopLight();
}
