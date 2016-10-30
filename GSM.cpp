#include "GSM.h"
#include <string.h>
#include <avr/wdt.h>

GSM::GSM(){
	_cell = &Serial;
#ifdef DEBUG
	_debug = &Serial1;
#endif
}

GSM::GSM(Stream* serial){
	_cell = serial;
#ifdef DEBUG
	_debug = &Serial;
#endif
}

#ifdef DEBUG
GSM::GSM(Stream* serial, Stream* debug){
	_cell = serial;
	_debug = debug;
}
#endif

void GSM::setBuffer(char * buffer, int size) {
	_buffer = buffer;
	_bufferSize = size;
}

#ifdef DEBUG
char *GSM::setDebugSerial(Stream* debug){
	_debug = debug;
}
#endif

void GSM::send(char * str) {
	_cell->println(str);
}

#ifdef DEBUG
void GSM::print_P(const char *str)
{
  uint8_t val;
	const char *s = str;
  while (true) {
    val=pgm_read_byte(s);
    if (!val) {
			break;
		}
		_debug->print(val, HEX); _debug->write(' ');
    s++;
  }

	s=str;
  while (true) {
    val=pgm_read_byte(s);
    if (!val) break;
		if(val==13) {
			_debug->print(F("\\r"));
		} else 	if(val==10) {
			_debug->print(F("\\n"));
		} else{
			_debug->write(val);
		}
    s++;
  }
}

void GSM::hexPrint(const char *str) {
  uint8_t val;
	const char *s = str;
  while (true) {
    val=s[0];
    if (!val) break;
		_debug->print(val, HEX); _debug->print(' ');
    s++;
  }

	s=str;
  while (true) {
    val=s[0];
    if (!val) break;
		if(val==13) {
			_debug->print(F("\\r"));
		} else 	if(val==10) {
			_debug->print(F("\\n"));
		} else{
			_debug->write(val);
		}
    s++;
  }
}
#endif

/**
 * Parameters: 
 * expected - PSTR expected response str
 * readBeyond - number of bytes to continue to read after 'expected' is found or -1 to read all available data(basically continue till timeout)
 * response timeout in ms, returns if this time has passed since last received letter/character
 * Return true if expected is found otherwise false
 */
bool GSM::readAndCheckResponse(const char* expected, int readBeyond, int timeout) {
#ifdef DEBUG
  _debug->print(millis());
	_debug->print(F(" Check for '"));
	print_P(expected);
	if(readBeyond != -1) {
		_debug->print(F("' and read beyond "));
		_debug->println(readBeyond);
	} else {
		_debug->println(F("' and read all after."));
	}
#endif
	bool found = false;
	char *foundAt = NULL;
	long _endTime = millis()+timeout;		
	_bufferIndex = 0;
  _buffer[_bufferIndex] = 0; // end the string at start

	// keep reading while not timeout
	while(millis() < _endTime) {
		wdt_reset();
		if(_bufferIndex >= RESPONSE_BUFFER_SIZE-1){ //RESPONSE_BUFFER_SIZE-1) {
			// filled the buffer
			// first copy the same amount of data we are looking for to the beginning of buffer and then continue to fill up
			// since we have not found what we look for but partial bits might be already read
			
			uint8_t expected_len = strlen_P(expected);
#ifdef DEBUG
			_debug->print(millis());
			_debug->print(F(" Full("));
			_debug->print(_bufferIndex);
			_debug->print(F("):"));
			_debug->println(_buffer);
#endif
			if(NULL!=foundAt) { // expected is already found so copy from start of expected so we will keep any data after as well
				// copy from found to the end of buffer, place at begining
				_bufferIndex = strlen(foundAt); // lengt of data from found to end of buffer
				strlcpy(_buffer, foundAt, _bufferIndex); 
				if(_bufferIndex > RESPONSE_BUFFER_SIZE/2) { // We need to use memmove since the two regions overlaps
						memmove(_buffer, foundAt, _bufferIndex+1); // the length is one more to inlcude terminating 0
					} else {
						strcpy(_buffer, foundAt); // we alwasy 0 terminate strings so safe to use strcpy
					}
			} else { // else copy same amount as length of expected minus one since we might just be missing the last byte of expected
				if(expected_len - 1 > RESPONSE_BUFFER_SIZE/2) { // We need to use memmove since the two regions overlaps
						memmove(_buffer, &_buffer[_bufferIndex-(expected_len-1)], expected_len-1 +1); // the length is one more to inlcude terminating 0
					} else {
						strcpy(_buffer, &_buffer[_bufferIndex-(expected_len-1)]); // we alwasy 0 terminate strings so safe to use strcpy
					}
				_bufferIndex = expected_len-1;
			}
#ifdef DEBUG
			_debug->print(millis());
			_debug->print(F(" New:"));
			hexPrint(_buffer);
			_debug->println();
#endif
		}
		
		// check if new data available and if so store in buffer
		if(_cell->available()>0) {
			_buffer[_bufferIndex] = _cell->read();
			if(_buffer[_bufferIndex] != 0) {
				_buffer[++_bufferIndex] = 0; // put a zero to terminate string
			}
			_endTime = millis()+timeout; // shift end time forward
			// if not found before see if expected is in buffer
			if(!found) {
				foundAt = strstr_P(_buffer, expected);
				if (NULL != foundAt) {
					// expected found
					found = true;
#ifdef DEBUG
					_debug->print(millis());
					_debug->print(F(" Found at "));
					_debug->print(_bufferIndex);
					_debug->print(F(": will "));
					if(-1 == readBeyond) {
						// read all available bytes
						_debug->println(F("read all."));
					} else if(0 == readBeyond) {
						_debug->println(F("read no more."));
					} else {
						// read readBeyond number or bytes
						_debug->print(F("read "));
						_debug->print(readBeyond);
						_debug->println(F(" more bytes."));
					}
#endif
				}
			}
			
			// if found, decide if we should continue or not
			if(found) {
				if(readBeyond == 0) {
					// we have found expected and should not read more, break out
					break;
				} else if(readBeyond != -1) {
					// we have a number of bytes to continue to read so read one more and check again
					// if readBeyound is -1 we will just continue until we timeout
					readBeyond--;
				}
			} // if(found)
		} // if(_cell->available()>0)
	} // while

#ifdef DEBUG
	if(found) {
		_debug->print(millis());
		_debug->print(F(" Found '"));
		print_P(expected);
		_debug->print(F("' in '"));
		hexPrint(_buffer);
		_debug->println(F("'"));
	} else {
		_debug->print(millis());
		_debug->print(F(" Timeout after "));
		_debug->print(timeout);
		_debug->println(F("ms."));
		_debug->print(F("Did not find '"));
		print_P(expected);
		_debug->print(F("' in '"));
		hexPrint(_buffer);
		_debug->println(F("'"));
	}
#endif
	delay(5); // a slight delay without which communication will fail
	//Serial1.print(millis());Serial1.print('\'');hexPrint(_buffer); Serial1.println('\'');
	return found;
}

bool GSM::successfulResponse(int timeout) 
{
	return readAndCheckResponse(PSTR("OK\r\n"), 0, timeout);
}

NetworkStatus_t GSM::getNetworkStatus(void) {
	return _status;
}