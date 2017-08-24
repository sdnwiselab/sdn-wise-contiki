#!/bin/bash
TARGET="sky"
COOJA_BUILD=1

mkdir cooja_firmwares

make TARGET=$TARGET DEFINES=COOJA=$COOJA_BUILD,SINK=1 
mv sdn-wise.$TARGET cooja_firmwares/sink.$TARGET 
make TARGET=$TARGET DEFINES=COOJA=$COOJA_BUILD,SINK=0
mv sdn-wise.$TARGET cooja_firmwares/node.$TARGET 
make TARGET=$TARGET clean
