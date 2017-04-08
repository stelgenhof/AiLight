/**
 * Ai-Thinker RGBW Light Firmware - MQTT Module
 *
 * The MQTT module holds all the code to manage all functions for communicating
 * with the MQTT broker.
 *
 * Parts of the code inspired by the Espurna firmware from Xose Pérez
 * (https://bitbucket.org/xoseperez/espurna)
 *
 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <vector>

// MQTT Event type definitions
#define MQTT_EVENT_CONNECT 0
#define MQTT_EVENT_DISCONNECT 1
#define MQTT_EVENT_MESSAGE 2

WiFiClient client;
PubSubClient mqtt(client);
boolean _mqtt_connected = false;
std::vector<void (*)(uint8_t, const char *, const char *)> _mqtt_callbacks;

// ---------------------------
// API functions
// ---------------------------

/**
 * @brief Publish a message to an MQTT topic
 *
 * @param topic the MQTT topic to publish the message to
 * @param message the message to be published
 */
void mqttPublish(const char *topic, const char *message) {

  // Don't do anything if we are not connected to the MQTT broker
  if (!mqtt.connected()) {
    return;
  }

  if ((strlen(topic) > 0) && (strlen(message) > 0)) {
    mqtt.publish(topic, message, MQTT_RETAIN);

#ifdef DEBUG
    DEBUGLOG("[MQTT] Published message '%s' to '%s'\n", message, topic);
#endif
  }
}

/**
 * @brief Subscribe to an MQTT topic
 *
 * @param topic the MQTT topic to subscribe to
 * @param qos the desired QoS level (defaults to MQTT_QOS_LEVEL)
 */
void mqttSubscribe(const char *topic, uint8_t qos = MQTT_QOS_LEVEL) {

  // Don't do anything if we are not connected to the MQTT broker
  if (!mqtt.connected()) {
    return;
  }

  if (strlen(topic) > 0) {
    mqtt.subscribe(topic, qos);

#ifdef DEBUG
    DEBUGLOG("[MQTT] Subscribed to topic '%s'\n", topic);
#endif
  }
}

/**
 * @brief Unsubscribe from an MQTT topic
 *
 * @param topic the MQTT topic to unsubscribe from
 */
void mqttUnsubscribe(const char *topic) {

  // Don't do anything if we are not connected to the MQTT broker
  if (!mqtt.connected()) {
    return;
  }

  if (strlen(topic) > 0) {
    mqtt.unsubscribe(topic);

#ifdef DEBUG
    DEBUGLOG("[MQTT] Unsubscribed from topic '%s'\n", topic);
#endif
  }
}

/**
 * @brief Register MQTT callback functions
 *
 * @param the callback function to register
 */
void mqttRegister(void (*callback)(uint8_t, const char *, const char *)) {
  _mqtt_callbacks.push_back(callback);
}

// ---------------------------
// Internal functions
// ---------------------------

/**
 * @brief Event handler for when a connection to the MQTT has been establised.
 */
void _mqttOnConnect() {
#ifdef DEBUG
  DEBUGLOG("[MQTT] Connected\n");
#endif

  // Notify subscribers (connected)
  for (uint8_t i = 0; i < _mqtt_callbacks.size(); i++) {
    (*_mqtt_callbacks[i])(MQTT_EVENT_CONNECT, NULL, NULL);
  }
}

/**
 * @brief Event handler for when the connection to the MQTT broker has been
 * disconnected.
 */
void _mqttOnDisconnect() {
#ifdef DEBUG
  DEBUGLOG("[MQTT] Disconnected\n");
#endif

  // Notify subscribers (disconnected)
  for (uint8_t i = 0; i < _mqtt_callbacks.size(); i++) {
    (*_mqtt_callbacks[i])(MQTT_EVENT_DISCONNECT, NULL, NULL);
  }
}

/**
 * @brief Event handler for when a message is received from the MTT broker
 *
 * @param topic the MQTT topic to which the message has been published
 * @param payload the contents/message that has been published
 * @param length the length of the published message
 */
void _mqttOnMessage(char *topic, char *payload, uint8_t length) {

  // Don't do anything if we are not connected to the MQTT broker
  if (!mqtt.connected()) {
    return;
  }

  // Convert payload into char variable
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = 0;

#ifdef DEBUG
  DEBUGLOG("[MQTT] Received '%s'-'%s'\n", topic, message);
#endif

  // Notify subscribers (message received)
  for (uint8_t i = 0; i < _mqtt_callbacks.size(); i++) {
    (*_mqtt_callbacks[i])(MQTT_EVENT_MESSAGE, topic, message);
  }
}

/**
 * @brief Handles the connection to the MQTT broker.
 */
void _mqttConnect() {

  // Return if MQTT is not enabled
  if (!MQTT_ENABLED) {
    return;
  }

  bool response = false;

  // Try to make a connection to the MQTT broker
  if (!mqtt.connected()) {

#ifdef DEBUG
    DEBUGLOG("[MQTT] Connecting to broker '%s:%i'", MQTT_SERVER, MQTT_PORT);
#endif

    mqtt.setServer(MQTT_SERVER, MQTT_PORT);

    if ((strlen(MQTT_USER) > 0) && (strlen(MQTT_PASSWORD) > 0)) {
      DEBUGLOG(" as user '%s'\n", MQTT_USER);
      response = mqtt.connect(getDeviceID(), MQTT_USER, MQTT_PASSWORD);
    } else {
      DEBUGLOG("\n");
      response = mqtt.connect(getDeviceID());
    }
  }

  if (response) {
    _mqttOnConnect();
    _mqtt_connected = true;
  } else {

#ifdef DEBUG
    DEBUGLOG("[MQTT] Connection failed (rc=%d)\n", mqtt.state());
#endif
  }
}

// ---------------------------
// Core functions
// ---------------------------

/**
 * @brief Bootstrap function for the MQTT connection
 */
void setupMQTT() {

  // Return if MQTT is not enabled
  if (!MQTT_ENABLED) {
    return;
  }

#ifdef DEBUG
  DEBUGLOG("[MQTT] Initialization...\n");
#endif

  mqtt.setCallback([](char *topic, byte *payload, uint8_t length) {
    _mqttOnMessage(topic, (char *)payload, length);
  });

  _mqttConnect(); // Make connection
}

/**
 * @brief Listen to MQTT requests (pub/sub)
 */
void loopMQTT() {

  // Return if MQTT is not enabled
  if (!MQTT_ENABLED) {
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!mqtt.connected()) {
      if (_mqtt_connected) {
        _mqtt_connected = false;
      }

      // Wait 10 seconds before trying to connect again
      unsigned long currPeriod = millis() / MQTT_RECONNECT_TIME;
      static unsigned long lastPeriod = 0;
      if (currPeriod != lastPeriod) {
        lastPeriod = currPeriod;
        _mqttConnect();
      }
    }

    if (mqtt.connected()) {
      mqtt.loop();
    }
  }
}
