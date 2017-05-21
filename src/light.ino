/**
 * Ai-Thinker RGBW Light Firmware - Light Module
 *
 * The Light module contains all the code to process incoming commands and set
 * the light attributes (RGBW, brightness, etc.) accordingly.
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.

 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */

/**
 * @brief Publish a message to an MQTT topic
 *
 * @param type the MQTT event type (e.g. 'connect', 'message', etc.)
 * @param topic the MQTT topic to which the message has been published
 * @param payload the contents/message that has been published
 */
void lightMQTTCallback(uint8_t type, const char *topic, const char *payload) {

  // Handling the event of connecting to the MQTT broker
  if (type == MQTT_EVENT_CONNECT) {
    mqttSubscribe(cfg.mqtt_command_topic);
    mqttPublish(cfg.mqtt_lwt_topic, MQTT_STATUS_ONLINE);
  }

  // Handling the event of disconnecting from the MQTT broker
  if (type == MQTT_EVENT_DISCONNECT) {
    mqttUnsubscribe(cfg.mqtt_command_topic);
  }

  // Handling the event a message is received from the MQTT broker
  if (type == MQTT_EVENT_MESSAGE) {

    // Listen to this lights' MQTT command topic
    if (strcmp(topic, cfg.mqtt_command_topic) == 0) {

      // Convert payload into char variable
      uint8_t length = strlen(payload);
      char message[length + 1];
      memcpy(message, payload, length);
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
      sendState(); // Notify subscribers about new state
    }
  }
}

/**
 * @brief Publish the current state of the AiLight
 */
void sendState() {
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root[KEY_STATE] = AiLight.getState() ? MQTT_PAYLOAD_ON : MQTT_PAYLOAD_OFF;
  root[KEY_BRIGHTNESS] = AiLight.getBrightness();
  root[KEY_WHITE] = AiLight.getColor().white;
  root[KEY_COLORTEMP] = AiLight.getColorTemperature();

  JsonObject &color = root.createNestedObject(KEY_COLOR);
  color[KEY_COLOR_R] = AiLight.getColor().red;
  color[KEY_COLOR_G] = AiLight.getColor().green;
  color[KEY_COLOR_B] = AiLight.getColor().blue;

  root[KEY_GAMMA_CORRECTION] = AiLight.hasGammaCorrection();

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

  if (root.containsKey(KEY_BRIGHTNESS)) {
    AiLight.setBrightness(root[KEY_BRIGHTNESS]);
  }

  if (root.containsKey(KEY_COLOR)) {
    AiLight.setColor(root[KEY_COLOR][KEY_COLOR_R], root[KEY_COLOR][KEY_COLOR_G],
                     root[KEY_COLOR][KEY_COLOR_B]);
  }

  if (root.containsKey(KEY_WHITE)) {
    AiLight.setWhite(root[KEY_WHITE]);
  }

  if (root.containsKey(KEY_COLORTEMP)) {
    AiLight.setColorTemperature(root[KEY_COLORTEMP]);
  }

  if (root.containsKey(KEY_STATE)) {
    if (strcmp(root[KEY_STATE], MQTT_PAYLOAD_ON) == 0) {
      AiLight.setState(true);
    } else if (strcmp(root[KEY_STATE], MQTT_PAYLOAD_OFF) == 0) {
      AiLight.setState(false);
    }
  }

  return true;
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
  AiLight.setState(cfg.is_on);

  mqttRegister(lightMQTTCallback);
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
}
