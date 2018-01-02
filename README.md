SDN-WISE-CONTIKI
====================================
[![Build Status](https://travis-ci.org/sdnwiselab/sdn-wise-contiki.svg?branch=master)](https://travis-ci.org/sdnwiselab/sdn-wise-contiki)

The Open Source OS for the Internet of Things meets the Software Defined Networking.

### Build

To build SDN-WISE run the `sdn-wise/RUNME.sh` script.

To select the target for your build change the `TARGET` variable inside this script with one of the supported contiki [platforms](http://www.contiki-os.org/hardware.html).

To build SDN-WISE for Cooja emulated devices please set `COOJA_BUILD=1` otherwise set `COOJA_BUILD=0`.

The script will deploy two compiled firmwares inside `sdn-wise/cooja_firmware`. One for the sink of the network, called `sink.target`, and one for the other nodes, called `node.target`.


### Versioning

This project uses [semantic versioning](http://semver.org).

## Papers

Our approach is detailed in three scientific contributions:
```
@inproceedings{Galluccio:2015,
  author = {Paolo {Di Dio}  and Salvatore Faraci and Laura Galluccio and Sebastiano Milardo and Giacomo Morabito and Sergio Palazzo and Patrizia Livreri},
  booktitle = {2016 Mediterranean Ad Hoc Networking Workshop (Med-Hoc-Net)},
  doi       = {10.1109/MedHocNet.2016.7528421},
  title     = {{Exploiting state information to support QoS in Software-Defined WSNs}},
  year      = {2016},
  url       = {http://ieeexplore.ieee.org/document/7528421/},
}
```

```
@inproceedings{Galluccio:2015,
  author    = {Laura Galluccio and Sebastiano Milardo and Giacomo Morabito and Sergio Palazzo},
  booktitle = {2015 IEEE Conference on Computer Communications Workshops (INFOCOM WKSHPS)},
  doi       = {10.1109/INFCOMW.2015.7179322},
  title     = {{Reprogramming Wireless Sensor Networks by using SDN-WISE: A hands-on demo}},
  year      = {2015},
  url       = {http://ieeexplore.ieee.org/document/7179322/},
}
```

```
@inproceedings{Galluccio:2015,
  author    = {Laura Galluccio and Sebastiano Milardo and Giacomo Morabito and Sergio Palazzo},
  booktitle = {2015 IEEE Conference on Computer Communications (INFOCOM)},
  doi       = {10.1109/INFOCOM.2015.7218418},
  title     = {{SDN-WISE: Design, prototyping and experimentation of a stateful SDN solution for WIreless SEnsor networks}},
  year      = {2015},
  url       = {http://ieeexplore.ieee.org/document/7218418/},
}
```

