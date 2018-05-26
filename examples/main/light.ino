/**
 * Ai-Thinker RGBW Light Firmware - Light Module
 *
 * The Light module contains all the code to process incoming commands and set
 * the light attributes (RGBW, brightness, etc.) accordingly.
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * Created by Sacha Telgenhof <me at sachatelgenhof dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2018 Sacha Telgenhof
 */

/**
 * @brief Handle the various MQTT Events (Connect, Disconnect, etc.)
 *
 * @param type the MQTT event type (e.g. 'connect', 'message', etc.)
 * @param topic the MQTT topic to which the message has been published
 * @param payload the contents/message that has been published
 */
void deviceMQTTCallback(uint8_t type, const char *topic, const char *payload) {

  // Handling the event of connecting to the MQTT broker
  if (type == MQTT_EVENT_CONNECT) {
    mqttSubscribe(cfg.mqtt_command_topic);
    mqttPublish(cfg.mqtt_lwt_topic, MQTT_STATUS_ONLINE);

    // MQTT discovery for Home Assistant
    if (cfg.mqtt_ha_use_discovery && !cfg.mqtt_ha_is_discovered) {
      static const int BUFFER_SIZE =
          JSON_OBJECT_SIZE(9) + 128; // '128' is an arbritrary number. Increase
                                     // if required by the payload
      StaticJsonBuffer<BUFFER_SIZE> mqttJsonBuffer;
      JsonObject &md_root = mqttJsonBuffer.createObject();

      md_root["name"] = cfg.hostname;
      md_root["platform"] = "mqtt_json";
      md_root["state_topic"] = cfg.mqtt_state_topic;
      md_root["command_topic"] = cfg.mqtt_command_topic;
      md_root["rgb"] = true;
      md_root[KEY_COLORTEMP] = true;
      md_root[KEY_BRIGHTNESS] = true;
      md_root[KEY_WHITE] = true;
      md_root["availability_topic"] = cfg.mqtt_lwt_topic;

      // Build the payload
      char md_buffer[md_root.measureLength() + 1];
      md_root.printTo(md_buffer, sizeof(md_buffer));

      // Construct the topic name for HA MQTT discovery
      char *dc_topic = new char[128];
      sprintf_P(dc_topic, PSTR("%s/light/%s/config"), cfg.mqtt_ha_disc_prefix,
                cfg.hostname);

      mqttPublish(dc_topic, md_buffer);

      cfg.mqtt_ha_is_discovered = true;
      EEPROM_write(cfg);
    }
  }

  // Handling the event of disconnecting from the MQTT broker
  if (type == MQTT_EVENT_DISCONNECT) {
    mqttUnsubscribe(cfg.mqtt_command_topic);

    if (WiFi.isConnected()) {
      mqttReconnectTimer.once(RECONNECT_TIME, mqttConnect);
    }
  }

  // Handling the event a message is received from the MQTT broker
  if (type == MQTT_EVENT_MESSAGE) {

    // Listen to this lights' MQTT command topic
    if (os_strcmp(topic, cfg.mqtt_command_topic) == 0) {

      // Convert payload into char variable
      uint8_t length = os_strlen(payload);
      char message[length + 1];
      os_memcpy(message, payload, length);
      message[length] = 0;

      if (!processJson(message)) {
        return;
      }

      // Store light parameters for persistance
      cfg.is_on = AiLight.getState();
      cfg.brightness = AiLight.getBrightness();
      cfg.color_temp = AiLight.getColorTemperature();
      cfg.color = {AiLight.getColor().red, AiLight.getColor().green,
                   AiLight.getColor().blue, AiLight.getColor().white};

      EEPROM_write(cfg);
      sendState(); // Notify subscribers about the new state
    }
  }
}

/**
 * @brief Publish the current state of the AiLight
 */
