#!/bin/bash

TMP_PATH="./"
APP_NAME="rfa"
AVRGCC="$(which avr-gcc)"
AVROBJCOPY="$(which avr-objcopy)"
#AVRGCC="/storage/dev/Develop/Atmel/avr8-gnu-toolchain-linux_x86_64/bin/avr-gcc"
#AVROBJCOPY="/storage/dev/Develop/Atmel/avr8-gnu-toolchain-linux_x86_64/bin/avr-objcopy"

MMCU="atmega128rfa1"
FLAGS="-O2 -Wall -std=gnu99 -mmcu=$MMCU"

SOURCES="./main.c"
#SOURCES+=" ./SPI/SPIManager.c"
#SOURCES+=" ./MsgGateway/RfaMsgGtw.c"
#SOURCES+=" ./Containers/Queue.c"

#LwMesh
SOURCES+=" ../LwMesh/hal.c"
SOURCES+=" ../LwMesh/halSleep.c"
SOURCES+=" ../LwMesh/halTimer.c"
SOURCES+=" ../LwMesh/halUart.c"
SOURCES+=" ../LwMesh/phy.c"
SOURCES+=" ../LwMesh/nwk.c"
SOURCES+=" ../LwMesh/nwkDataReq.c"
SOURCES+=" ../LwMesh/nwkFrame.c"
SOURCES+=" ../LwMesh/nwkGroup.c"
SOURCES+=" ../LwMesh/nwkRoute.c"
SOURCES+=" ../LwMesh/nwkRouteDiscovery.c"
SOURCES+=" ../LwMesh/nwkRx.c"
SOURCES+=" ../LwMesh/nwkSecurity.c"
SOURCES+=" ../LwMesh/nwkTx.c"
SOURCES+=" ../LwMesh/sys.c"
SOURCES+=" ../LwMesh/sysEncrypt.c"
SOURCES+=" ../LwMesh/sysTimer.c"

#---- main.c ----
$AVRGCC $FLAGS $SOURCES -o $TMP_PATH$APP_NAME".elf"
$AVROBJCOPY -j .text -j .data -O ihex $TMP_PATH$APP_NAME".elf" $TMP_PATH$APP_NAME".hex"
