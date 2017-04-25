/**
 * Ai-Thinker RGBW Light Firmware - WiFi Module
 *
 * The WiFi module holds all the code to manage all functions for setting up the
 * WiFi connection.
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.

 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */

// ---------------------------
// Core functions
// ---------------------------

/**
 * @brief Bootstrap function for the WiFi connection
 */
void setupWiFi() {

  // Set WiFi hostname
  if (strlen(cfg.hostname) == 0) {
    strcpy(cfg.hostname, getDeviceID());
    EEPROM_write(cfg);
  }
  WiFi.hostname(cfg.hostname);

  // Set WiFi module to STA mode and set Power Output
  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    WiFi.setOutputPower(WIFI_OUTPUT_POWER);
  }

  // Connecting
  WiFi.disconnect();
  DEBUGLOG("[WIFI] Connecting to %s\n", cfg.wifi_ssid);
  WiFi.begin(cfg.wifi_ssid, cfg.wifi_psk);
  MDNS.begin(cfg.hostname);
  MDNS.addService("http", "tcp", 80);

  // Check connection and switch to AP mode if no connection
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    DEBUGLOG("[WIFI] Connection not established! Changing into AP mode...\n");

    // Go into software AP mode.
    WiFi.mode(WIFI_AP);
    delay(10);

    WiFi.softAP(cfg.hostname, ADMIN_PASSWORD);
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
