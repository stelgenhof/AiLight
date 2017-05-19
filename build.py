#!/bin/python

# Ai-Thinker RGBW Light Firmware - PlatformIO Build script
#
# This file is part of the Ai-Thinker RGBW Light Firmware.
# For the full copyright and license information, please view the LICENSE
# file that was distributed with this source code.

# Created by Sacha Telgenhof <stelgenhof at gmail dot com>
# (https://www.sachatelgenhof.nl)
# Copyright (c) 2016 - 2017 Sacha Telgenhof

Import("env")

def before_build(source, target, env):
    env.Execute("$PROJECT_DIR/node_modules/.bin/gulp")

env.AddPreAction("$BUILD_DIR/src/main.ino.o", before_build)
