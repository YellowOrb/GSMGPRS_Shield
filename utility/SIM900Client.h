#ifndef sim900client_h
#define sim900client_h
#include "GPRSClient.h"
#include "SIM900GPRS.h"

enum ClientState_t { 
	DISCONNECTED,
	CLIENT_CONNECTED,
	SENDING,
	UNKNOWN_CLIENT_STATE
};


class SIM900Client : public GPRSClient {
private:
	void internalConnect1(void);
	int internalConnect2(uint16_t port);
	
protected:
	SIM900GPRS *_gprs;
	ClientState_t _state = DISCONNECTED;
	
public:
	SIM900Client(SIM900GPRS *gprs);
	
	ClientState_t getClientState(void);
	
	// the following methods comes from GPRSClient.h
	int connect(IPAddress ip, uint16_t port);
  int connect(const char *host, uint16_t port);
  void stop();

	void beginWrite();
	void endWrite();
};

#endif