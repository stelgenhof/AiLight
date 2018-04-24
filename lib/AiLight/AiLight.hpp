/**
 * AiLight Library
 *
 * AiLight is a simple library to control an AiLight that contains the MY9291
 * LED driver and encapsulates the MY9291 LED driver made by Xose Pérez
 *
 * This file is part of the Ai-Thinker RGBW Light Firmware.
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.

 * Created by Sacha Telgenhof <me at sachatelgenhof dot com>
 * (https://www.sachatelgenhof.nl)
 * Copyright (c) 2016 - 2018 Sacha Telgenhof
 */

#ifndef AiLight_h
#define AiLight_h

#include <my9291.h>

// MY9291 PIN settings
#define MY9291_DI_PIN 13
#define MY9291_DCKI_PIN 15

// The maximum level used for colour channels and brightness
#define MY9291_LEVEL_MAX 255

// Structure for holding the levels of all the colour channels

struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
};

// This table remaps linear input values to nonlinear gamma-corrected output
// values. The output values are specified for 8-bit colours with a gamma
// correction factor of 2.8
const static uint8_t PROGMEM gamma8[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,
    2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,
    4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,   8,
    8,   9,   9,   9,   10,  10,  10,  11,  11,  11,  12,  12,  13,  13,  13,
    14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,
    21,  22,  22,  23,  24,  24,  25,  25,  26,  27,  27,  28,  29,  29,  30,
    31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,  41,  42,
    43,  44,  45,  46,  47,  48,  49,  50,  50,  51,  52,  54,  55,  56,  57,
    58,  59,  60,  61,  62,  63,  64,  66,  67,  68,  69,  70,  72,  73,  74,
    75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,  90,  92,  93,  95,
    96,  98,  99,  101, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119,
    120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146,
    148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175, 177,
    180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252,
    255};

class AiLightClass {
public:
  AiLightClass(void);
  AiLightClass(const AiLightClass &obj);
  ~AiLightClass(void);

  /**
   * @brief Returns the current state of the AiLight (i.e on or off)
   *
   * @return the current state of the AiLight
   */
  bool getState(void);

  /**
   * @brief Sets the state of the AiLight (i.e on or off)
   *
   * @param state the desired state (true/false)
   *
   * @return void
   */
  void setState(bool state);

  /**
   * @brief Returns the currently set level of brightness
   *
   * @return the currently set level of brightness
   */
  uint8_t getBrightness(void);

  /**
   * @brief Sets the level for brightness
   *
   * This functions sets the level for brightness and switches on the AiLight.
   *
   * @param level the desired brightness level (range 0 - 255)
   *
   * @return void
   */
  void setBrightness(uint16_t level);

  /**
   * @brief Returns the currently set levels of all 4 colour channels (RGBW)
   *
   * @return the set levels of all 4 colour channels (RGBW)
   */
  Color getColor(void);

  /**
   * @brief Sets the levels for the red, green and blue colour channels
   *
   * This functions set the individual levels for the red, green and blue colour
   * channels and switches on the AiLight. Note that all four colour channels
   * work in conjunction: i.e. the white colour channel is not changed when the
   * levels of the RGB colour channels are changed.
   *
   * @param red the desired level for the red colour channel (range 0 - 255)
   * @param green the desired level for the green colour channel (range 0 - 255)
   * @param blue the desired level for the blue colour channel (range 0 - 255)
   *
   * @return void
   */
  void setColor(uint8_t red, uint8_t green, uint8_t blue);

  /**
   * @brief Sets the level for the white colour channel
   *
   * This functions set the individual level for white colour channel and
   * switches on the AiLight. Note that all four colour channels work in
   * conjunction: i.e. the RGB colour channels are not changed when the level of
   * the white colour channel is changed.
   *
   * @param white the desired level for the white colour channel (range 0 - 255)
   *
   * @return void
   */
  void setWhite(uint8_t white);

  /**
   * @brief Returns the currently set colour temperature
   *
   * @return the currently set colour temperature (in mired)
   */
  uint16_t getColorTemperature(void);

  /**
   * @brief Sets the colour of the AiLight based on the given colour temperature
   *
   * This method sets the colour of the AiLight based on the given colour
   * temperature and switches on the AiLight.
   *
   * @param temperature the desired colour temperature (in mired)
   */
  void setColorTemperature(uint16_t temperature);

  /**
   * @brief Converts a colour temperature to the equivalent RGB colours
   *
   * This method converts a given colour temperature to RGB colour values.
   * The colour temperature is defined in mired (micro reciprocal degree, the
   * mired is a unit of measurement used to express colour temperature).
   * (Conversion is based on the algorithm by Tanner Helland.)
   *
   * @param temperature the desired colour temperature (in mired)
   * @return Color a color object (RGBW) representing the given colour temperature
   *
   * Sources:
   * http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
   * https://en.wikipedia.org/wiki/Mired
   */
  Color colorTemperature2RGB(uint16_t temperature);

  /**
   * @brief Returns whether Gamma Correction is enabled or disabled
   *
   * @return the current status whether Gamma Correction is enabled or disabled
   */
  bool hasGammaCorrection(void);

  /**
   * @brief Use Gamma Correction or not (i.e on or off)
   *
   * Gamma correction controls the overall brightness of an image. Images which
   * are not properly corrected can look either bleached out, or too dark.
   * Trying to reproduce colors accurately also requires some knowledge of
   * gamma. Varying the amount of gamma correction changes not only the
   * brightness, but also the ratios of red to green to blue.
   *
   * Sources:
   * http://cgsd.com/papers/gamma_intro.html
   * https://learn.adafruit.com/led-tricks-gamma-correction/the-issue
   *
   * @param value the desired state for using Gamma Correction (true/false)
   *
   * @return void
   */
  void useGammaCorrection(bool gamma);

private:
  my9291 *_my9291; // MY9291 driver handle

  // Current colour levels (RGBW). Initial values are 1/4th of maximum
  Color _color = {MY9291_LEVEL_MAX >> 2, MY9291_LEVEL_MAX >> 2,
                  MY9291_LEVEL_MAX >> 2, 0};

  // Current brightness level. Initial value is 1/4th of maximum
  uint8_t _brightness = MY9291_LEVEL_MAX >> 2;

  // Current colour temperature setting. Initial value is equivalent of 2700K
  uint16_t _colortemp = 370;

  /**
   * @brief Sets the levels of all colour levels (RGBW) to the MY9291 LED
   * driver.
   *
   * This internal method sets the levels of all colour levels (RGBW) to the
   * MY9291 LED driver including the brightness level. To switch on the AiLight,
   * the setState() method needs to be used subsequently.
   *
   * @return void
   */
  void setRGBW();

  // Gamma correction is enabled or disabled
  bool _gammacorrection = false;
};

extern AiLightClass AiLight;
#endif
