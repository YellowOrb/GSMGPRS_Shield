#ifndef gprs_h
#define gprs_h
#include "GSM.h"

class GPRS : public GSM {

public:
	virtual NetworkStatus_t attachGPRS(const char * const domain, const char * const username=NULL, const char * const  password=NULL);
	virtual void detachGPRS();
	virtual bool isGPRSAvailable();
		
	virtual char* getIP(char* ip, int length)=0;
};

#endif