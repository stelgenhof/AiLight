# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) and this project adheres to [Semantic Versioning](http://semver.org).

## [Unreleased]

### Added
- When an OTA update has been initiated, a message window - with a nice progress bar - is being displayed to indicate the user that an update is in progress. The Web UI is then reloaded automatically.
- When the user chooses RESTART or RESET, a user friendly message window is being shown.

### Changed
- Changed position of password visibility icon to be inside input box.
- Adjusted path to gulp binary in the 'build.py' file to better support Windows environments.

### Fixed
- Included missing gulp-util package in the 'package.json' file.

### Removed
- SVG font (used for icons) as most browsers started unsupporting it. Helps reducing the size of the firmware.


## [0.3.0] - 2017-05-09

### Added
- MQTT Last Will and Testament, giving the MQTT broker and other clients option to know if the Ai-Thinker RGBW light has been disconnected gracefully or not.
- Gamma Correction: makes the colours of the LED light to appear closer to what our eyes perceive. This allows for better colour representations.
- favico added to HTML UI
- Added model name to distinguish naming between AiLight and Ai-Thinker light bulb manufacturer/model name.

### Changed
- Migrated to AsyncMQTTClient library (replacing PubSubClient Library) * Please be aware of changes to the platformio.ini and config.h files!
- Build.py script now uses locally installed Gulp binary instead of global one
- HTML UI title includes now the device name so it's easier to identify which light you are looking at
- Reduced size of HTML UI by removing unused style sheet elements, shrinking logo and removing unnecessary code.
- The sliders are now accompanied with a value bubble to make it easier understanding what the actual value is.

### Fixed
- Reset button now properly performs a factory reset. Previously it was executing a restart.
- OTA upload was behaving erratically caused by incorrectly implementing the asynchronous 'onProgress' method (Wrong datatypes used).

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
