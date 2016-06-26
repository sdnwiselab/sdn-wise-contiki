#!/bin/bash
TARGET="stm32f401re-nucleo-spirit1" 
BOARD="ids01a4"

mkdir cooja_firmwares

make TARGET=$TARGET BOARD=$BOARD DEFINES=COOJA=0,SINK=1 
arm-none-eabi-objcopy -O binary sdn-wise.$TARGET sdn-wise.$TARGET
mv sdn-wise.$TARGET cooja_firmwares/sink.$TARGET 

make TARGET=$TARGET BOARD=$BOARD DEFINES=COOJA=0,SINK=0
arm-none-eabi-objcopy -O binary sdn-wise.$TARGET sdn-wise.$TARGET
mv sdn-wise.$TARGET cooja_firmwares/node.$TARGET 

make TARGET=$TARGET BOARD=$BOARD clean
