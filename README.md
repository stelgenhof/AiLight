![AiLight Build](https://github.com/stelgenhof/AiLight/workflows/AiLight%20Build/badge.svg?branch=develop) [![No Maintenance Intended](http://unmaintained.tech/badge.svg)](http://unmaintained.tech/)

![AiLight](https://raw.githubusercontent.com/wiki/stelgenhof/AiLight/images/ailight_logo.png)

**AiLight** is a custom firmware for the inexpensive Ai-Thinker (or equivalent) RGBW WiFi light bulbs that has an ESP8266 on board and is controlled by the MY9291 or MY9231 LED driver.

Current Stable Release: **v1.0.0** (Please read the [changelog](https://github.com/stelgenhof/AiLight/blob/master/CHANGELOG.md) for detailed information).

![Ai-Thinker RGBW Light bulb](https://github.com/stelgenhof/AiLight/wiki/images/aithinker_light.png)

## Features

With **AiLight** you can:

- switch the light on or off
- set the level of the 4 colour channels (Red, Green, Blue and White)
- set the brightness level
- set the light at a particular [colour temperature](https://github.com/stelgenhof/AiLight/wiki/Colour-Temperature)
- let the light [flash](https://github.com/stelgenhof/AiLight/wiki/Flashing-the-Light) (i.e. blinking with a given colour and brightness)
- enable [Gamma Correction](https://github.com/stelgenhof/AiLight/wiki/Gamma-Correction) to make the LED colours appear closer to what our eyes perceive
- set the light to [transition](https://github.com/stelgenhof/AiLight/wiki/Transition) to the new state, rather than immediately.

This can all be done with the built-in (mobile friendly) Web UI or in [Home Assistant](https://home-assistant.io
) (using the MQTT built-in integration via JSON). The Web UI also gives you the ability to configure your Smart Light remotely. You can easily change your WiFi settings or the configuration of your MQTT broker.

### Other
- [REST API](https://github.com/stelgenhof/AiLight/wiki/REST-API)
- MQTT Last Will and Testament enabled
- Support for Home Assistant's [MQTT Discovery](https://github.com/stelgenhof/AiLight/wiki/Home-Assistant-MQTT-Discovery)
- Support for [Over The Air](https://github.com/stelgenhof/AiLight/wiki/OTA-Updates) (OTA) firmware updates
- Preserve light settings and configuration after power cycle or restart
- Perform remote [restart](https://github.com/stelgenhof/AiLight/wiki/Restart-%26-Reset) using the built-in HTML UI.
- [Reset](https://github.com/stelgenhof/AiLight/wiki/Restart-%26-Reset) to factory defaults using the built-in HTML UI (* 'factory' here means the default settings of the **AiLight** firmware upon compile time)


Making this firmware was largely inspired by the [MY9291](https://github.com/xoseperez/my9291) LED driver and the [Espurna](https://github.com/xoseperez/espurna) firmware of [Xose Pérez](https://github.com/xoseperez).

## Getting started
Got curious and want to use **AiLight** too? Head over to the [Wiki](https://github.com/stelgenhof/AiLight/wiki) where you can find all relevant topics on how to [connect](https://github.com/stelgenhof/AiLight/wiki/Connection), [flash](https://github.com/stelgenhof/AiLight/wiki/Flashing-the-Firmware) and use the **AiLight** firmware!


## Bugs and Feedback
For bugs, questions and discussions, please use the [Github Issues](https://github.com/stelgenhof/AiLight/issues).

## Contributing

Contributions are encouraged and welcome; I am always happy to get feedback or pull requests on Github :) Create [Github Issues](https://github.com/stelgenhof/AiLight/issues) for bugs and new features and comment on the ones you are interested in.

If you enjoy what I am making, an extra cup of coffee is very much appreciated :). Your support helps me to put more time into Open-Source Software projects like this.

<a href="https://www.buymeacoffee.com/sachatelgenhof" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: auto !important;width: auto !important;" ></a>

## Credits and License

The **AiLight** Firmware is open-sourced software licensed under the [MIT license](http://opensource.org/licenses/MIT). For the full copyright and license information, please see the <license> file that was distributed with this source code.</license>
