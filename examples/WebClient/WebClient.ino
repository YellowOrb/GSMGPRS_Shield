/*
 GSM Web client

 This sketch connects to a website using a GSM shield.

 Circuit:
 * GSM shield attached

 created 20 October 2014
 by Karl-Petter Åkesson,
 based on work
 by Tom Igoe
 */

// libraries
#include <SoftwareSerial.h>
#include <SIM900.h>

#define debug Serial1

// PIN Number
#define PINNUMBER ""

// APN data
#define GPRS_APN       "online.telia.se" // replace your GPRS APN
#define GPRS_LOGIN     ""                // replace with your GPRS login
#define GPRS_PASSWORD  ""                // replace with your GPRS password

// initialize the library instance
SIM900GPRS gprs;
SIM900Client client(&gprs);

// This example downloads the URL "http://arduino.cc/"

// URL, path & port (for example: arduino.cc)
char server[] = "arduino.cc";
char path[] = "/asciilogo.txt";
int port = 80; // 80 for HTTP

void setup()
{
  // initialize serial communications
  debug.begin(115200);
  Serial.begin(57600); // must set the baudrate to the modem and initialize the serial port to the modem
  debug.println("Starting Arduino web client.");
  // connection state
  boolean notConnected = true;

  // Start GSM shield
  gprs.turnOn();
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while(notConnected)
  {
    if((gprs.begin()==GSM_READY) &
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD)==GPRS_READY))
      notConnected = false;
    else
    {
      debugln("Not connected");
      delay(1000);
    }
  }

  debugln("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, port))
  {
    debugln("connected");
    // Make a HTTP request:
    client.beginWrite();
    client.print("GET ");
    client.print(path);
    client.println(" HTTP/1.0");
    client.println();
    client.endWrite();
  } 
  else
  {
    // if you didn't get a connection to the server:
    debugln("connection failed");
  }
}

void loop()
{
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available())
  {
    char c = client.read();
    debug(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.available() && !client.connected())
  {
    debugln();
    debugln("disconnecting.");
    client.stop();

    // do nothing forevermore:
    for(;;)
      ;
  }
}
