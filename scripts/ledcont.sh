#!/bin/bash

if [ $1 == 1 ]
then
    echo 1 | sudo tee /sys/class/leds/input2\:\:capslock/brightness
elif [ $1 == 0 ]
then
    echo 0 | sudo tee /sys/class/leds/input2\:\:capslock/brightness
else 
    echo "Please provide 1 or 0 to enable or disable the led"
fi