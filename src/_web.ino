/**
 * Ai-Thinker RGBW Light Firmware - Web Module
 *
 * The Web module contains all the code for handling the HTTP User Interface.
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
 * @brief Check whether the requestor is authorized using the requested API
 * endpoint
 *
 * @param request the API endpoint request object
 *
 * @return bool true if authorized, otherwise false
 */
bool authorizeAPI(AsyncWebServerRequest *request) {

  // Check if API Key is provided
  if (!request->hasHeader(HTTP_HEADER_APIKEY)) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["error"] = "400";
    root["message"] = "The required API Key is missing";

    char buffer[root.measureLength() + 1];
    root.printTo(buffer, sizeof(buffer));

    AsyncWebServerResponse *response =
        request->beginResponse(400, HTTP_MIMETYPE_JSON, buffer);
    response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
    request->send(response);

    return false;
  } else {
    AsyncWebHeader *h = request->getHeader(HTTP_HEADER_APIKEY);
    if (!h->value().equals(cfg.api_key)) {

      DynamicJsonBuffer jsonBuffer;
      JsonObject &root = jsonBuffer.createObject();
      root["error"] = "401";
      root["message"] = "The given API Key is incorrect";

      char buffer[root.measureLength() + 1];
      root.printTo(buffer, sizeof(buffer));

      AsyncWebServerResponse *response =
          request->beginResponse(401, HTTP_MIMETYPE_JSON, buffer);
      response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
      request->send(response);

      return false;
    }
  }

  return true;
}

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
  createAboutJSON(device);

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
  settings[KEY_MQTT_LWT_TOPIC] = cfg.mqtt_lwt_topic;
  settings[KEY_MQTT_HA_USE_DISCOVERY] = cfg.mqtt_ha_use_discovery;
  settings[KEY_MQTT_HA_IS_DISCOVERED] = cfg.mqtt_ha_is_discovered;

  // Ensure HA MQTT Discovery Prefix has a proper value
  if (cfg.mqtt_ha_disc_prefix == NULL || cfg.mqtt_ha_disc_prefix[0] == 0xFF) {
    os_strcpy(cfg.mqtt_ha_disc_prefix, MQTT_HOMEASSISTANT_DISCOVERY_PREFIX);
  }
  settings[KEY_MQTT_HA_DISCOVERY_PREFIX] = cfg.mqtt_ha_disc_prefix;

  // REST API
  settings[KEY_REST_API_ENABLED] = cfg.api;
  if (cfg.api_key == NULL || cfg.api_key[0] == 0xFF) {
    os_strcpy(cfg.api_key, ADMIN_PASSWORD);
  }
  settings[KEY_REST_API_KEY] = cfg.api_key;

  settings[KEY_POWERUP_MODE] = cfg.powerup_mode;

  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  ws.text(id, buffer);
}

