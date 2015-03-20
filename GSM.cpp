#include "GSM.h"
#include <string.h>

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
 * response timeout in ms
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
	long _endTime = millis()+timeout;		
	_bufferIndex = 0;
  _buffer[_bufferIndex] = 0; // end the string at start

	// keep reading while not timeout
	while(millis() < _endTime) {
		if(_bufferIndex == RESPONSE_BUFFER_SIZE-1 && readBeyond == -1) {
			// filled the buffer AND read all available data, lets continue
			// first copy the same amount of data we are looking for to the beginning of buffer and then continue to fill up
			// since we have not found what we look for but partial bits might be already read
			uint8_t expected_len = strlen_P(expected);
			strlcpy(_buffer, &_buffer[_bufferIndex-expected_len], expected_len);
			_bufferIndex = expected_len;
			_buffer[_bufferIndex] = 0; // put a zero to indicate empty or last position in string
		}
		
		// check if new data available and if so store in buffer
		if(_cell->available()>0) {
			_buffer[_bufferIndex++] = _cell->read();
			_buffer[_bufferIndex] = 0; // put a zero to indicate empty or last position in string
			
			// if not found before see if expected is in buffer
			if(!found && (NULL != strstr_P(_buffer, expected))) {
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
//	Serial1.print('\'');Serial1.print(_buffer); Serial1.println('\'');
	return found;
}

bool GSM::successfulResponse(int timeout) 
{
	return readAndCheckResponse(PSTR("OK\r\n"), 0, timeout);
}

NetworkStatus_t GSM::getNetworkStatus(void) {
	return _status;
}