#!/bin/bash
set -ev

sudo apt-get update -qq

sudo apt-get install cmake
sudo apt-get install libbluetooth-dev
sudo apt-get install libsdl-dev
sudo apt-get install freeglut3-dev
