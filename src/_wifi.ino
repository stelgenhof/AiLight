/**
 * Ai-Thinker RGBW Light Firmware - WiFi Module
 *
 * The WiFi module holds all the code to manage all functions for setting up the
 * WiFi connection.
 *
 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

// ---------------------------
// Core functions
// ---------------------------

/**
 * @brief Bootstrap function for the WiFi connection
 */
void setupWiFi() {

  // Set WiFi module to STA mode and set Power Output
  WiFi.mode(WIFI_STA);
  WiFi.setOutputPower(WIFI_OUTPUT_POWER);

  // Connecting
  WiFi.begin(WIFI_SSID, WIFI_PSK);

#ifdef DEBUG
  DEBUGLOG("[WIFI] Connecting to %s\n", WIFI_SSID);
#endif

  // Wait for connection
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {

#ifdef DEBUG
    DEBUGLOG("[WIFI] Connection not established! Rebooting...");
#endif

    delay(5000);
    ESP.restart();
  }
}

/**
 * @brief Displays the current WiFi Connection status and details (for debugging
 * purposes)
 */
void wifiStatus() {

  // Don't do anything if we are not connected to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if ((WiFi.getMode() & WIFI_STA) == WIFI_STA) {

    const char *const PHY_MODE_NAMES[]{"", "B", "G", "N"};
    const char *const OP_MODE_NAMES[]{"NULL", "STA", "AP", "STA+AP"};

    DEBUGLOG("[WIFI] SSID        : %s\n", WiFi.SSID().c_str());
    DEBUGLOG("[WIFI] IP Address  : %s\n", WiFi.localIP().toString().c_str());
    DEBUGLOG("[WIFI] MAC Address : %s\n", WiFi.macAddress().c_str());
    DEBUGLOG("[WIFI] Gateway     : %s\n", WiFi.gatewayIP().toString().c_str());
    DEBUGLOG("[WIFI] DNS         : %s\n", WiFi.dnsIP().toString().c_str());
    DEBUGLOG("[WIFI] Subnet Mask : %s\n", WiFi.subnetMask().toString().c_str());
    DEBUGLOG("[WIFI] Host        : %s\n", WiFi.hostname().c_str());
    DEBUGLOG("[WIFI] Channel     : %d\n", WiFi.channel());
    DEBUGLOG("[WIFI] PHY Mode    : %s\n", PHY_MODE_NAMES[WiFi.getPhyMode()]);
    DEBUGLOG("[WIFI] Oper. Mode  : %s\n", OP_MODE_NAMES[WiFi.getMode()]);
  }
}
