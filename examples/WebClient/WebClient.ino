/*
  Web client
 
 This sketch connects to a website (http://www.google.com)
 using a GPRS shield. 
 
 created 18 Dec 2014
 by Karl-Petter Åkesson,
 based on work
 by David A. Mellis, Tom Igoe & Adrian McEwen 
 */

#include <GPRSClient.h>
#include <GPRS.h>
#include <GSM.h>
char server[] = "www.google.com";    // name address for Google (using DNS)

GPRSClient client;

void setup() {
	// Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the GSM shield
  if (GSM.begin(9600) == 0) {
    Serial.println("Failed to configure GSM");
  }

  // attach GPRS
  if (GPRS.attach(9600) == 0) {
    Serial.println("Failed to attach GPRS");
  }
  Serial.println("connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
}

void loop()
{
  // if there are incoming bytes available 
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();

		GPRS.detach();
		GSM.end();
    // do nothing forevermore:
    while(true);
  }
}
