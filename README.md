![AiLight](https://raw.githubusercontent.com/wiki/stelgenhof/AiLight/images/ailight_logo.png)

**AiLight** is a custom firmware for the inexpensive Ai-Thinker RGBW WiFi light bulbs that has the ESP8266 MCU at its core. Xose Pérez has written an excellent [article](http://tinkerman.cat/ailight-hackable-rgbw-light-bulb/) on his blog how you can upload your own firmware to this light.

![AiLight](https://www.sachatelgenhof.nl/user/pages/02.blog/ailight/screen_combo_m.png)

## Main Features

**AiLight** allows you to:

- switch the light on or off
- set the level of the 4 colour channels (Red, Green, Blue and White)
- set the brightness level
- set the light at a particular [colour temperature](https://github.com/stelgenhof/AiLight/wiki/Colour-Temperature)
- let the light [flash](https://github.com/stelgenhof/AiLight/wiki/Flashing-the-Light) (i.e. blinking with a given colour and brightness)
- enable [Gamma Correction](https://github.com/stelgenhof/AiLight/wiki/Gamma-Correction) to make the LED colours appear closer to what our eyes perceive
- set the light to [transition](https://github.com/stelgenhof/AiLight/wiki/Transition) to the new state, rather than immediately.

This can all be done with the built-in (mobile friendly) Web UI or in [Home Assistant](https://home-assistant.io) (using the MQTT built-in integration via JSON). The Web UI also gives you the ability to configure your Ai-Thinker RGBW Light remotely. You can easily change your WiFi settings or the configuration of your MQTT broker.

### Other Features

- MQTT Last Will and Testament enabled
- Support for Over The Air (OTA) firmware updates
- Preserve light settings and configuration after power cycle or restart
- Perform remote [restart](https://github.com/stelgenhof/AiLight/wiki/Restart-%26-Reset) using the built-in HTML UI.
- [Reset](https://github.com/stelgenhof/AiLight/wiki/Restart-%26-Reset) to factory defaults using the built-in HTML UI (* 'factory' here means the default settings of the **AiLight** firmware upon compile time)


**Roadmap**

- MQTT Discovery
- Effects
- ~~Transitions~~
- ~~Remember light state after power cycle~~
- ~~MQTT Last Will And Testament~~
- ~~Gamma Correction~~

Making this firmware was largely inspired by the [MY9291](https://github.com/xoseperez/my9291) LED driver and the [Espurna](https://bitbucket.org/xoseperez/espurna) firmware of [Xose Pérez](https://github.com/xoseperez).

## Getting started
Curious and want to use **AiLight**? Head over to the Wiki where you can find all relevant topics on how to connect, flash and use the **AiLight** firmware!

## Bugs and Feedback
For bugs, questions and discussions, please use the [Github Issues](https://github.com/stelgenhof/AiLight/issues).

## Contributing

Contributions are encouraged and welcome; I am always happy to get feedback or pull requests on Github :) Create [Github Issues](https://github.com/stelgenhof/AiLight/issues) for bugs and new features and comment on the ones you are interested in.

## Credits and License

The Ai-Thinker RGBW Light Firmware is open-sourced software licensed under the [MIT license](http://opensource.org/licenses/MIT). For the full copyright and license information, please see the <license> file that was distributed with this source code.</license>
