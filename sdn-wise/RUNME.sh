#!/bin/bash
TARGET="sky"

mkdir cooja_firmwares

make TARGET=$TARGET DEFINES=COOJA=1,SINK=1 
mv sdn-wise.$TARGET cooja_firmwares/sink.$TARGET 
make TARGET=$TARGET DEFINES=COOJA=1,SINK=0
mv sdn-wise.$TARGET cooja_firmwares/node.$TARGET 
make TARGET=$TARGET clean