void wsProcessMessage(uint8_t num, char *payload, size_t length) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(payload);
  bool settings_changed = false;
  bool needRestart = false;

  if (!root.success()) {
    DEBUGLOG("[WEBSOCKET] Error parsing data\n");
    return;
  }

  // Process commands
  if (root.containsKey("command")) {
    const char *command = root["command"];

    DEBUGLOG("[WEBSOCKET] Client #%u requested a %s\n", num, command);

    // Execute restart command
    if (os_strcmp(command, "restart") == 0) {
      ESP.restart();
    }

    // Execute reset command (load factory defaults)
    if (os_strcmp(command, "reset") == 0) {
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
      if (os_strcmp(cfg.hostname, hostname) != 0) {
        os_strcpy(cfg.hostname, hostname);

        cfg.mqtt_ha_is_discovered =
            false; // Since hostname is used as name in HA

        needRestart = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_SERVER)) {
      const char *mqtt_server = settings[KEY_MQTT_SERVER];
      if (os_strcmp(cfg.mqtt_server, mqtt_server) != 0) {
        os_strcpy(cfg.mqtt_server, mqtt_server);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_PORT)) {
      uint16_t mqtt_port = (os_strlen(settings[KEY_MQTT_PORT]) > 0)
                               ? settings[KEY_MQTT_PORT]
                               : MQTT_PORT;
      if (cfg.mqtt_port != mqtt_port) {
        cfg.mqtt_port = mqtt_port;
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_USER)) {
      const char *mqtt_user = settings[KEY_MQTT_USER];
      if (os_strcmp(cfg.mqtt_user, mqtt_user) != 0) {
        os_strcpy(cfg.mqtt_user, mqtt_user);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_PASSWORD)) {
      const char *mqtt_password = settings[KEY_MQTT_PASSWORD];
      if (os_strcmp(cfg.mqtt_password, mqtt_password) != 0) {
        os_strcpy(cfg.mqtt_password, mqtt_password);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_STATE_TOPIC)) {
      const char *mqtt_state_topic = settings[KEY_MQTT_STATE_TOPIC];
      if (os_strcmp(cfg.mqtt_state_topic, mqtt_state_topic) != 0) {
        os_strcpy(cfg.mqtt_state_topic, mqtt_state_topic);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_COMMAND_TOPIC)) {
      const char *mqtt_command_topic = settings[KEY_MQTT_COMMAND_TOPIC];
      if (os_strcmp(cfg.mqtt_command_topic, mqtt_command_topic) != 0) {
        os_strcpy(cfg.mqtt_command_topic, mqtt_command_topic);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_LWT_TOPIC)) {
      const char *mqtt_lwt_topic = settings[KEY_MQTT_LWT_TOPIC];
      if (os_strcmp(cfg.mqtt_lwt_topic, mqtt_lwt_topic) != 0) {
        os_strcpy(cfg.mqtt_lwt_topic, mqtt_lwt_topic);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_HA_USE_DISCOVERY)) {
      bool mqtt_ha_use_discovery = settings[KEY_MQTT_HA_USE_DISCOVERY];
      if (cfg.mqtt_ha_use_discovery != mqtt_ha_use_discovery) {
        cfg.mqtt_ha_use_discovery = mqtt_ha_use_discovery;

        // Reset that light has been discovered already
        if (!mqtt_ha_use_discovery) {
          cfg.mqtt_ha_is_discovered = false;
        }

        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_MQTT_HA_DISCOVERY_PREFIX)) {
      const char *mqtt_ha_disc_prefix = settings[KEY_MQTT_HA_DISCOVERY_PREFIX];
      if (os_strcmp(cfg.mqtt_ha_disc_prefix, mqtt_ha_disc_prefix) != 0) {
        os_strcpy(cfg.mqtt_ha_disc_prefix, mqtt_ha_disc_prefix);
        mqtt_changed = true;
      }
    }

    if (settings.containsKey(KEY_WIFI_SSID)) {
      const char *wifi_ssid = settings[KEY_WIFI_SSID];
      if (os_strcmp(cfg.wifi_ssid, wifi_ssid) != 0) {
        os_strcpy(cfg.wifi_ssid, wifi_ssid);
        wifi_changed = true;
      }
    }

    if (settings.containsKey(KEY_WIFI_PSK)) {
      const char *wifi_psk = settings[KEY_WIFI_PSK];
      if (os_strcmp(cfg.wifi_psk, wifi_psk) != 0) {
        os_strcpy(cfg.wifi_psk, wifi_psk);
        wifi_changed = true;
      }
    }

    // REST API
    if (settings.containsKey(KEY_REST_API_ENABLED)) {
      bool rest_api_enabled = settings[KEY_REST_API_ENABLED];
      if (cfg.api != rest_api_enabled) {
        cfg.api = rest_api_enabled;
        needRestart = true;
      }
    }

    if (settings.containsKey(KEY_REST_API_KEY)) {
      const char *api_key = settings[KEY_REST_API_KEY];
      if (os_strcmp(cfg.api_key, api_key) != 0) {
        os_strcpy(cfg.api_key, api_key);
      }
    }

    if (settings.containsKey(KEY_POWERUP_MODE)) {
      uint8_t powerup_mode = (os_strlen(settings[KEY_POWERUP_MODE]) > 0)
                                 ? settings[KEY_POWERUP_MODE]
                                 : POWERUP_MODE;
      if (cfg.powerup_mode != powerup_mode) {
        cfg.powerup_mode = powerup_mode;
      }
    }

    // Reconnect to the MQTT broker due to new settings
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
    AiLight.setState(
        (os_strcmp(root[KEY_STATE], MQTT_PAYLOAD_ON) == 0) ? true : false);
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

#ifdef DEBUG
      IPAddress ip = client->remoteIP();
#endif

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
      os_memcpy(message + info->index, data, len);

      // Last packet
      if (info->index + len == info->len) {
        wsProcessMessage(client->id(), message, info->len);
        free(message);
      }
    }
  });
  server->addHandler(&ws);
  server->addHandler(&events);

  server->rewrite("/", HTTP_ROUTE_INDEX);

  // Send a file when /index is requested
  server->on(HTTP_ROUTE_INDEX, HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse_P(200, HTTP_MIMETYPE_HTML, html_gz, html_gz_len);

    response->addHeader("Content-Encoding", "gzip");
    response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);

    request->send(response);
  });

  if (cfg.api) {
    server->onRequestBody([](AsyncWebServerRequest *request, uint8_t *data,
                             size_t len, size_t index, size_t total) {
      // Process requested changes for the light
      if (request->url().equals(HTTP_APIROUTE_LIGHT)) {

        // Check for appropriate HTTP method
        if (request->method() != HTTP_PATCH) {
          AsyncWebServerResponse *response =
              request->beginResponse(405, HTTP_MIMETYPE_JSON);
          response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
          response->addHeader(HTTP_HEADER_ALLOW, "GET, PATCH");
          request->send(response);
        }

        if (!authorizeAPI(request)) {
          return;
        }

        if (!processJson((char *)data)) {
          DynamicJsonBuffer jsonBuffer;
          JsonObject &root = jsonBuffer.createObject();
          root["error"] = "400";
          root["message"] = "Unable to process the JSON message";

          char buffer[root.measureLength() + 1];
          root.printTo(buffer, sizeof(buffer));

          AsyncWebServerResponse *response =
              request->beginResponse(400, HTTP_MIMETYPE_JSON, buffer);
          response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
          request->send(response);

          return;
        }

        sendState(); // Notify subscribers about the new state

        // Send response
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        createStateJSON(root);

        char buffer[root.measureLength() + 1];
        root.printTo(buffer, sizeof(buffer));

        AsyncWebServerResponse *response =
            request->beginResponse(200, HTTP_MIMETYPE_JSON, buffer);
        response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
        request->send(response);
      }
    });
  }

  if (cfg.api) {

    // 'Light' API Endpoint
    server->on(HTTP_APIROUTE_LIGHT, HTTP_GET,
               [](AsyncWebServerRequest *request) {
                 // Check for appropriate HTTP method
                 if (request->method() != HTTP_GET) {
                   AsyncWebServerResponse *response =
                       request->beginResponse(405, HTTP_MIMETYPE_JSON);
                   response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
                   response->addHeader(HTTP_HEADER_ALLOW, "GET, PATCH");
                   request->send(response);

                   return;
                 }

                 if (!authorizeAPI(request)) {
                   return;
                 }

                 // Send response
                 DynamicJsonBuffer jsonBuffer;
                 JsonObject &root = jsonBuffer.createObject();
                 createStateJSON(root);

                 char buffer[root.measureLength() + 1];
                 root.printTo(buffer, sizeof(buffer));

                 AsyncWebServerResponse *response =
                     request->beginResponse(200, HTTP_MIMETYPE_JSON, buffer);
                 response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
                 request->send(response);
               });

    // 'About' API Endpoint
    server->on(HTTP_APIROUTE_ABOUT, HTTP_ANY,
               [](AsyncWebServerRequest *request) {
                 // Only allow HTTP_GET method
                 if (request->method() != HTTP_GET) {
                   AsyncWebServerResponse *response =
                       request->beginResponse(405, HTTP_MIMETYPE_JSON);
                   response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
                   response->addHeader(HTTP_HEADER_ALLOW, "GET");
                   request->send(response);

                   return;
                 }

                 if (!authorizeAPI(request)) {
                   return;
                 }

                 DynamicJsonBuffer jsonBuffer;
                 JsonObject &root = jsonBuffer.createObject();
                 createAboutJSON(root);

                 AsyncResponseStream *response =
                     request->beginResponseStream(HTTP_MIMETYPE_JSON);
                 response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
                 root.printTo(*response);

                 request->send(response);
               });
  }

  // Handle unknown URI
  server->onNotFound([](AsyncWebServerRequest *request) {
    const char *mime_type = HTTP_MIMETYPE_HTML;

    if (cfg.api && request->url().startsWith(HTTP_APIROUTE_ROOT)) {
      mime_type = HTTP_MIMETYPE_JSON;
    }

    AsyncWebServerResponse *response = request->beginResponse(404, mime_type);
    response->addHeader(HTTP_HEADER_SERVER, SERVER_SIGNATURE);
    request->send(response);
  });

  server->begin();
  DEBUGLOG("[HTTP] Webserver running...\n");
}
