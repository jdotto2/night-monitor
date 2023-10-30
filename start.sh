#!/bin/bash

# Author: Joshua Dotto
# Date: 2023-10-29

# This file serves as the entry point for 
# the night-monitor project

curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
sudo service udev restart
sudo usermod -a -G dialout $USER
sudo usermod -a -G plugdev $USER
cd end-device && pio run -t upload
cd ../gateway && python3 main.py
