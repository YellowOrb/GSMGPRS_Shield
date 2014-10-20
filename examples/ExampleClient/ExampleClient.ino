#include <SoftwareSerial.h>
#include <SIM900.h>

SIM900GPRS gsm;
SIM900Client client(&gsm);

char http_cmd[] = "GET /test.html HTTP/1.0\r\nHost: www.yelloworb.com\r\n\r\n";

#define BUFFER_SIZE 512
char buffer[BUFFER_SIZE];
int ret=0;
char ipStr[17];
char imeiStr[16];
long _startTime;

size_t freeRam ()
  {
  return RAMEND - size_t (__malloc_heap_start);
  } // end of freeRam
  
void setup(){
  Serial.begin(9600);
  Serial.println(F("Test of GSMGPRS_Shield"));
  Serial.print(F("GSM lib "));
  Serial.println(GSM_LIB_VERSION);
  Serial.print(F("SIM900 lib "));
  Serial.println(SIM900_GPRS_VERSION);
  Serial.print(F("Free memory = "));
  Serial.println(freeRam());
  
  // try to turn on the GSM Shield, stop if we fail  
  if(!gsm.turnOn()) {
    goto STOP;
  }
  Serial.println(F("Module on"));
  
  // initialize GSM
  while(gsm.begin() != GSM_READY) {
    Serial.println("gsm setup problem");
    goto STOP;
  }
  Serial.println(F("GSM ready"));

  Serial.print(F("Signal strength: "));
  Serial.println(gsm.getSignalStrength());

  gsm.getIMEI(imeiStr, 16);
  Serial.print(F("IMEI: "));
  Serial.println(imeiStr);
  
  // wait until GPRS is available
  while(!gsm.isGPRSAvailable()) {
    delay(500);
  }
  Serial.println(F("GPRS is available, attach!"));
  
  while(GPRS_READY != gsm.attachGPRS("online.telia.se", NULL, NULL)) {
      Serial.println("gprs join network error");
      goto STOP;
  }
  Serial.println(F("GPRS ready"));
  
  gsm.getIP(ipStr, 17);
  Serial.print(F("IP Address is "));
  Serial.println(ipStr);
  
  if(false == client.connect("www.yelloworb.com", 3000)) {
      Serial.println(F("connect error"));
      goto STOP;
  }
  Serial.println(F("connect success"));
  

  Serial.println(F("waiting to fetch..."));
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
  Serial.print(F("Recv: "));
  Serial.print(ret);
  Serial.println(F(" bytes: "));
  Serial.println(buffer);
  Serial.println(F("fetch over..."));
  
  STOP:
  client.stop();
  gsm.detachGPRS(); 
  gsm.shutdown();
  Serial.println(F("All Done!"));
}

void loop(){
}
