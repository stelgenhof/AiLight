/**
 * Ai-Thinker RGBW Light Firmware - OTA Module
 *
 * The OTA (Over The Air) module holds all the code to manage over the air
 * firmware updates.
 *
 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */

#include "ArduinoOTA.h"

// ---------------------------
// Core functions
// ---------------------------

/**
 * @brief Bootstrap function for the OTA UDP service
 */
void setupOTA() {
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setHostname(DEVICE_ID);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() { DEBUGLOG("[OTA] Start\n"); });

  ArduinoOTA.onEnd([]() { DEBUGLOG("\n[OTA] End\n"); });

  ArduinoOTA.onProgress([](uint8_t progress, uint8_t total) {
    DEBUGLOG("[OTA] Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
#ifdef DEBUG
    DEBUGLOG("\n[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      DEBUGLOG("Authentication Failed\n");
    else if (error == OTA_BEGIN_ERROR)
      DEBUGLOG("Begin Failed\n");
    else if (error == OTA_CONNECT_ERROR)
      DEBUGLOG("Connection Failed\n");
    else if (error == OTA_RECEIVE_ERROR)
      DEBUGLOG("Receive Failed\n");
    else if (error == OTA_END_ERROR)
      DEBUGLOG("End Failed\n");
#endif
  });

  ArduinoOTA.begin();
}

/**
 * @brief Listen to OTA requests
 */
void loopOTA() { ArduinoOTA.handle(); }
