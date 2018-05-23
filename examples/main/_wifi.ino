/**
 * Ai-Thinker RGBW Light Firmware - WiFi Module
 *
 * The WiFi module holds all the code to manage all functions for setting up the
 * WiFi connection.
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
 * @brief Event Handler when an IP address has been assigned
 *
 * @param event WiFiEventStationModeGotIP Event
 */
void onSTAGotIP(WiFiEventStationModeGotIP event) {

#ifdef DEBUG
  const char *const PHY_MODE_NAMES[]{"", "B", "G", "N"};

  DEBUGLOG("[WIFI] SSID        : %s\n", WiFi.SSID().c_str());
  DEBUGLOG("[WIFI] IP Address  : %s\n", WiFi.localIP().toString().c_str());
  DEBUGLOG("[WIFI] MAC Address : %s\n", WiFi.macAddress().c_str());
  DEBUGLOG("[WIFI] Gateway     : %s\n", WiFi.gatewayIP().toString().c_str());
  DEBUGLOG("[WIFI] DNS         : %s\n", WiFi.dnsIP().toString().c_str());
  DEBUGLOG("[WIFI] Subnet Mask : %s\n", WiFi.subnetMask().toString().c_str());
  DEBUGLOG("[WIFI] Host        : %s\n", WiFi.hostname().c_str());
  DEBUGLOG("[WIFI] Channel     : %d\n", WiFi.channel());
  DEBUGLOG("[WIFI] PHY Mode    : %s\n", PHY_MODE_NAMES[WiFi.getPhyMode()]);
  DEBUGLOG("[WIFI] Oper. Mode  : STA\n");
  DEBUGLOG("\n");
#endif

  mqttConnect();
}

/**
 * @brief Event Handler when client connects when in AP Mode
 *
 * @param event WiFiEventSoftAPModeStationConnected Event
 */
void onAPConnected(WiFiEventSoftAPModeStationConnected event) {
  DEBUGLOG("[WIFI] SSID        : %s\n", cfg.hostname);
  DEBUGLOG("[WIFI] Password    : %s\n", ADMIN_PASSWORD);
  DEBUGLOG("[WIFI] IP Address  : %s\n", WiFi.softAPIP().toString().c_str());
  DEBUGLOG("[WIFI] MAC Address : %s\n", WiFi.softAPmacAddress().c_str());
  DEBUGLOG("[WIFI] Oper. Mode  : AP\n");
  DEBUGLOG("\n");
}

/**
 * @brief Event Handler when WiFi is disconnected
 *
 * @param event WiFiEventStationModeDisconnected Event
 */
void onSTADisconnected(WiFiEventStationModeDisconnected event) {
  DEBUGLOG("WiFi connection (%s) dropped.\n", event.ssid.c_str());
  DEBUGLOG("Reason: %d\n", event.reason);

  DEBUGLOG("Trying to reconnect\n");
  mqttReconnectTimer
      .detach(); // Ensure not to reconnect to MQTT while reconnecting to WiFi
  wifiReconnectTimer.once(RECONNECT_TIME, setupWiFi);
}

/**
 * @brief Bootstrap function for the WiFi connection
 */
void setupWiFi() {
  static WiFiEventHandler gotIpEventHandler, apConnectedEventHandler,
      disconnectedEventHandler;
  gotIpEventHandler = WiFi.onStationModeGotIP(onSTAGotIP);
  apConnectedEventHandler = WiFi.onSoftAPModeStationConnected(onAPConnected);
  disconnectedEventHandler = WiFi.onStationModeDisconnected(onSTADisconnected);

  // Set WiFi hostname
  if (os_strlen(cfg.hostname) == 0) {
    os_strcpy(cfg.hostname, getDeviceID());
    EEPROM_write(cfg);
  }
  WiFi.hostname(cfg.hostname);

  // Set WiFi module to STA mode and set Power Output
  if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    WiFi.setOutputPower(WIFI_OUTPUT_POWER);
    delay(10);
  }

  // (Re)connect
  WiFi.disconnect();
  DEBUGLOG("[WIFI] Connecting to %s\n", cfg.wifi_ssid);
  WiFi.begin(cfg.wifi_ssid, cfg.wifi_psk);

  MDNS.addService("http", "tcp", 80);

  // Check connection and switch to AP mode if no connection
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    DEBUGLOG("[WIFI] Connection not established! Changing into AP mode...\n");

    wifiReconnectTimer.detach(); // Ensure not to reconnect to WiFi while
                                 // changing into AP mode

    WiFi.mode(WIFI_AP);
    delay(10);

    WiFi.softAP(cfg.hostname, ADMIN_PASSWORD);
  }
}
