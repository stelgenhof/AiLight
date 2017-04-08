/**
 * Ai-Thinker RGBW Light Firmware
 *
 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * Copyright (c) 2017 Sacha Telgenhof
 */

#include "config.h"
#include "version.h"

/**
 * @brief Determines the ID of this Ai-Thinker RGBW Light
 *
 * @return the unique identifier of this Ai-Thinker RGBW Light
 */
const char *getDeviceID() {
  char *identifier = new char[30];
  strcpy(identifier, DEVICE_ID);
  strcat_P(identifier, PSTR("_"));

#ifdef ESP8266
  // Get ESP Chip ID and change into char array
  char cidBuf[7];
  sprintf(cidBuf, "%d", ESP.getChipId());
  strcat(identifier, cidBuf);
#endif

  return identifier;
}

/**
 * @brief Bootstrap/Initialization
 */
void setup() {

// Serial Port Initialization
#ifdef DEBUG
  Serial.begin(115200);
  Serial.setDebugOutput(true);
#endif

  setupLight();
  setupWiFi();
  setupOTA();
  setupMQTT();

#ifdef DEBUG
  DEBUGLOG("%s - %s\n", APP_NAME, APP_VERSION);
  DEBUGLOG("%s\n", getDeviceID());
  DEBUGLOG("\n")
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
