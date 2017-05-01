/**
 * Ai-Thinker RGBW Light Firmware - Web Module
 *
 * The Web module contains all the code for handling the HTTP User Interface.
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.

 * Created by Sacha Telgenhof <stelgenhof at gmail dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2017 Sacha Telgenhof
 */

AsyncWebServer *server;
AsyncWebSocket ws("/ws");

/**
 * @brief Publishes data to WebSocket client upon connection
 *
 * @param id the WebSocket client identifier
 *
 * @return void
 */
void wsStart(uint8_t id) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  // Operational state
  root[KEY_STATE] = AiLight.getState() ? MQTT_PAYLOAD_ON : MQTT_PAYLOAD_OFF;
  root[KEY_BRIGHTNESS] = AiLight.getBrightness();
  root[KEY_WHITE] = AiLight.getColor().white;
  root[KEY_COLORTEMP] = AiLight.getColorTemperature();

  JsonObject &color = root.createNestedObject(KEY_COLOR);
  color[KEY_COLOR_R] = AiLight.getColor().red;
  color[KEY_COLOR_G] = AiLight.getColor().green;
  color[KEY_COLOR_B] = AiLight.getColor().blue;

  root[KEY_GAMMA_CORRECTION] = AiLight.hasGammaCorrection();

  // Device settings/state
  JsonObject &device = root.createNestedObject("d");
  device["app_name"] = APP_NAME;
  device["app_version"] = APP_VERSION;
  device["build_date"] = __DATE__;
  device["build_time"] = __TIME__;
  device["memory"] = ESP.getFlashChipSize();
  device["free_heap"] = ESP.getFreeHeap();
  device["cpu_frequency"] = ESP.getCpuFreqMHz();
  device["manufacturer"] = "Ai-Thinker";
  device["model"] = "RGBW Light";
  device["device_ip"] = (WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP().toString()
                                                    : WiFi.localIP().toString();
  device["mac"] = WiFi.macAddress();

  // User settings
  JsonObject &settings = root.createNestedObject("s");
  settings[KEY_HOSTNAME] = cfg.hostname;
  settings[KEY_WIFI_SSID] = cfg.wifi_ssid;
  settings[KEY_WIFI_PSK] = cfg.wifi_psk;
  settings[KEY_MQTT_SERVER] = cfg.mqtt_server;
  settings[KEY_MQTT_PORT] = cfg.mqtt_port;
  settings[KEY_MQTT_USER] = cfg.mqtt_user;
  settings[KEY_MQTT_PASSWORD] = cfg.mqtt_password;
  settings[KEY_MQTT_STATE_TOPIC] = cfg.mqtt_state_topic;
  settings[KEY_MQTT_COMMAND_TOPIC] = cfg.mqtt_command_topic;

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));
  ws.text(id, buffer);
}

