/**
 * Ai-Thinker RGBW Light Firmware
 *
 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * Copyright (c) 2017 Sacha Telgenhof
 */

#include "config.h"
#include "version.h"

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
