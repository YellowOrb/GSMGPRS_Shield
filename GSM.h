#ifndef gsm_h
#define gsm_h
#include <Arduino.h>
#define GSM_LIB_VERSION F("v0.3")

//#define DEBUG

#define RESPONSE_BUFFER_SIZE 48 // enough to get modem response
#define TIMEOUT 400 //Default time out, 400 ms

enum NetworkStatus_t { 
	ERROR, IDLE, CONNECTING, GSM_READY, GPRS_READY, TRANSPARENT_CONNECTED, OFF
};

class GSM {

protected:
	Stream* _cell; // a Stream to the GSM unit, for instance a HardwareSerial or SoftwareSerial
#ifdef DEBUG
	Stream* _debug = 0; // a possible output stream
#endif
	char _internalBuffer[RESPONSE_BUFFER_SIZE];
	char* _buffer = _internalBuffer;
	int _bufferIndex = 0;
	int _bufferSize  = RESPONSE_BUFFER_SIZE;
	NetworkStatus_t _status = CONNECTING;

	bool readAndCheckResponse(const char* expected, int readBeyond=-1, int timeout=TIMEOUT);
	bool successfulResponse(int timeout=TIMEOUT);
	void print_P(const char* str);
	void hexPrint(const char* str);
	
public:
	GSM();
	GSM(Stream* serial);
#ifdef DEBUG
	GSM(Stream* serial, Stream* debug);
#endif

	NetworkStatus_t begin(char* pin=0, bool restart=true);
	NetworkStatus_t getNetworkStatus(void);
	void setBuffer(char* buffer, int size);
	void send(char *buffer);
	
	virtual bool shutdown()=0;
	virtual char* getIMEI(char* imei, int length)=0;
  virtual int getSIMStatus(void)=0;

  virtual int sendSMS(char* number, char* data)=0;
  virtual int readSMS(int messageIndex, char *message, int length)=0;
  virtual int deleteSMS(int index)=0;

  virtual int callUp(char* number)=0;
  virtual int answer(void)=0;
	virtual char getPhoneNumber(byte position, char* phone_number)=0;
	virtual char writePhoneNumber(byte position, char* phone_number)=0;
	virtual char deletePhoneNumber(byte position)=0;
	
	friend class GSMClient;
	friend class SIM900Client;
};

#endif