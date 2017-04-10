/**
 * Ai-Thinker RGBW Light Firmware - Light Module
 *
 * The Light module contains all the code to process incoming commands and set
 * the light attributes (RGBW, brightness, etc.) accordingly.
 *
 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * Copyright (c) 2017 Sacha Telgenhof
 */

#include "AiLight.hpp"
#include <ArduinoJson.h>

static const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);

// JSON Key names as used in Home Assistant
#define JSON_KEY_STATE "state"
#define JSON_KEY_BRIGHTNESS "brightness"
#define JSON_KEY_WHITE "white_value"
#define JSON_KEY_COLORTEMP "color_temp"
#define JSON_KEY_FLASH "flash"
#define JSON_KEY_COLOR "color"
#define JSON_KEY_COLOR_R "r"
#define JSON_KEY_COLOR_G "g"
#define JSON_KEY_COLOR_B "b"

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
    mqttSubscribe(MQTT_LIGHT_COMMAND_TOPIC);
  }

  // Handling the event of disconnecting from the MQTT broker
  if (type == MQTT_EVENT_DISCONNECT) {
    mqttUnsubscribe(MQTT_LIGHT_COMMAND_TOPIC);
  }

  // Handling the event a message is received from the MQTT broker
  if (type == MQTT_EVENT_MESSAGE) {

    // Listen to this lights command topic
    if (strcmp(topic, MQTT_LIGHT_COMMAND_TOPIC) == 0) {

      // Convert payload into char variable
      uint8_t length = strlen(payload);
      char message[length + 1];
      memcpy(message, payload, length);
      message[length] = 0;

      if (!processJson(message)) {
        return;
      }

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

  root[JSON_KEY_STATE] =
      AiLight.getState() ? MQTT_PAYLOAD_ON : MQTT_PAYLOAD_OFF;
  root[JSON_KEY_BRIGHTNESS] = AiLight.getBrightness();
  root[JSON_KEY_WHITE] = AiLight.getColor().white;
  root[JSON_KEY_COLORTEMP] = AiLight.getColorTemperature();

  JsonObject &color = root.createNestedObject(JSON_KEY_COLOR);
  color[JSON_KEY_COLOR_R] = AiLight.getColor().red;
  color[JSON_KEY_COLOR_G] = AiLight.getColor().green;
  color[JSON_KEY_COLOR_B] = AiLight.getColor().blue;

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  mqttPublish(MQTT_LIGHT_STATE_TOPIC, buffer);
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
  if (root.containsKey(JSON_KEY_FLASH)) {

    // Save current settings to be restored later
    currentColor = AiLight.getColor();
    currentBrightness = AiLight.getBrightness();
    currentState = AiLight.getState();

    flashLength = (uint16_t)root[JSON_KEY_FLASH] * 1000U;

    flashBrightness = (root.containsKey(JSON_KEY_BRIGHTNESS))
                          ? root[JSON_KEY_BRIGHTNESS]
                          : currentBrightness;

    if (root.containsKey(JSON_KEY_COLOR)) {
      flashColor = {root[JSON_KEY_COLOR][JSON_KEY_COLOR_R],
                    root[JSON_KEY_COLOR][JSON_KEY_COLOR_G],
                    root[JSON_KEY_COLOR][JSON_KEY_COLOR_B]};
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

  if (root.containsKey(JSON_KEY_BRIGHTNESS)) {
    AiLight.setBrightness(root[JSON_KEY_BRIGHTNESS]);
  }

  if (root.containsKey(JSON_KEY_COLOR)) {
    AiLight.setColor(root[JSON_KEY_COLOR][JSON_KEY_COLOR_R],
                     root[JSON_KEY_COLOR][JSON_KEY_COLOR_G],
                     root[JSON_KEY_COLOR][JSON_KEY_COLOR_B]);
  }

  if (root.containsKey(JSON_KEY_WHITE)) {
    AiLight.setWhite(root[JSON_KEY_WHITE]);
  }

  if (root.containsKey(JSON_KEY_COLORTEMP)) {
    AiLight.setColorTemperature(root[JSON_KEY_COLORTEMP]);
  }

  if (root.containsKey(JSON_KEY_STATE)) {
    if (strcmp(root[JSON_KEY_STATE], MQTT_PAYLOAD_ON) == 0) {
      AiLight.setState(true);
    } else if (strcmp(root[JSON_KEY_STATE], MQTT_PAYLOAD_OFF) == 0) {
      AiLight.setState(false);
    }
  }

  return true;
}

/**
 * @brief Bootstrap function for the RGBW light
 */
void setupLight() { mqttRegister(lightMQTTCallback); }

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
