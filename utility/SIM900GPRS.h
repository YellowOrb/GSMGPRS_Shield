#ifndef sim900gprs_h
#define sim900gprs_h
#include "GPRS.h"
#include <Time.h>

#define SIM900_GPRS_VERSION F("v0.1")

#define GSM_ON_PIN             9 // connect GSM Module turn ON to pin 77 

enum ConnectionStatus_t { 
	IP_INITIAL, 
	IP_START, 
	IP_CONFIG, 
	IP_GPRSACT,
	IP_STATUS,
	CONNECTING_LISTENING,
	CONNECT_OK,
	CLOSING,
	CLOSED,
	PDP_DEACT,
	UNKNOWN_GSM_STATUS
};

// this hardware implements both GSM and GPRS functionality, thus inherits from GPRS
class SIM900GPRS : public GPRS {
protected:
	ConnectionStatus_t parseConnectionStatus(char * str);
	ConnectionStatus_t getConnectionStatus();

public:
	SIM900GPRS();
	SIM900GPRS(Stream* serial);
#ifdef DEBUG
	SIM900GPRS(Stream* serial, Stream* debug);
#endif

	NetworkStatus_t begin(char* pin=NULL, bool restart=true);
	
	bool isGPRSAvailable();
	NetworkStatus_t attachGPRS(const char * const domain, const char * const username=NULL, const char * const  password=NULL);
	void detachGPRS();

	char* getIMEI(char* imei, int length);
	char* getIP(char* ip, int length);
	
	bool activateDateTime();
	bool deactivateDateTime();
	char* getTimeStr();
	time_t getUnixTime();
	
	bool turnOn();
	bool shutdown();
	
	int getSignalStrength();
		
	// Functions below not yet implemented
	int getSIMStatus(void);

  int sendSMS(char* number, char* data);
  int readSMS(int messageIndex, char *message, int length);
  int deleteSMS(int index);

  int callUp(char* number);
  int answer(void);
	char getPhoneNumber(byte position, char *phone_number);
	char writePhoneNumber(byte position, char *phone_number);
	char deletePhoneNumber(byte position);
	
	friend class SIM900Client;
	friend class SIM900Server;
};

#endif