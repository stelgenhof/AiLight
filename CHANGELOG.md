![AiLight](https://raw.githubusercontent.com/wiki/stelgenhof/AiLight/images/ailight_logo.png)

# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) and this project adheres to [Semantic Versioning](http://semver.org).

## [Unreleased]

### Added
- my92XX Support. This allows for support of light bulbs with the my9231 LED driver such as the LOHAS brand bulbs
. [\#50](https://github.com/stelgenhof/AiLight/pull/50) ([Nick Wolff] (https://github.com/darkfiberiru))

### Changed
- The example PlatformIO configuration has been changed due to the release of PlatformIO v4.0. If you use PlatformIO make sure the configuration entry 'env_default' is renamed to 'default_envs' in your platformio.ini file!
- Updated third-party dependencies.

### Fixed
- Connection to the MQTT broker was assumed to be always present and in case of a disconnect, a retry would be
 triggered. However in the case of the MQTT broker becoming unavailable, there was no possibility to get the
  connection back except for restarting the device. This has been resolved by checking at regular intervals if the
   connection is still there. [\#56](https://github.com/stelgenhof/AiLight/issues/56), [\#66](https://github.com
   /stelgenhof/AiLight/issues/66).
- Building with VSCode + PlatformIO 4.0 extension gives error: "`.text' will not fit in region `iram1_0_seg'". [\#59](https://github.com/stelgenhof/AiLight/pull/59) ([Donnie] (https://github.com/donkawechico))
- Corrected duplicate 'platform' JSON definition (to 'schema') for the MQTT discovery message. [\#55](https://github.com/stelgenhof/AiLight/pull/55) ([Ole-Kenneth] (https://github.com/olekenneth))
- Added link type (MIME type) for the HTML stylesheet to avoid potential misinterpretations.

### Removed
- LiteServer for testing the UI as it was hardly used.


## [0.6.0] - 2019-04-08

### Added
- Included additional HTTP response headers for the Web UI to increase security.
- As it may be hard to identify which ESP Core version is used, this Core version information has been added to the About page on the Web UI and the debug output.
- Enabled Travis CI so the AiLight firmware gets build and tested with each change.
- Allow Gamma Correction option to be set via REST API and MQTT. [\#23](https://github.com/stelgenhof/AiLight/issues/23)
- Possibility to define the state (ON/OFF) of the light upon power on / reset. Three options are available: Always On, Always Off and As Before. This can be extremely useful if for example you have your light connected to a regular switch and like it to behave like a general light switch. [\#24](https://github.com/stelgenhof/AiLight/issues/24).
- MQTT availability topic for HomeAssistant is included and set to the same topic as the Last Will and Testament topic. [\#35](https://github.com/stelgenhof/AiLight/issues/35).

### Changed
- Locked down versions of the platform and dependent libraries to safeguard stability. Altered the debug flag in the platformio.ini configuration file to show all warnings.
- Home Assistant 0.84 (and up) changed the configuration syntax for the MQTT JSON platform type. A compiler directive is introduced to allow the firmware to be compiled for versions 0.84 or older of Home Assistant. [\#51](https://github.com/stelgenhof/AiLight/issues/51) 
- Added monitor_speed parameter for development environments in the platformio.ini configuration file to be able to override the default (9600).
- In addition to the hostname, if the MQTT state topic, command topic or state topic are changed, the device will re-register itself on Home Assistant via the MQTT Auto discovery (if enabled). These settings are part of the Home Assistant device configuration and therefor need to be re-initialized if they are changed.   
- Renamed the 'extra_script' parameter to 'extra_scripts' in the platformio.ini configuration file as 'extra_script' will be deprecated.
- Increased buffer for MQTT discovery payload. Payload was too small causing Home Assistant not to recognize all configuration options.
- Added DEBUG block for code parts that are only required in debug mode.
- Suppressed message during build and included step to remove prior generated files for the WebUI (gulpfile.js).
- Changed the name of the ESPAsyncWebServer library in the platformio.ini configuration file since original project has changed their naming.
- Updated e-mail address and copyright year.

### Fixed
- MQTT Reconnect timer was using the WiFi timeout directive rather than the applicable MQTT timeout directive. Created a new compiler directive to allow for setting the WiFi reconnect time to a custom value.
- Corrected OTA port argument in the platformio.ini configuration file (Needs equal sign)
- When settings in the Web UI were saved, the Lights tab was made active. This made verifying your changed settings quite cumbersome. After saving, the Settings tab now stays active. [\#14](https://github.com/stelgenhof/AiLight/issues/14).
- Removed the Program Memory Macros for all constants used with AsyncWebServer methods. These were causing random Software WDT resets and removing these were the remedy. [\#27](https://github.com/stelgenhof/AiLight/issues/27).
- Corrected debug flag in the platformio.ini configuration file.

### Removed

## [0.5.0] - 2017-08-25

### Added
- REST API
- Support for Home Assistant's MQTT Discovery. Have **AiLight** set up your light automatically!

### Changed
- Moved MQTT Discovery notify event and reconnect timer to device callback function. MQTT functions become decoupled from device implementation.
- Re-factored MQTT and WiFi connections using event driven methods that are executed asynchronously.
- Changed signature use of MQTT callback handlers (based on AsyncMQTT's own examples.)
- Updated Bulma CSS Framework to 0.4.4 (including other NPM packages).
- Clear EEPROM space before loading factory defaults.
- Replaced core function 'memcpy' with the ESP8266 SDK counterpart.

### Fixed
- #13 Ensured (new) configuration setting has a proper value before sending via WebSockets.
- Decreased refresh time as UI is available again within 10 seconds.

### Removed

## [0.4.1] - 2017-06-29

### Added
-	Altered validation of WiFi Passwords in the Web UI to allow for WiFi networks without a password.
- Added validation for WiFi SSID in the Web UI (required and cannot exceed 31 characters).
- Added Gulp task for compiling release binaries. Compiled binaries are now available from this version going forward.

### Changed
- The Web UI client side script has been upgraded to ES6 (ES2015).
-	Replaced core functions like 'strcpy', 'strcmp', etc. with the ESP8266 SDK counterpart.
- Existing WebSocket client connections are now disconnected upon OTA start.

### Fixed
- The OTA complete message in the Web UI was still showing multiple times due to a typo.

## [0.4.0] - 2017-06-17

### Added
- In stead of immediately switching to a new state, you can now make the transition to a desired state (i.e. colours, brightness, etc.) perform more gradually. This will change the light (cross fade) from the current to the next state given the specified time.
- Now the html.gz.h file is included for people that are not able to build this file themselves.
- When an OTA update has been initiated, a message window - with a nice progress bar - is being displayed in the Web UI to indicate the user that an update is in progress. Subsequently, the Web UI is now reloaded automatically.
- When the user chooses RESTART or RESET, a user friendly message window is being shown in the Web UI.
- Gulpfile now includes task for generating the gamma correction table.

### Changed
- Changed position of password visibility icon to be inside input box.
- Adjusted path to gulp binary in the 'build.py' file to provide better support Windows OS.

### Fixed
- Fixed issue #10: In the Web UI, the object holding the form's input values wasn't initialized, resulting in the user settings not being saved.
- Ensured OTA 'complete' message isn't shown multiple times.
- Fixed issue #8: `[Violation] Added non-passive event listener to a scroll-blocking 'touchmove' event` in Web UI.
- Included missing gulp-util package in the 'package.json' file.

### Removed
- SVG font (used for icons) as most browsers started not supporting it anymore. Helps reducing the size of the firmware.
- Removed unnecessary 'onConnect' handler.


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
- OTA upload was behaving erratically caused by incorrectly implementing the asynchronous 'onProgress' method (Wrong data-types used).

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
