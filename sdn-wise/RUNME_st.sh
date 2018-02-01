#!/bin/bash
TARGET="stm32f401re-nucleo-spirit1" 
BOARD="ids01a4"
SENSORBOARD="iks01a1"

mkdir cooja_firmwares

make TARGET=$TARGET BOARD=$BOARD DEFINES=COOJA=0,SINK=1,_MY_ADDRESS=1,SDN_WISE_DEBUG=0 
arm-none-eabi-objcopy -O binary sdn-wise.$TARGET sdn-wise.$TARGET
mv sdn-wise.$TARGET cooja_firmwares/sink.$TARGET 
make TARGET=$TARGET BOARD=$BOARD clean


make TARGET=$TARGET BOARD=$BOARD DEFINES=COOJA=0,SINK=0,_MY_ADDRESS=2,SDN_WISE_DEBUG=1
arm-none-eabi-objcopy -O binary sdn-wise.$TARGET sdn-wise_2.$TARGET
mv sdn-wise_2.$TARGET cooja_firmwares/node_2.$TARGET 
make TARGET=$TARGET BOARD=$BOARD clean


make TARGET=$TARGET BOARD=$BOARD SENSORBOARD=$SENSORBOARD DEFINES=COOJA=0,SINK=0,_MY_ADDRESS=3,SDN_WISE_DEBUG=1
arm-none-eabi-objcopy -O binary sdn-wise.$TARGET sdn-wise_3.$TARGET
mv sdn-wise_3.$TARGET cooja_firmwares/node_3.$TARGET 
make TARGET=$TARGET BOARD=$BOARD SENSORBOARD=$SENSORBOARD clean


make TARGET=$TARGET BOARD=$BOARD SENSORBOARD=$SENSORBOARD DEFINES=COOJA=0,SINK=0,_MY_ADDRESS=4,SDN_WISE_DEBUG=1
arm-none-eabi-objcopy -O binary sdn-wise.$TARGET sdn-wise_4.$TARGET
mv sdn-wise_4.$TARGET cooja_firmwares/node_4.$TARGET 
make TARGET=$TARGET BOARD=$BOARD SENSORBOARD=$SENSORBOARD clean


make TARGET=$TARGET BOARD=$BOARD clean
