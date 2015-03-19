#ifndef gprsclient_h
#define gprsclient_h
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"
#include "GPRS.h"

class GSMClient : public Client {
	
protected:
	GPRS *_gprs;
	int internalRead(uint8_t *buf, size_t size, bool line);
	
public:
  GSMClient(GPRS *gprs);

	uint8_t status();
	
	// the following methods are inherited from from Client.h
	
	// connect is hardware dependent so those are just virutal here
  virtual int connect(IPAddress ip, uint16_t port)=0;
  virtual int connect(const char *host, uint16_t port)=0;
  
	size_t write(uint8_t);
  size_t write(const uint8_t *buf, size_t size);
  int available();
  int read();
  int read(uint8_t *buf, size_t size);
	int readln(uint8_t *buf, size_t size);
  int peek();
  void flush();
  virtual void stop()=0;
  uint8_t connected();
  operator bool();
  bool operator==(const GSMClient&);
  bool operator!=(const GSMClient& client) { return !this->operator==(client); };
};

#endif