/*
   A simple example making the AiLight showing a rainbow effect using the
   AiLight library.

   This file is part of the AiLight library.
   Copyright (c) 2017 Sacha Telgenhof (<stelgenhof at gmail dot com>). All
   rights reserved.

   For the full copyright and license information, please view the LICENSE
   file that was distributed with this source code.
 */

#include "AiLight.hpp"

#define RAINBOW_DELAY 10 // Time to wait for next colour (in milliseconds)

/**
 * @brief Show a rainbow effect
 *
 * Thanks to Xose Pérez for the rainbow effect code
 * (https://github.com/xoseperez/my9291)
 *
 * @param index an index to determine the colour to be displayed
 *
 * @return void
 */
void rainbowEffect(uint8_t index) {
    if (index < 85) {
        AiLight.setColor(index * 3, 255 - index * 3, 0);
    } else if (index < 170) {
        index -= 85;
        AiLight.setColor(255 - index * 3, 0, index * 3);
    } else {
        index -= 170;
        AiLight.setColor(0, index * 3, 255 - index * 3);
    }
}

/**
 * @brief Bootstrap/Initialisation
 */
void setup() {
    AiLight.setState(true); // Switch on the light
}

/**
 * @brief Main loop
 */
void loop() {
    static unsigned long previousMillis = millis();

    // Show the rainbow...
    if (millis() - previousMillis > RAINBOW_DELAY) {
        static uint8_t count = 0;
        previousMillis = millis();
        rainbowEffect(count++);
    }
}
