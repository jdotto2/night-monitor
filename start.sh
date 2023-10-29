#!/bin/bash

# Author: Joshua Dotto
# Date: 2023-10-29

# This file serves as the entry point for 
# the night-monitor project

cd end-device && pio run -t upload
cd ../gateway && python3 main.py
