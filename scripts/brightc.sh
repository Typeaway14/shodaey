#!/bin/bash
#date: 24-08-2022
#author: Typeaway14
#purpose: control display brightness more precisely than keyboard hotkey

#use xrandr to find display 'name'. eDP on my laptop

VALUE=${1:-1}

if [[ $VALUE < 1.50001 ]];
then
	#xrandr --output 'name' --brightness $VALUE
	xrandr --output eDP --brightness $VALUE
else
	echo "Please provide value lesser than 1"
fi
