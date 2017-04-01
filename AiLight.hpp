/*
  AiLight Library

  AiLight is a simple library to control an AiLight that contains the MY9291 LED
  driver and encapsulates the MY9291 LED driver made by Xose Pérez

  Copyright 2017 Sacha Telgenhof (stelgenhof@gmail.com). All rights reserved.

  For the full copyright and license information, please view the LICENSE
  file that was distributed with this source code.
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
   * @return void
   */
  void setBrightness(uint8_t level);

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
   * temperature and switches on the AiLight. The colour temperature is defined
   * in mired (micro reciprocal degree, the mired is a unit of measurement used
   * to express colour temperature). Subsequently, the colour temperature is
   * converted to equivalent RGB values as based on the algorithm by Tanner
   * Helland.
   *
   * Sources:
   * http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
   * https://en.wikipedia.org/wiki/Mired
   *
   */
  void setColorTemperature(uint16_t temperature);

private:
  my9291 *_my9291; // MY9291 driver handle

  // Current colour levels (RGBW). Initial values are 1/4th of maximum
  Color _color = {MY9291_LEVEL_MAX >> 2, MY9291_LEVEL_MAX >> 2,
                  MY9291_LEVEL_MAX >> 2, MY9291_LEVEL_MAX >> 2};

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
};

extern AiLightClass AiLight;
#endif
