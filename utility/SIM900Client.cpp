#include "SIM900Client.h"

SIM900Client::SIM900Client(SIM900GPRS *gprs): GPRSClient(gprs)
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
	// +CIPSEND
	_gprs->_cell.println(F("AT+CIPSEND"));

	if(!_gprs->readAndCheckResponse(PSTR(">"))) {
		// Serial.println(F("No >"));
		return;
	}
	_state = SENDING;
}

/**
 * Stops writing data to a server.
 */
void SIM900Client::endWrite() 
{
	write('\032');
	if(!_gprs->readAndCheckResponse(PSTR("SEND OK"), 2, 10000)) {
		// Serial.print(F("No SEND OK"));
		return;
	}
	_state = CLIENT_CONNECTED; // back to client connected state
}

// the two connect methods for the class differ very little, only if the host is given by naem or ip
// To have easy to manage code we split that into internaConnect1 and internalConnect2
void SIM900Client::internalConnect1() {
	_gprs->detachGPRS();
	_gprs->_cell.print(F("AT+CIPSTART=\"TCP\",\"")); // RETURNS: +CGATT: 1  OK	
}
// here is the host sent to modem
int SIM900Client::internalConnect2(uint16_t port) {
	_gprs->_cell.print(F("\","));
	_gprs->_cell.println(port,DEC);

	if(!_gprs->successfulResponse()) {
		return false;
	}
	
	if(!_gprs->readAndCheckResponse(PSTR("CONNECT OK"), 0, 5000)) {
		// if we did not get CONNECT OK something went wrong
		_gprs->_cell.readBytes(_gprs->_buffer, _gprs->_bufferSize);
		// read the STATE in the response
		char * stateStr = strstr_P(_gprs->_buffer, PSTR("STATE: "));
		if(NULL!=stateStr) {
			char * end = strchr(stateStr,'\r');
			end[0] = 0;
			int status = _gprs->parseConnectionStatus(stateStr+7); // 7 is the length of "STATE: "
			_gprs->_status = status==CONNECT_OK?CONNECTED:IDLE;
		}
		return false;
	}
	_gprs->_status = _gprs->getConnectionStatus()==CONNECT_OK?CONNECTED:IDLE;
	_state = CLIENT_CONNECTED;
	return true;
}

/**
 * Connects to a specified IP address and port. The return value indicates success or failure.
 */
int SIM900Client::connect(IPAddress ip, uint16_t port)
{ 
	internalConnect1();
	ip.printTo(_gprs->_cell);
	return internalConnect2(port);
}

/**
 * Connects to a specified host and port. The return value indicates success or failure.
 */
int SIM900Client::connect(const char *host, uint16_t port)
{
	internalConnect1();
	_gprs->_cell.print(host);
	return internalConnect2(port);	
}

/**
 * Disconnects from the server
 */
void SIM900Client::stop()
{
	if(SENDING == _state) {
		_gprs->_cell.println('\032'); //	ctrl+z
		// if successful a SEND OK will follow
		// if not successful SEND FAIL or +CME ERROR <n> will follow
		// after this TCP socket response will follow and end with a CLOSE if remote server closes connection

		if(_gprs->readAndCheckResponse(PSTR("SEND OK"))) {
		}
	}
	ConnectionStatus_t status = _gprs->getConnectionStatus();
	if(CONNECT_OK==status || CONNECTING == status) {
		_gprs->_cell.println(F("AT+CIPCLOSE"));
				
		if(!_gprs->readAndCheckResponse(PSTR("CLOSE OK"), -1, 3000)) {
			return;
		}
		_state = DISCONNECTED;
	}
	_gprs->_status = _gprs->getConnectionStatus()==CONNECT_OK?CONNECTED:IDLE;
}