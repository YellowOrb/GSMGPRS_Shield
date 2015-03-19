#include "GSMClient.h"
#include "GPRS.h"

GSMClient::GSMClient(GPRS *gprs){
	_gprs = gprs;
}

/**
 * Returns whether or not the client is connected. A client is considered 
 * connected if the connection has been closed but there is still unread data.
 */
uint8_t GSMClient::connected()
{
	return _gprs->getNetworkStatus()==CONNECTED;	 
}

/**
 * Write data to the server that the client is connected to.
 * client.write(data)
 * client.write(buffer)
 * client.write(buffer, size)
 * Parameters
 * data: the value to write (byte or char)
 * buffer : an array of data (byte or char) to write
 * size : size of the buffer to write (byte)
 * Returns
 * byte - write() returns the number of bytes written.
 */
size_t GSMClient::write(uint8_t b) {
  return _gprs->_cell->write(b);
}
size_t GSMClient::write(const uint8_t *buf, size_t size) {
  return _gprs->_cell->write(buf, size);
}

/**
 * Returns the number of bytes available for reading (that is, the amount of
 * data that has been written to the client by the server it is connected to).
 * 
 * available() inherits from the Stream utility class.
 */
int GSMClient::available()
{
	// wait for reply
//	long _startTime = millis();		
//	while (_gprs->_cell->available() < 1){ 		    
//		if( (millis() - _startTime) > 200) {
//			 return 0; // timed out
//		} 
//	}
	return _gprs->_cell->available();
}

int GSMClient::internalRead(uint8_t *buf, size_t size, bool line) {
	size_t i;
	long endTime;
	for(i=0;i<size-1;i++) { /// fill the buffer except the last position which we reserve to put a 0 in
		// wait until we have something to read since read will return nonsens if data is not available
		// or return if no data is available
		endTime = millis() + 1000;
		while(available()==0) {
			if( millis() > endTime) {
				buf[i]='\0'; // end the string
				return i; // timed out
			}
		}
		buf[i]=_gprs->_cell->read();
		if(buf[i]==0) {
			break;
		}
		if(line && buf[i]=='\n') {
			if( (i>0) && (buf[i-1]=='\r') ) {
				buf[i-1]='\0';
				i--;
			} else {
				buf[i]='\0';
			}
			break;
		}
	}
	buf[i]='\0'; // end the string
	return i;
}

int GSMClient::readln(uint8_t *buf, size_t size)
{
	return internalRead(buf, size, true);
}

int GSMClient::read(uint8_t *buf, size_t size)
{
	return internalRead(buf, size, false);
}

int GSMClient::read()
{
	return _gprs->_cell->read();
}

int GSMClient::peek()
{
	return _gprs->_cell->peek();;
}

void GSMClient::flush()
{
	_gprs->_cell->flush();
}

GSMClient::operator bool() {
  return true;
}

bool GSMClient::operator==(const GSMClient& rhs) {
  return true;
}