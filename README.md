# GSMGPRS Shield library

This library is an attempt to create a unified library for all kinds of GSM/GPRS Shields that exists for the Arduino. The idea is not to implement support for all of them but rather define a small number of classes that implementations can inherit from and thus make it easy for a sketch developer to keep his/her sketch code but change the hardware. Preferably the sketch developer would even exchange the GSM/GPRS Shield for a Wifi or Ethernet based one.

NB Seems there already exists a library like this http://arduino.cc/en/Reference/GSM but at the momenet I cannot find the source code. Probably the code is at https://github.com/arduino/Arduino/tree/master/libraries/GSM. Will see how easy that is to adapt to new hardware.

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