void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  createStateJSON(root);

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  mqttPublish(cfg.mqtt_state_topic, buffer); // Notify all MQTT subscribers
  ws.textAll(buffer);                        // Notify all WebSocket clients
}

/**
 * @brief Process the received JSON payload
 */
bool processJson(char *message) {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    DEBUGLOG("[LIGHT] Unable to parse message\n");
    return false;
  }

  // Flash
  if (root.containsKey(KEY_FLASH)) {

    // Save current settings to be restored later
    currentColor = AiLight.getColor();
    currentBrightness = AiLight.getBrightness();
    currentState = AiLight.getState();

    flashLength = (uint16_t)root[KEY_FLASH] * 1000U;

    flashBrightness = (root.containsKey(KEY_BRIGHTNESS)) ? root[KEY_BRIGHTNESS]
                                                         : currentBrightness;

    if (root.containsKey(KEY_COLOR)) {
      flashColor = {root[KEY_COLOR][KEY_COLOR_R], root[KEY_COLOR][KEY_COLOR_G],
                    root[KEY_COLOR][KEY_COLOR_B]};
    } else {
      flashColor = {currentColor.red, currentColor.green, currentColor.blue};
    }

    flashColor.red = map(flashColor.red, 0, 255, 0, flashBrightness);
    flashColor.green = map(flashColor.green, 0, 255, 0, flashBrightness);
    flashColor.blue = map(flashColor.blue, 0, 255, 0, flashBrightness);

    flash = true;
    startFlash = true;
  } else {
    flash = false;
  }

  if (root.containsKey(KEY_TRANSITION)) {
    transitionTime = root[KEY_TRANSITION]; // Time in seconds
    startTransTime = millis();
  } else {
    transitionTime = 0;
  }

  if (root.containsKey(KEY_BRIGHTNESS)) {

    // In transition/fade
    if (transitionTime > 0) {
      transBrightness = root[KEY_BRIGHTNESS];

      // If light is off, start fading from Zero
      if (!AiLight.getState()) {
        AiLight.setBrightness(0);
      }

      stepBrightness = calculateStep(AiLight.getBrightness(), transBrightness);
      stepCount = 0;
    } else {
      AiLight.setBrightness(root[KEY_BRIGHTNESS]);
    }
  }

  if (root.containsKey(KEY_COLOR)) {

    // In transition/fade
    if (transitionTime > 0) {
      transColor.red = root[KEY_COLOR][KEY_COLOR_R];
      transColor.green = root[KEY_COLOR][KEY_COLOR_G];
      transColor.blue = root[KEY_COLOR][KEY_COLOR_B];

      // If light is off, start fading from Zero
      if (!AiLight.getState()) {
        AiLight.setColor(0, 0, 0);
      }

      stepR = calculateStep(AiLight.getColor().red, transColor.red);
      stepG = calculateStep(AiLight.getColor().green, transColor.green);
      stepB = calculateStep(AiLight.getColor().blue, transColor.blue);

      stepCount = 0;
    } else {
      AiLight.setColor(root[KEY_COLOR][KEY_COLOR_R],
                       root[KEY_COLOR][KEY_COLOR_G],
                       root[KEY_COLOR][KEY_COLOR_B]);
    }
  }

  if (root.containsKey(KEY_WHITE)) {
    // In transition/fade
    if (transitionTime > 0) {
      transColor.white = root[KEY_WHITE];

      // If light is off, start fading from Zero
      if (!AiLight.getState()) {
        AiLight.setWhite(0);
      }

      stepW = calculateStep(AiLight.getColor().white, transColor.white);

      stepCount = 0;
    } else {
      AiLight.setWhite(root[KEY_WHITE]);
    }
  }

  if (root.containsKey(KEY_COLORTEMP)) {
    // In transition/fade
    if (transitionTime > 0) {
      transColor = AiLight.colorTemperature2RGB(root[KEY_COLORTEMP]);

      // If light is off, start fading from Zero
      if (!AiLight.getState()) {
        AiLight.setColor(0, 0, 0);
      }

      stepR = calculateStep(AiLight.getColor().red, transColor.red);
      stepG = calculateStep(AiLight.getColor().green, transColor.green);
      stepB = calculateStep(AiLight.getColor().blue, transColor.blue);

      stepCount = 0;
    } else {
      AiLight.setColorTemperature(root[KEY_COLORTEMP]);
    }
  }

  if (root.containsKey(KEY_STATE)) {
    state = (os_strcmp(root[KEY_STATE], MQTT_PAYLOAD_ON) == 0) ? true : false;

    if (transitionTime > 0 && !state) {
      transColor.red = 0;
      transColor.green = 0;
      transColor.blue = 0;

      stepR = calculateStep(AiLight.getColor().red, transColor.red);
      stepG = calculateStep(AiLight.getColor().green, transColor.green);
      stepB = calculateStep(AiLight.getColor().blue, transColor.blue);
    } else {
      AiLight.setState(state);
    }
  }

  if (root.containsKey(KEY_GAMMA_CORRECTION)) {
    bool use_gamma_correction = root[KEY_GAMMA_CORRECTION];
    AiLight.useGammaCorrection(use_gamma_correction);
  }

  return true;
}

