## Arduino HTTP IR Remote

The program allows the board to receive and decode keys from a generic IR remote
and to send HTTP requests to a local server.

In this project I'm controlling a private server that can handle JSON and control the smart light bulb I have connected.


Made for NodeMCU 1.0 using IRremote library with a connected IR Receiver module.

### Circuit

```
IRReceiver Module   VCC GND DAT
                     |   |   |
NodeMCU V1.0        3V3 GND  D2
```
