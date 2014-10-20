#include "GPRSClient.h"
#include "GPRS.h"

GPRSClient::GPRSClient(GPRS *gprs){
	_gprs = gprs;
}

/**
 * Returns whether or not the client is connected. A client is considered 
 * connected if the connection has been closed but there is still unread data.
 */
uint8_t GPRSClient::connected()
{
	return _gprs->getNetworkStatus()==CONNECTED;	 
}

/**
 * Write data to the server the client is connected to.
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
size_t GPRSClient::write(uint8_t b) {
  return _gprs->_cell.write(b);
}
size_t GPRSClient::write(const uint8_t *buf, size_t size) {
  return _gprs->_cell.write(buf, size);
}

/**
 * Returns the number of bytes available for reading (that is, the amount of
 * data that has been written to the client by the server it is connected to).
 * 
 * available() inherits from the Stream utility class.
 */
int GPRSClient::available()
{
	// wait for reply
	long _startTime = millis();		
	while (_gprs->_cell.available() < 1){ 		    
		if( (millis() - _startTime) > 250) {
			 return 0; // timed out
		} 
	}
	return _gprs->_cell.available();
}

int GPRSClient::read(uint8_t *buf, size_t size)
{
	int i;
	uint8_t c;
	
	for(i=0;i<size;i++)
	{
		c=read();
		if(c==0)
			break;
		buf[i]=c;
	}
	
	return i;
}

int GPRSClient::read()
{
	return _gprs->_cell.read();;
}

int GPRSClient::peek()
{
	return _gprs->_cell.peek();;
}

void GPRSClient::flush()
{
	_gprs->_cell.flush();
}

GPRSClient::operator bool() {
  return true;
}

bool GPRSClient::operator==(const GPRSClient& rhs) {
  return true;
}