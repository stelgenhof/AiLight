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

#include "AiLight.hpp"

AiLightClass::AiLightClass(void) {
  _my9291 = new my9291(MY9291_DI_PIN, MY9291_DCKI_PIN, MY9291_COMMAND_DEFAULT);

  setRGBW(); // Initialise colour channels
}

AiLightClass::AiLightClass(const AiLightClass &obj) {
  _my9291 = new my9291(MY9291_DI_PIN, MY9291_DCKI_PIN, MY9291_COMMAND_DEFAULT);
  *_my9291 = *obj._my9291;

  setRGBW(); // Initialise colour channels
}

AiLightClass::~AiLightClass(void) { delete _my9291; }

uint8_t AiLightClass::getBrightness(void) { return _brightness; }

void AiLightClass::setBrightness(uint16_t level) {
  _brightness = constrain(level, 0, MY9291_LEVEL_MAX); // Force boundaries

  setRGBW();
}

bool AiLightClass::getState(void) { return _my9291->getState(); }

void AiLightClass::setState(bool state) { _my9291->setState(state); }

Color AiLightClass::getColor(void) { return _color; }

void AiLightClass::setColor(uint8_t red, uint8_t green, uint8_t blue) {
  _color.red = red;
  _color.green = green;
  _color.blue = blue;

  setRGBW();
}

void AiLightClass::setWhite(uint8_t white) {
  _color.white = white;

  setRGBW();
}

uint16_t AiLightClass::getColorTemperature(void) { return _colortemp; }

void AiLightClass::setColorTemperature(uint16_t temperature) {
  Color ctColor = colorTemperature2RGB(temperature);

  _color.red = ctColor.red;
  _color.green = ctColor.green;
  _color.blue = ctColor.blue;

  setRGBW();
}

Color AiLightClass::colorTemperature2RGB(uint16_t temperature) {
  _colortemp = temperature; // Save colour temperature setting
  Color ctColor;

  temperature = (temperature == 0) ? 1 : temperature; // Avoid division by zero

  // Convert from mired value to relative Kelvin temperature. The temperature
  // must fall between 1000 and 40000 degrees. All calculations require
  // tmpKelvin \ 100, so only do the conversion once
  uint16_t tmpKelvin = constrain(1000000UL / temperature, 1000, 40000) / 100;

  // Perform conversions from colour temperature to RGB values

  // Red
  float red = (tmpKelvin <= 66)
                  ? MY9291_LEVEL_MAX
                  : 329.698727446 * pow((tmpKelvin - 60), -0.1332047592);

  ctColor.red = constrain(red, 0, MY9291_LEVEL_MAX); // Force boundaries

  // Green
  float green = (tmpKelvin <= 66)
                    ? 99.4708025861 * log(tmpKelvin) - 161.1195681661
                    : 288.1221695283 * pow(tmpKelvin, -0.0755148492);

  ctColor.green = constrain(green, 0, MY9291_LEVEL_MAX); // Force boundaries

  // Blue
  float blue = (tmpKelvin >= 66)
                   ? MY9291_LEVEL_MAX
                   : ((tmpKelvin <= 19) ? 0
                                        : 138.5177312231 * log(tmpKelvin - 10) -
                                              305.0447927307);

  ctColor.blue = constrain(blue, 0, MY9291_LEVEL_MAX); // Force boundaries

  return ctColor;
}

bool AiLightClass::hasGammaCorrection(void) { return _gammacorrection; }

void AiLightClass::useGammaCorrection(bool gamma) {
  _gammacorrection = gamma;
  setRGBW();
}

void AiLightClass::setRGBW() {
  uint8_t red =
      (_gammacorrection) ? pgm_read_byte(&gamma8[_color.red]) : _color.red;
  uint8_t green =
      (_gammacorrection) ? pgm_read_byte(&gamma8[_color.green]) : _color.green;
  uint8_t blue =
      (_gammacorrection) ? pgm_read_byte(&gamma8[_color.blue]) : _color.blue;

  _my9291->setColor((my9291_color_t){
      (uint32_t)map(red, 0, MY9291_LEVEL_MAX, 0, _brightness),
      (uint32_t)map(green, 0, MY9291_LEVEL_MAX, 0, _brightness),
      (uint32_t)map(blue, 0, MY9291_LEVEL_MAX, 0, _brightness),
      (uint32_t)map(_color.white, 0, MY9291_LEVEL_MAX, 0, _brightness)});
}

AiLightClass AiLight;
