#ifndef gprs_h
#define gprs_h
#include "GSM.h"

class GPRS : public GSM {

public:
	int attach(char* domain, char* dom1, char* dom2);
	int detach();
};
#endif