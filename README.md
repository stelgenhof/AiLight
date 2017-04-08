# Ai-Thinker RGBW Light Firmware (Home Assistant)

The Ai-Thinker RGBW Light Firmware is a custom firmware for the inexpensive WiFi RGB light bulbs that has the ESP8266 MCU at its core. Xose Pérez has written an excellent [article](http://tinkerman.cat/ailight-hackable-rgbw-light-bulb/) on his blog how you can upload your own firmware to this light. This firmware is designed to operate with [Home Assistant](https://home-assistant.io) using the [MQTT](https://mqtt.org) protocol.

Note: Be aware that uploading this firmware replaces the factory firmware and all warranties become void.

## Features

This Ai-Thinker RGBW Light Firmware currently allows you to:

- set the state (i.e. switching on/off)
- set the level of the 4 colour channels (Red, Green, Blue and White)
- set the level of brightness
- set the colour temperature
- let the light flash (with a given colour and brightness)

**Roadmap**

- Effects
- Transitions
- Remember light state after power cycle
- MQTT Discovery

Making this firmware was largely inspired by the [MY9291](https://github.com/xoseperez/my9291) LED driver and [Espurna](https://bitbucket.org/xoseperez/espurna) firmware of [Xose Pérez](https://github.com/xoseperez).

The Ai-Thinker RGBW Light Firmware provides you some basic functionality to control your The Ai-Thinker RGBW Light with Home Assistant. If you need more advanced features, the [Espurna](https://bitbucket.org/xoseperez/espurna) firmware is an excellent alternative. Also [Samuel Mertenat](https://github.com/mertenats) has created a similar [firmware](https://github.com/mertenats/AI-Thinker_RGBW_Bulb) that provides a good alternative.

## Requirements

To build and upload the firmware you need to have [PlatformIO](http://platformio.org/) installed. This firmware is designed for the ESP8266 and AVR devices only. Other [MCU](https://en.wikipedia.org/wiki/Microcontroller) are not supported.

## Installation/Configuration

1. Clone this repository
2. Rename the `config.example.h` file in the 'src' folder to `config.h`
3. Depending on your situation, set the at least the following values in the `config.h` file:

  - WIFI_SSID // Your WiFi SSID
  - WIFI_PSK // Your WiFi PSK/Password
  - MQTT_SERVER // The hostname/IP address of your MQTT broker
  - MQTT_USER // The username to connect to your MQTT broker
  - MQTT_PASSWORD // The password to connect to your MQTT broker
  - MQTT_LIGHT_STATE_TOPIC // The MQTT state topic for Home Assistant
  - MQTT_LIGHT_COMMAND_TOPIC // The MQTT command topic for Home Assistant

  Other configuration variables can be left as is, however feel free to adjust to your liking.

4. Rename the `platformio.example.ini` file to `platformio.ini`. The PlatformIO configuration should work as is, however it is recommended to change the OTA port number and OTA password when using your Ai-Thinker RGBW Light in production. (In that case don't forget to update the respective variables in your `config.h` file too).

5. Click on the PlatformIO "Build" button (or issue the "platformio run" command from the PlatformIO terminal).

That's all it takes, you're ready to go!

If no compilation errors popped up, you can upload the firmware to your Ai-Thinker RGBW Light using an FTDI (or alike) programmer. This is a required step of course since your Ai-Thinker RGBW Light still has the original factory firmware. Once uploaded, you can use OTA to upload the firmware over the air.

**Targets** The PlatformIO configuration contains 4 environments for building and flashing your Ai-Thinker RGBW Light:

- 'env-dev': development/debug environment for building and uploading the firmware via an FTDI (or alike) programmer
- 'env-prod': production environment (optimized for size) for building and uploading the firmware via an FTDI (or alike) programmer

In addition, for each of these 2 environments, an OTA environment is available for uploading the firmware over OTA.

## Dependencies

- [MY9291](https://github.com/xoseperez/my9291) LED driver.
- PubSubClient library
- ArduinoJson library

## Contributing

Contributions are encouraged and welcome; I am always happy to get feedback or pull requests on Github :)

## Credits and License

The Ai-Thinker RGBW Light Firmware is open-sourced software licensed under the [MIT license](http://opensource.org/licenses/MIT). For the full copyright and license information, please see the

<license> file that was distributed with this source code.</license>
