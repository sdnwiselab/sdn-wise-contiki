#!/bin/bash
TARGET="stm32f401re-nucleo-spirit1"

mkdir firmwares

make TARGET=$TARGET BOARD=ids01a4 clean
make TARGET=$TARGET BOARD=ids01a4 DEFINES=SINK=1 
mv sdn-wise.$TARGET sink.$TARGET 
make TARGET=$TARGET BOARD=ids01a4 DEFINES=SINK=0 
mv sdn-wise.$TARGET node.$TARGET

arm-none-eabi-objcopy -O binary sink.$TARGET firmwares/sink.bin
arm-none-eabi-objcopy -O binary node.$TARGET firmwares/node.bin
