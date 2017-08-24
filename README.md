# sdn-wise-contiki
[![Build Status](https://travis-ci.org/sdnwiselab/sdn-wise-contiki.svg?branch=master)](https://travis-ci.org/sdnwiselab/sdn-wise-contiki)

The Open Source OS for the Internet of Things meets the Software Defined Networking.

### Build

To build SDN-WISE run the `sdn-wise/RUNME.sh` script.

To select the target for your build change the `TARGET` variable inside this script with one of the supported contiki [platforms](http://www.contiki-os.org/hardware.html).

To build SDN-WISE for Cooja emulated devices please set `COOJA_BUILD=1` otherwise set `COOJA_BUILD=0`.

The script will deploy two compiled firmwares inside `sdn-wise/cooja_firmware`. One for the sink of the network, called `sink.target`, and one for the other nodes, called `node.target`.


### Versioning

This project uses [semantic versioning](http://semver.org).
