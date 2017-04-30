# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) and this project adheres to [Semantic Versioning](http://semver.org).

## [Unreleased]

### Added
- favico added to HTML UI
- Added model name to distinguish nameing between AiLight and Ai-Thinker light bulb manufacturer/model name.

### Changed
- Migrated to AsyncMQTTClient library (replacing PubSubClient Library) * Please be aware of changes to the platformio.ini and config.h files!
- Build.py script now uses locally installed Gulp binary instead of global one
- HTML UI title includes now the device name so it's easier to identify which light you are looking at

### Fixed

### Removed

## [0.2.0-alpha] - 2017-04-27
After two weeks of hacking and modding, a first Alpha release is ready! The AiLight firmware allows you via WiFi to:
- switch the light on or off
- set the level of the 4 colour channels (Red, Green, Blue and White)
- set the brightness level
- set the light at a particular colour temperature
- let the light flash (with a defined colour and brightness)
- after powering on the light again, the last known settings are remembered (colour, brightness, etc.)
This can all be done in HomeAssistant (using the MQTT integration) or the built in (mobile friendly) UI.

## [0.1.0] - 2017-04-21
Initial Release of the AiLight: AiLight is a simple library to control an AiLight that contains the MY9291 LED driver.
