#!/bin/sh

TMP_PATH="./"
APP_NAME="rfa"
AVRDUDE="$(which avrdude)"
#AVRDUDE="/media/fox/b12b4036-a226-45b3-af74-94b408258fc1/AtmelTools/avrdude-6.1/avrdude"
MMCU="m128rfa1"
#PROGRAMMER="atmelice_isp"
PROGRAMMER="usbasp"

echo $AVRDUDE -c $PROGRAMMER -P usb -p $MMCU -U lfuse:w:0xf7:m
$AVRDUDE -c $PROGRAMMER -P usb -p $MMCU -U lfuse:w:0xf7:m

