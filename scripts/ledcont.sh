#!/bin/bash

if [ $1 == 1 ]
then
    echo 1 | sudo tee /sys/class/leds/input2\:\:capslock/brightness
else
    echo 0 | sudo tee /sys/class/leds/input2\:\:capslock/brightness
fi