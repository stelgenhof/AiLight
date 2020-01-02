/**
 * AiLight Firmware - Configuration
 *
 * This file is part of the AiLight Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * Created by Sacha Telgenhof <me at sachatelgenhof dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2020 Sacha Telgenhof
 */

/**
 * Light
 * ---------------------------
 * Use the below variables to set the default behaviour of your Smart Light.
 * These will be used as the factory defaults of your device.
 */
#define LIGHT_STATE false
#define LIGHT_BRIGHTNESS 0
#define LIGHT_COLOR_TEMPERATURE 0
#define LIGHT_COLOR_RED 64
#define LIGHT_COLOR_GREEN 64
#define LIGHT_COLOR_BLUE 64
#define LIGHT_COLOR_WHITE 0

#define HOSTNAME "AiLight"
#define ADMIN_PASSWORD "hinotori"

#define POWERUP_MODE POWERUP_OFF

/**
 * LedDriver
 * --------------------------
 * Define type and number of chips used. Allow firmware use with multiple
 * types/designs of lights
 */
#define MY92XX_TYPE MY92XX_MODEL_MY9291
#define MY92XX_COUNT 1

/**
 * OTA (Over The Air) Updates
 * ---------------------------
 */
#define OTA_PORT 8266

/**
 * WiFi
 * ---------------------------
 * Use the below variables to set the default WiFi settings of your Smart Light.
 * These will be used as the factory defaults of your device. If no
 * SSID/PSK are provided, your Smart Light will start in AP mode.
 */
#define WIFI_SSID ""
#define WIFI_PSK ""
#define WIFI_OUTPUT_POWER 1.0 // 20.5 is the maximum output power

/**
 * Timeout period for the device to keep trying to (re)connect to the
 * configured WiFi Access Point. If this timeout period has been reached, the
 * device will assume a WiFi connection can not be made and will switch to
 * Soft AP mode.
#define WIFI_RECONNECT_TIMEOUT 60 // Timeout (in seconds)

/**
 * MQTT
 * ---------------------------
 * Use the below variables to set the default MQTT settings of your Smart Light.
 * These will be used as the factory defaults of your device and
 * making the connection to your Home Assistant instance. Most of these settings
 * can also be changed in the UI environment.
 */
#define MQTT_PORT 1883
#define MQTT_SERVER ""
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_RECONNECT_TIME 10000
#define MQTT_QOS_LEVEL 0
#define MQTT_RETAIN false
#define MQTT_KEEPALIVE 30

#define MQTT_PAYLOAD_ON "ON"
#define MQTT_PAYLOAD_OFF "OFF"

#define MQTT_STATUS_ONLINE "online"
#define MQTT_STATUS_OFFLINE "offline"

#define MQTT_HOMEASSISTANT_DISCOVERY_ENABLED false
#define MQTT_HOMEASSISTANT_DISCOVERY_PREFIX "homeassistant"

/**
 * Home Assistant 0.84 removed the "mqtt_json" platform type, replacing it with
 * a combination of "platform: mqtt" and "schema: json". If you are using
 * version 0.84 or older of Home Assistant and using the MQTT discovery feature,
 * set the following directive to "true"
 */
#define MQTT_HOMEASSISTANT_DISCOVERY_PRE_0_84 false

/**
 * HTTP
 * ---------------------------
 * Use the below variables to set the default HTTP settings of your Smart Light.
 * These will be used as the factory defaults of your device.
 */
#define REST_API_ENABLED false
