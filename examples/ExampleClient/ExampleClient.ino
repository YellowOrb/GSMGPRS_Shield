#include <SIM900.h>

#define debug Serial1

SIM900GPRS gsm;
SIM900Client client(&gsm);

char http_cmd[] = "GET /test.html HTTP/1.0\r\nHost: www.yelloworb.com\r\n\r\n";

#define BUFFER_SIZE 512
char buffer[BUFFER_SIZE];
int ret=0;
char ipStr[17];
char imeiStr[16];
long _startTime;

size_t freeRam () {
  return RAMEND - size_t (__malloc_heap_start);
} // end of freeRam
  
void setup(){
  Serial.begin(57600);
  debug.begin(115200);
  debug.println(F("Test of GSMGPRS_Shield"));
  debug.print(F("GSM lib "));
  debug.println(GSM_LIB_VERSION);
  debug.print(F("SIM900 lib "));
  debug.println(SIM900_GPRS_VERSION);
  debug.print(F("Free memory = "));
  debug.println(freeRam());
  
  // try to turn on the GSM Shield, stop if we fail  
  if(!gsm.turnOn()) {
    debug.println(F("Failed to turn on!"));
    goto STOP;
  }
  debug.println(F("Module on"));
  
  // initialize GSM
  for(ret=0; ret<15 && GSM_READY != gsm.begin(); ret ++ ) {
    debug.println("gsm setup problem");
    delay(1000);
  }
  debug.println(F("GSM ready"));

  debug.print(F("Signal strength: "));
  debug.println(gsm.getSignalStrength());

  gsm.getIMEI(imeiStr, 16);
  debug.print(F("IMEI: "));
  debug.println(imeiStr);
  
  // wait until GPRS is available
  for(ret=0; ret<15 && !gsm.isGPRSAvailable(); ret++) {
    debug.println("no gprs yet");
    delay(1000);
  }
  if(ret == 15) goto STOP;
  debug.println(F("GPRS is available, attach!"));
  
  for(ret=0; ret<15 && GPRS_READY != gsm.attachGPRS("online.telia.se", NULL, NULL); ret++) {
      debug.println("gprs join network error");
      delay(1000);
  }
  if(ret == 15) goto STOP;
  debug.println(F("GPRS ready"));
  
  gsm.getIP(ipStr, 17);
  debug.print(F("IP Address is "));
  debug.println(ipStr);
  
  if(false == client.connect("www.yelloworb.com", 80)) {
      debug.println(F("connect error"));
      goto STOP;
  }
  debug.println(F("connect success"));
  

  debug.println(F("waiting to fetch..."));
  client.beginWrite();
  client.print(http_cmd);
  client.endWrite();

  _startTime = millis();
  while (client.connected()) {
    int available = client.available();
    if(available>0) {
      while(available>0) {
        buffer[ret++] = client.read();
        available--;
      }
    }
    if( (millis() - _startTime) > 1000) {
      break; ; // timed out
    }     
  }
  buffer[ret] = '\0';
  debug.print(F("Recv: "));
  debug.print(ret);
  debug.println(F(" bytes: "));
  debug.println(buffer);
  debug.println(F("fetch over..."));
  
  STOP:
  client.stop();
  gsm.detachGPRS(); 
  gsm.shutdown();
  debug.print(F("Free memory = "));
  debug.println(freeRam());
  debug.println(F("All Done!"));
}

void loop(){
}
