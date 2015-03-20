#ifndef gprs_h
#define gprs_h
#include "GSM.h"

class GPRS : public GSM {

public:

	GPRS():GSM(){}
	GPRS(Stream* serial):GSM(serial){}
#ifdef DEBUG
	GPRS(Stream* serial, Stream* debug):GSM(serial, debug){}
#endif

	virtual NetworkStatus_t attachGPRS(const char * const domain, const char * const username=NULL, const char * const  password=NULL);
	virtual void detachGPRS();
	virtual bool isGPRSAvailable();
		
	virtual char* getIP(char* ip, int length)=0;
};

#endif