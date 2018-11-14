#!/bin/bash

# Installation
sudo apt-get install git-core
cd
git clone git://git.drogon.net/wiringPi
cd ~/wiringPi
git pull origin
cd ~/wiringPi
./build
cd
sudo apt install build-essential
mkdir src
mv blink.c src
cd src
gcc -Wall -o blink blink.c -lwiringPi
sudo cp blink /usr/local/bin
