#ifndef gprsclient_h
#define gprsclient_h
#include "Print.h"
#include "Server.h"
#include "IPAddress.h"

class GPRSServer : public Server {

public:
	GPRSServer();
  GPRSServer(uint8_t sock);

	uint8_t status();
	
	// the following methods comes from Server.h
 	virtual void begin() =0;
	
  virtual void available(Client *client);

  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);

  using Print::write;
};

#endif