/**
 * @brief Populate the given JsonObject with details about this firmware
 *
 * @param object the JsonObject that will hold details about this firmware
 *
 * @return void
 */
void createAboutJSON(JsonObject &object) {
  object["app_name"] = APP_NAME;
  object["app_version"] = APP_VERSION;
  object["build_date"] = __DATE__;
  object["build_time"] = __TIME__;
  object["memory"] = ESP.getFlashChipSize();
  object["free_heap"] = ESP.getFreeHeap();
  object["cpu_frequency"] = ESP.getCpuFreqMHz();
  object["manufacturer"] = DEVICE_MANUFACTURER;
  object["model"] = DEVICE_MODEL;
  object["device_ip"] = (WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP().toString()
                                                    : WiFi.localIP().toString();
  object["mac"] = WiFi.macAddress();
}

/**
 * @brief Populate the given JsonObject with the current state of this light
 *
 * @param object the JsonObject that will hold the current state of this light
 */
void createStateJSON(JsonObject &object) {
  object[KEY_STATE] = AiLight.getState() ? MQTT_PAYLOAD_ON : MQTT_PAYLOAD_OFF;
  object[KEY_BRIGHTNESS] = AiLight.getBrightness();
  object[KEY_WHITE] = AiLight.getColor().white;
  object[KEY_COLORTEMP] = AiLight.getColorTemperature();

  JsonObject &color = object.createNestedObject(KEY_COLOR);
  color[KEY_COLOR_R] = AiLight.getColor().red;
  color[KEY_COLOR_G] = AiLight.getColor().green;
  color[KEY_COLOR_B] = AiLight.getColor().blue;

  object[KEY_GAMMA_CORRECTION] = AiLight.hasGammaCorrection();
}

/**
 * @brief Bootstrap function for the RGBW light
 */
void setupLight() {

  // Restore last used settings (Note: set colour temperature first as it
  // changed the RGB channels!)
  AiLight.setColorTemperature(cfg.color_temp);
  AiLight.setColor(cfg.color.red, cfg.color.green, cfg.color.blue);
  AiLight.setWhite(cfg.color.white);
  AiLight.setBrightness(cfg.brightness);
  AiLight.useGammaCorrection(cfg.gamma);

  switch (cfg.powerup_mode) {
  case POWERUP_ON:
    AiLight.setState(true);
    break;
  case POWERUP_SAME:
    AiLight.setState(cfg.is_on);
    break;
  case POWERUP_OFF:
  default:
    AiLight.setState(false);
    break;
  }

  mqttRegister(deviceMQTTCallback);
}

/**
 * @brief Process requests and keep on running...
 */
void loopLight() {

  // Flashing
  if (flash) {
    if (startFlash) {
      startFlash = false;
      flashStartTime = millis();
      AiLight.setState(false);
    }

    // Run the flash sequence for the defined period.
    if ((millis() - flashStartTime) <= (flashLength - 100U)) {
      if ((millis() - flashStartTime) % 1000 <= 500) {
        AiLight.setColor(flashColor.red, flashColor.green, flashColor.blue);
        AiLight.setBrightness(flashBrightness);
        AiLight.setState(true);
      } else {
        AiLight.setState(false);
      }
    } else {
      // Return to the state before the flash
      flash = false;

      AiLight.setState(currentState);
      AiLight.setColor(currentColor.red, currentColor.green, currentColor.blue);
      AiLight.setBrightness(currentBrightness);

      sendState(); // Notify subscribers again about current state
    }
  }

  // Transitioning/Fading
  if (transitionTime > 0) {
    AiLight.setState(true);

    uint32_t currentTransTime = millis();

    // Cross fade the RGBW channels every millisecond
    if (currentTransTime - startTransTime > transitionTime) {
      if (stepCount <= 1000) {
        startTransTime = currentTransTime;

        // Transition/fade RGB LEDS (if level is different from current)
        if (stepR != 0 || stepG != 0 || stepB != 0) {
          AiLight.setColor(calculateLevel(stepR, AiLight.getColor().red,
                                          stepCount, transColor.red),
                           calculateLevel(stepG, AiLight.getColor().green,
                                          stepCount, transColor.green),
                           calculateLevel(stepB, AiLight.getColor().blue,
                                          stepCount, transColor.blue));
        }

        // Transition/fade white LEDS (if level is different from current)
        if (stepW != 0) {
          AiLight.setWhite(calculateLevel(stepW, AiLight.getColor().white,
                                          stepCount, transColor.white));
        }

        // Transition/fade brightness (if level is different from current)
        if (stepBrightness != 0) {
          AiLight.setBrightness(calculateLevel(stepBrightness,
                                               AiLight.getBrightness(),
                                               stepCount, transBrightness));
        }

        stepCount++;
      } else {
        transitionTime = 0;
        stepCount = 0;
        AiLight.setState(state);

        sendState(); // Notify subscribers again about current state

        // Update settings
        cfg.is_on = AiLight.getState();
        cfg.brightness = AiLight.getBrightness();
        cfg.color = {AiLight.getColor().red, AiLight.getColor().green,
                     AiLight.getColor().blue, AiLight.getColor().white};
        EEPROM_write(cfg);
      }
    }
  }
}

/**
 * @brief Determines the step needed to change to the target value
 *
 * @param currentLevel the current level
 * @param targetLevel the target level
 *
 * @return the step value needed to change to the target value
 */
int16_t calculateStep(uint8_t currentLevel, uint8_t targetLevel) {
  int16_t step = targetLevel - currentLevel;
  if (step) {
    step = 1000 / step;
  }

  return step;
}

/**
 * @brief Calculates the next level of a channel (RGBW/Brightness)
 *
 * @param step the step needed for changing to the target value
 * @param val the current value in the transitioning loop
 * @param i the current index in the transitioning loop
 * @param targetLevel the target level
 *
 * @return the next level of a channel (RGBW/Brightness)
 */
uint8_t calculateLevel(int step, int val, uint16_t i, uint8_t targetLevel) {
  if ((step) && i % step == 0) {
    if (step > 0) {
      val++;

      // Prevent overshooting the target level
      if (val > targetLevel) {
        val = targetLevel;
      }
    } else if (step < 0) {
      val--;

      // Prevent undershooting the target level
      if (val < targetLevel) {
        val = targetLevel;
      }
    }
  }

  val = constrain(val, 0, MY9291_LEVEL_MAX); // Force boundaries

  return val;
}
