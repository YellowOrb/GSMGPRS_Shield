#include "SIM900Client.h"

SIM900Client::SIM900Client(SIM900GPRS *gprs): GSMClient(gprs)
{
	_gprs = gprs;
};


ClientState_t SIM900Client::getClientState(void)
{
	return _state;
}

/**
 * Tells the client to start writing to the server it is connected to.
 */
void SIM900Client::beginWrite()
{
#ifdef DEBUG
	_gprs->_debug->print(millis()); _gprs->_debug->println(F(" AT+CIPSEND"));
#endif
	// +CIPSEND
	_gprs->_cell->println(F("AT+CIPSEND"));

	if(!_gprs->readAndCheckResponse(PSTR(">"), 0)) {
		return;
	}
	_state = SENDING;
}

/**
 * Stops writing data to a server.
 */
void SIM900Client::endWrite() 
{
#ifdef DEBUG
	_gprs->_debug->print(millis()); _gprs->_debug->println(F(" ctrl-z"));
#endif
	int i = 0;
	for(i=0;i<5;i++) {
		write('\032');
		if(_gprs->readAndCheckResponse(PSTR("SEND OK"), 2, 1000)) {
			_state = CLIENT_CONNECTED; // back to client connected state
			return;
		}
	}
}

// the two connect methods for the class differ very little, only if the host is given by name or ip
// To manage the code easier it is split into internalConnect1 and internalConnect2
void SIM900Client::internalConnect1() {
	// make sure we are in correct state, see SIM900_AT_Command_Manual_V1.09 page 187
	ConnectionStatus_t status = _gprs->getConnectionStatus();
	if(!(status == IP_INITIAL || status == IP_STATUS)) {
		// close down any left GPRS connections or similar for comming commands to work, brings chip back to state IP INITIAL
		_gprs->detachGPRS();
	}
#ifdef DEBUG
	_gprs->_debug->print(millis()); _gprs->_debug->println(F(" AT+CIPSTART="));
#endif
	_gprs->_cell->print(F("AT+CIPSTART=\"TCP\",\""));
}

// here is the host sent to modem, etiher as ip-number or as domain, see connect functions below
int SIM900Client::internalConnect2(uint16_t port) {
	_gprs->_cell->print(F("\","));
	_gprs->_cell->println(port,DEC);

	if(!_gprs->readAndCheckResponse(PSTR("OK\r\n"),0)) {
		return false;
	}
	
	if(!_gprs->readAndCheckResponse(PSTR("CONNECT OK"), 0, 15000)) {
		// if we did not get CONNECT OK something went wrong
		_gprs->_cell->readBytes(_gprs->_buffer, _gprs->_bufferSize);
		// read the STATE in the response
		char * stateStr = strstr_P(_gprs->_buffer, PSTR("STATE: "));
		if(NULL!=stateStr) {
			char * end = strchr(stateStr,'\r');
			end[0] = 0;
			int status = _gprs->parseConnectionStatus(stateStr+7); // 7 is the length of "STATE: "
			_gprs->_status = status==CONNECT_OK?TRANSPARENT_CONNECTED:IDLE;
		}
		return false;
	}
	_gprs->_status = _gprs->getConnectionStatus()==CONNECT_OK?TRANSPARENT_CONNECTED:IDLE;
	_state = CLIENT_CONNECTED;
	return true;
}

/**
 * Connects to a specified IP address and port. The return value indicates success or failure.
 * Will time out after 15 seconds
 */
int SIM900Client::connect(IPAddress ip, uint16_t port)
{ 
	internalConnect1();
	ip.printTo(*_gprs->_cell);
	return internalConnect2(port);
}

/**
 * Connects to a specified host and port. The return value indicates success or failure.
 * Will time out after 15 seconds
 */
int SIM900Client::connect(const char *host, uint16_t port)
{
	internalConnect1();
	_gprs->_cell->print(host);
	return internalConnect2(port);	
}

/**
 * Disconnects from the server
 */
void SIM900Client::stop()
{
	if(SENDING == _state) {
#ifdef DEBUG
		_gprs->_debug->print(millis()); _gprs->_debug->println(F(" ctrl-z in stop()"));
#endif
		_gprs->_cell->println('\032'); //	ctrl+z
		// if successful a SEND OK will follow
		// if not successful SEND FAIL or +CME ERROR <n> will follow
		// after this TCP socket response will follow and end with a CLOSE if remote server closes connection

		if(_gprs->readAndCheckResponse(PSTR("SEND OK"))) {
		}
	}
	
	ConnectionStatus_t status = _gprs->getConnectionStatus();
#ifdef DEBUG
	_gprs->_debug->print(millis()); _gprs->_debug->print(F(" connections status: ")); _gprs->_debug->println(status);
#endif
	if(CONNECT_OK==status || CONNECTING_LISTENING == status) {
#ifdef DEBUG
		_gprs->_debug->print(millis()); _gprs->_debug->println(F(" AT+CIPCLOSE"));
#endif
		_gprs->_cell->println(F("AT+CIPCLOSE"));
		
		// if not already closed will response with CLOSE OK but if closed +CME ERROR: 3 CLOSED, thus just look for ´CLOSE
		if(!_gprs->readAndCheckResponse(PSTR("CLOSE"), -1, 3000)) { 
			return;
		}
		_state = DISCONNECTED;
		_gprs->_status = _gprs->getConnectionStatus()==CONNECT_OK?TRANSPARENT_CONNECTED:IDLE;
	} else {
		_gprs->_status = IDLE;
	}
}