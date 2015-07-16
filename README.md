# GSMGPRS Shield library

This library is an attempt to create a unified library for all kinds of GSM/GPRS Shields that exists for the Arduino. The idea is not to implement support for all of them but rather define a small number of classes that implementations can inherit from and thus make it easy for a sketch developer to keep his/her sketch code but change the hardware. Preferably the sketch developer would even exchange the GSM/GPRS Shield for a Wifi or Ethernet based one.

## Smaller footprint
There exists an official similar library http://arduino.cc/en/Reference/GSM but in my view it is over-designed. Take a look at the source code at https://github.com/arduino/Arduino/tree/master/libraries/GSM. Another issue with this is that it consumes a lot of both ROM and RAM. As an example looking at the WebClient example which is almost identical with the GsmWebClient it becomes quite obvious. Compiled for the Nano one gets the following:

### WebClient(using this library)
Sketch uses 9,778 bytes (31%) of program storage space. Maximum is 30,720 bytes.
Global variables use 684 bytes (33%) of dynamic memory, leaving 1,364 bytes for local variables. Maximum is 2,048 bytes.

### GsmWebClient(using the official library)
Compiling the sketch generates the following memory usage report from the Arduino IDE as an indication of its memory print.
```
Sketch uses 16,948 bytes (55%) of program storage space. Maximum is 30,720 bytes.
Global variables use 1,053 bytes (51%) of dynamic memory, leaving 995 bytes for local variables. Maximum is 2,048 bytes.
```

## Current Status
 * At the moment only implemented for the [SIM900](http://www.simcom.ee/modules/gsm-gprs/sim900/) module.
 * 2015-03-31 - Tested some different libraries using HardwareSerial. Not 100% stable at 115 200 baud, misses data but at 57 600 baud all works fine. SoftwareSerial has not been tested.

## Known issues
Though there currently exists a number of problems for this to really become a reality:
 * WifiServer and EthernetServer are [not replaceble](https://groups.google.com/a/arduino.cc/forum/#!topic/developers/0tn0E5Uy_-A)

## Existing libraries
  * Seeed Studio libraries - Seeed Studio provides the following libraries
    * [GPRS_Shield_Suli](https://github.com/Seeed-Studio/GPRS_Shield_Suli)
    * [GPRSInterface](https://github.com/Seeed-Studio/GPRSInterface)
    * [Xively GPRS](https://github.com/Seeed-Studio/Xively_GPRS) includes probably a copy of any of the other Seeed Studio libraries
    * [Seeeduino GPRS](https://github.com/Seeed-Studio/Seeeduino_GPRS) is yet another version but for a slightly different chip, the SIM800 instead of the SIM900.
  
  * [GSM Shield Arduino](https://code.google.com/p/gsm-shield-arduino/) is the library with the official website http://www.gsmlib.org. The code has been migrated from google code to the GitHub repo [GSM-GPRS-GPS-Shield](https://github.com/MarcoMartines/GSM-GPRS-GPS-Shield). This library has also been copied into a four different GitHub repos(maybe people should learn to fork :) ):
    * https://github.com/tomn46037/GSMSHIELD
    * https://github.com/jefflab/GSMSHIELD
    * https://github.com/per/gsmshield
    * https://github.com/fauzanqadri/GSMSHIELD
  * Though that code has also been further developed into the library for the GBoard that also exists at a number of different places on GitHub. 
    * [GSM_Shield](https://github.com/jgarland79/GSM_Shield) - this seems to be the "official" version
    * [GBoard GSM](https://github.com/tuletech/gboard_gsm)
  * [GSM-ARDUINO](https://github.com/IEFRD/GSM-ARDUINO) is yet another library
  
None of these libraries seems to have read the [Arduino API Style Guide](http://arduino.cc/en/Reference/APIStyleGuide) that among other things state: "For network applications, use the Client and Server libraries as the basis" since none of them are based on these.

## Other existing libraries that could possibly use this library:
 * [HttpClient](https://github.com/amcewen/HttpClient) by Adrian McEwen
 * [HTTPClient](https://github.com/interactive-matter/HTTPClient) by Interactive Matter(does not use Client but EthernetClient)
 * [uHTTP](https://github.com/nomadnt/uHTTP)
