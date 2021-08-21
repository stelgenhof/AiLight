#!/bin/python

# AiLight Firmware - PlatformIO Build script
#
# This file is part of the AiLight Firmware.
# For the full copyright and license information, please view the LICENSE
# file that was distributed with this source code.
#
# Created by Sacha Telgenhof <me at sachatelgenhof dot com>
# (https://www.sachatelgenhof.com)
# Copyright (c) 2016 - 2021 Sacha Telgenhof

Import("env")

def before_build(source, target, env):
    env.Execute("$PROJECT_DIR/node_modules/.bin/gulp")

env.AddPreAction("$BUILD_DIR/src/main.ino.cpp.o", before_build)
