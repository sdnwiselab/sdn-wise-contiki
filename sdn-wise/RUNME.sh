#!/bin/bash
TARGET="sky"

make TARGET=$TARGET clean
make TARGET=$TARGET DEFINES=SINK=1
mv sdn-wise.$TARGET sink.$TARGET 
make TARGET=$TARGET DEFINES=SINK=0
mv sdn-wise.$TARGET node.$TARGET