void wsProcessMessage(uint8_t num, char *payload, size_t length) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(payload);
  bool settings_changed = false;
  bool needRestart = false;

  root.printTo(Serial);

  if (!root.success()) {
    DEBUGLOG("[WEBSOCKET] Error parsing data\n");
    return;
  }

  // Process commands
  if (root.containsKey("command")) {
    const char *command = root["command"];

    DEBUGLOG("[WEBSOCKET] Client #%u requested a %s\n", num, command);

    // Execute restart command
    if (strcmp(command, "restart") == 0) {
      ESP.restart();
    }

    // Execute reset command (load factory defaults)
    if (strcmp(command, "reset") == 0) {
      loadFactoryDefaults();
      ESP.restart();
    }
  }

  // Process new settings
  if (root.containsKey(KEY_SETTINGS) && root[KEY_SETTINGS].is<JsonObject &>()) {
    bool mqtt_changed = false;
    bool wifi_changed = false;
    settings_changed = true;

    JsonObject &settings = root[KEY_SETTINGS];
    DEBUGLOG("[WEBSOCKET] Received new settings\n");

    if (settings.containsKey(KEY_HOSTNAME)) {
      const char *hostname = settings[KEY_HOSTNAME];
      if (strcmp(cfg.hostname, hostname) != 0) {
        strcpy(cfg.hostname, hostname);
        needRestart = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_SERVER)) {
      const char *mqtt_server = settings[KEY_MQTT_SERVER];
      if (strcmp(cfg.mqtt_server, mqtt_server) != 0) {
        strcpy(cfg.mqtt_server, mqtt_server);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_PORT)) {
      uint16_t mqtt_port = (strlen(settings[KEY_MQTT_PORT]) > 0)
                               ? settings[KEY_MQTT_PORT]
                               : MQTT_PORT;
      if (cfg.mqtt_port != mqtt_port) {
        cfg.mqtt_port = mqtt_port;
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_USER)) {
      const char *mqtt_user = settings[KEY_MQTT_USER];
      if (strcmp(cfg.mqtt_user, mqtt_user) != 0) {
        strcpy(cfg.mqtt_user, mqtt_user);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_PASSWORD)) {
      const char *mqtt_password = settings[KEY_MQTT_PASSWORD];
      if (strcmp(cfg.mqtt_password, mqtt_password) != 0) {
        strcpy(cfg.mqtt_password, mqtt_password);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_STATE_TOPIC)) {
      const char *mqtt_state_topic = settings[KEY_MQTT_STATE_TOPIC];
      if (strcmp(cfg.mqtt_state_topic, mqtt_state_topic) != 0) {
        strcpy(cfg.mqtt_state_topic, mqtt_state_topic);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_COMMAND_TOPIC)) {
      const char *mqtt_command_topic = settings[KEY_MQTT_COMMAND_TOPIC];
      if (strcmp(cfg.mqtt_command_topic, mqtt_command_topic) != 0) {
        strcpy(cfg.mqtt_command_topic, mqtt_command_topic);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_WIFI_SSID)) {
      const char *wifi_ssid = settings[KEY_WIFI_SSID];
      if (strcmp(cfg.wifi_ssid, wifi_ssid) != 0) {
        strcpy(cfg.wifi_ssid, wifi_ssid);
        wifi_changed = true;
      }
    }

    if (settings.containsKey(KEY_WIFI_PSK)) {
      const char *wifi_psk = settings[KEY_WIFI_PSK];
      if (strcmp(cfg.wifi_psk, wifi_psk) != 0) {
        strcpy(cfg.wifi_psk, wifi_psk);
        wifi_changed = true;
      }
    }

    // Restart the MQTT broker due to new settings
    if (mqtt_changed) {
      EEPROM_write(cfg);
      mqtt.disconnect();
    }

    // Reconnect to WiFi due to new settings
    if (wifi_changed) {
      EEPROM_write(cfg);
      setupWiFi();
    }
  }

  // Process light parameters
  if (root.containsKey(KEY_BRIGHTNESS)) {
    AiLight.setBrightness(root[KEY_BRIGHTNESS]);
  }

  if (root.containsKey(KEY_COLORTEMP)) {
    AiLight.setColorTemperature(root[KEY_COLORTEMP]);
  }

  if (root.containsKey(KEY_COLOR)) {
    AiLight.setColor(root[KEY_COLOR][KEY_COLOR_R], root[KEY_COLOR][KEY_COLOR_G],
                     root[KEY_COLOR][KEY_COLOR_B]);
  }

  if (root.containsKey(KEY_WHITE)) {
    AiLight.setWhite(root[KEY_WHITE]);
  }

  if (root.containsKey(KEY_STATE)) {
    bool state = (strcmp(root[KEY_STATE], MQTT_PAYLOAD_ON) == 0) ? true : false;
    AiLight.setState(state);
  }

  if (root.containsKey(KEY_GAMMA_CORRECTION)) {
    bool gamma = root[KEY_GAMMA_CORRECTION];
    AiLight.useGammaCorrection(gamma);
  }

  // Store light parameters for persistance
  cfg.is_on = AiLight.getState();
  cfg.brightness = AiLight.getBrightness();
  cfg.color_temp = AiLight.getColorTemperature();
  cfg.color = {AiLight.getColor().red, AiLight.getColor().green,
               AiLight.getColor().blue, AiLight.getColor().white};
  cfg.gamma = AiLight.hasGammaCorrection();
  EEPROM_write(cfg);

  if (needRestart) {
    ESP.restart();
  }

  if (!settings_changed) {
    sendState();
  }
}

/**
 * @brief Bootstrap function setting up the HTTP and WebSocket servers.
 */
void setupWeb() {
  server = new AsyncWebServer(80);

  // Setup WebSocket and handle WebSocket events
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len) {

    if (type == WS_EVT_CONNECT) {
      IPAddress ip = client->remoteIP();
      DEBUGLOG("[WEBSOCKET] client #%u connected (IP: %s)\n", client->id(),
               ip.toString().c_str());
      wsStart(client->id());
    } else if (type == WS_EVT_DISCONNECT) {
      DEBUGLOG("[WEBSOCKET] client #%u disconnected\n", client->id());
    } else if (type == WS_EVT_ERROR) {
      DEBUGLOG("[WEBSOCKET] client #%u error(%u): %s\n", server->url(),
               client->id(), *((uint16_t *)arg), (char *)data);
    } else if (type == WS_EVT_PONG) {
      DEBUGLOG("[WEBSOCKET] #%u pong(%u): %s\n", client->id(), len,
               (len) ? (char *)data : "");
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      static char *message;

      // First packet
      if (info->index == 0) {
        message = (char *)malloc(info->len);
      }

      // Store data
      memcpy(message + info->index, data, len);

      // Last packet
      if (info->index + len == info->len) {
        wsProcessMessage(client->id(), message, info->len);
        free(message);
      }
    }

  });
  server->addHandler(&ws);

  server->rewrite("/", "/index.html");

  // Send a file when /index is requested
  server->on("/index.html", HTTP_ANY, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse_P(200, "text/html", html_gz, html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  // Handle unknown URI
  server->onNotFound(
      [](AsyncWebServerRequest *request) { request->send(404); });

  server->begin();
  DEBUGLOG("[HTTP] Webserver running...\n");
}
