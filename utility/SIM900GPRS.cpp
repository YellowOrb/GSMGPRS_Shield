#include "SIM900GPRS.h"

const char ip_initial[] PROGMEM = "IP INITIAL";
const char ip_start[] PROGMEM = "IP START";
const char ip_config[] PROGMEM = "IP CONFIG";
const char ip_gprsact[] PROGMEM = "IP GPRSACT";
const char ip_status[] PROGMEM = "IP STATUS";
const char tcp_connecting[] PROGMEM = "TCP CONNECTING";
const char connect_ok[] PROGMEM = "CONNECT OK";
const char tcp_closing[] PROGMEM = "TCP CLOSING";
const char tcp_closed[] PROGMEM = "TCP CLOSED";
const char pdp_deact[] PROGMEM = "PDP DEACT";

const char * const connectionStatusStrings[] PROGMEM =
{   
		ip_initial,
		ip_start,
		ip_config, 
		ip_gprsact,
		ip_status, 
		tcp_connecting,
		connect_ok,
		tcp_closing,
		tcp_closed,    
		pdp_deact 
};

SIM900GPRS::SIM900GPRS():GPRS(){}
SIM900GPRS::SIM900GPRS(Stream *serial):GPRS(serial){}
#ifdef DEBUG
SIM900GPRS::SIM900GPRS(Stream *serial, Stream *debug):GPRS(serial, debug){}
#endif

void powerOnCycle() {
	pinMode(GSM_ON_PIN, OUTPUT); 
	digitalWrite(GSM_ON_PIN,LOW);
	delay(100);
	digitalWrite(GSM_ON_PIN,HIGH);
	delay(500); //2500 
	digitalWrite(GSM_ON_PIN,LOW);
	delay(100); // 3500
}

bool SIM900GPRS::turnOn(){
	// check if already on, if so return true
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT"));
#endif
	_cell->println(F("AT"));
	if(successfulResponse(100)){
#ifdef DEBUG
		_debug->println(F("Already on"));
#endif
		return true;
	}
	
	powerOnCycle();
	
	// try to send AT and get OK back
	int i = 0;
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT"));
#endif
	_cell->println(F("AT"));
	while(!successfulResponse(250) && (i < 10)) {
		i++;
		_cell->println(F("AT"));
	}
	if(i == 10) { // did not get response after 10 AT commands, could not turn on
		_status = ERROR;
		return false;
	}
	_status = CONNECTING;
	return true;
}

/**
 * Connects to the GSM network identified on the SIM card.
Syntax

gsm.begin()
gsm.begin(pin)
gsm.begin(pin, restart)
gsm.begin(pin, restart, sync)
Parameters

pin : character array with the PIN to access a SIM card (default = 0)
restart : boolean, determines whether to restart modem or not (default= true)
sync : boolean, synchronous (true, default) or asynchronous (false) mode
Returns

char : 0 if asynchronous. If synchronous, returns status : ERROR, IDLE, CONNECTING, GSM_READY, GPRS_READY, TRANSPARENT_CONNECTED
 */
// TODO - Implement not supported features
NetworkStatus_t SIM900GPRS::begin(char* pin, bool restart)
{
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" ATE0"));
#endif
	_cell->println(F("ATE0"));
	if(!successfulResponse()) {
		return ERROR;
	} 

#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" ATV1"));
#endif
	_cell->println(F("ATV1"));
	if(!successfulResponse()) { //set verbose mode
		return ERROR;
	}	
	
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+IPR=0"));
#endif
	_cell->println(F("AT+IPR=0"));
	if(!successfulResponse()) { //set autoBaud (default not really needed)
		return ERROR;
	}

#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CMEE=1"));
#endif	
	_cell->println(F("AT+CMEE=1"));
	if(!successfulResponse()) { //set extended error report
		// CMEE=0 just ERROR
		// CMEE=1 just error number
		// CMEE=2 full error
		return ERROR;	
	}
	
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CREG=0"));
#endif	
	_cell->println(F("AT+CREG=0")); // no presentation of unsolicited result code +CREG
	if(!successfulResponse()) { 
		return ERROR;	
	}
//	_cell->println(F("AT+IFC=1,1"));
//	if(!successfulResponse()) { //set software flow control
//		return ERROR;	
//	}
	
	return GSM_READY;
}

/**
 * Obtain modem IMEI (command AT)
 * Parameters - imei string to put imei number into, length - length of imei string 
 * Returns - string with modem IMEI number or NULL is failed
 */
char* SIM900GPRS::getIMEI(char* imei, int length)
{
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+GSN"));
#endif
	_cell->println(F("AT+GSN")); //RETURNS: 0000646714 OK
	if(!successfulResponse()) { // no response
		return NULL;
	}
	char* end = strstr_P(_buffer, PSTR("OK"));
	end -=3; // end points att O in OK and is preceeded with \r\n
	end[0] = 0;
	strncpy(imei, _buffer+2, length); // the response we get starts with \r\n, skip those
	return imei; 
}

bool SIM900GPRS::isGPRSAvailable() {
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CGATT?"));
#endif
	_cell->println(F("AT+CGATT?")); // check if GPRS is activated
	// RETURNS: +CGATT: 1  OK
	if(!successfulResponse(3000)) { // no response
		return NULL;
	}
	char* end = strstr_P(_buffer, PSTR("OK"));
	end -=1;
	end[0] = 0;
	char* gprs = strchr(_buffer,':');
	if(gprs[2]=='1') {
		return true;
	} else {
		return false;
	}
}

NetworkStatus_t SIM900GPRS::attachGPRS(const char * const domain, const char * const username, const char * const password) {
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CREG?"));
#endif
	_cell->println(F("AT+CREG?")); // check if registered in network
	// RETURNS: +CREG: 1,x  OK
	if(!successfulResponse()) { // no response
		_status = ERROR;
		return _status;
	}
	char* end = strstr_P(_buffer, PSTR("OK"));
	end -=1;
	end[0] = 0;
	char* registered = strstr_P(_buffer, PSTR("+CREG: 0,"));
	if(registered[9] !='1') {
		_status = CONNECTING;
		return _status;
	}
	
	delay(15); // short delay needed for modem to detect next command
	// we only want a single IP Connection at a time.
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CIPMUX=0"));
#endif
	_cell->println(F("AT+CIPMUX=0"));
	if(!successfulResponse()) {
		_status = CONNECTING;
		return _status;
	}

	// we want non-transparent mode
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CIPMODE=0"));
#endif
	_cell->println(F("AT+CIPMODE=0"));
	if(!successfulResponse()) {
		_status = CONNECTING;
		return _status;
	}
	
	// start GPRS task and set APN, username and password
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CSTT="));
#endif
	_cell->print(F("AT+CSTT=\""));
	_cell->print(domain);
	_cell->print(F("\",\""));
	if(NULL != username) _cell->print(username);
	_cell->print(F("\",\""));
	if(NULL != password) _cell->print(password);
	_cell->println(F("\""));
	if(!successfulResponse()) {
		_status = CONNECTING;
		return _status;
	}
	
	// chip is in state IP START
	
	// bring up wireless connection with GPRS or CSD
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CIICR"));
#endif
	_cell->println(F("AT+CIICR"));
	if(!successfulResponse(15000)) {
		_status = CONNECTING;
		return _status;
	}
	
	// chip is in state IP CONFIG or IP GPRSACT
	_status = GPRS_READY;
	return GPRS_READY;
}

void SIM900GPRS::detachGPRS() {
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CIPSHUT"));
#endif
	_cell->println(F("AT+CIPSHUT")); // deactivate GPRS PDP context
	if(!readAndCheckResponse(PSTR("SHUT OK"), 0)) { // no response
		return;
	}
	_status = IDLE;
}

/**
 * Gets signal quality, returns the rssi value
 * 	0 		-115 dBm or less
 * 	1  		-111 dBm
 * 	2..30 -110..-54 dBm
 * 	31		-52 dBm or greater
 * 	99		not known or detectable
 */
int SIM900GPRS::getSignalStrength(){
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CSQ"));
#endif
	_cell->println(F("AT+CSQ")); 
	//RETURNS: +CSQ: 7,0 OK
	/*
	 +CSQ: <rssi>, <ber>
		<rssi>
			0 		-115 dBm or less
			1  		-111 dBm
			2..30 -110..-54 dBm
			31		-52 dBm or greater
			99		not known or detectable
		<ber> (in percent)
			0..7 	as RXQUAL values in the table GSM 05-08 [20] subclause 7.2.4
			99 		not knonw or detectable
	*/
	if(!successfulResponse()) { // no response
		return 99;
	}

	char* end = strstr_P(_buffer, PSTR("OK"));
	if(NULL == end ) {
		return 99;
	}
	end -=1;
	end[0] = 0;
	char* gprs = strchr(_buffer,':');
	return atoi(gprs);
}

/**
 * Disconnects from the GSM network identified on the SIM card by powering the modem off.
 * Syntax - gsm.shutdown()
 * Returns - boolean : true when successful
 */
bool SIM900GPRS::shutdown(){
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CPOWD=1"));
#endif
	_cell->println(F("AT+CPOWD=1"));
	if(readAndCheckResponse(PSTR("NORMAL POWER DOWN"), -1, 1000)) {
		_status = OFF;
		return true;
	} else {
		// force off
#ifdef DEBUG
		_debug->println(F("power on"));
#endif
		powerOnCycle();
		
#ifdef DEBUG
		_debug->println(F("power off"));
#endif
		digitalWrite(GSM_ON_PIN,LOW);
		delay(100);
		digitalWrite(GSM_ON_PIN,HIGH);
		delay(1500); //2500 
		digitalWrite(GSM_ON_PIN,LOW);
		delay(100); // 3500
		
#ifdef DEBUG
			_debug->println(F("check off"));
#endif
		if(readAndCheckResponse(PSTR("NORMAL POWER DOWN"),-1, 500)) {
			_status = OFF;
			return true;
		} else {
			return false;
		}
	}
}

ConnectionStatus_t SIM900GPRS::parseConnectionStatus(char * str)
{
#ifdef DEBUG
	char outputStr[40];
	_debug->print(millis()); _debug->print(F(" Parse status: ")); _debug->println(str);
#endif
	int i = 0;
	int max = sizeof(connectionStatusStrings)/sizeof(*connectionStatusStrings);
#ifdef DEBUG
	_debug->print(millis()); _debug->print(F(" max: ")); _debug->println(max);
#endif
#ifdef DEBUG
	strcpy_P(outputStr, (char*)pgm_read_word(&(connectionStatusStrings[i])));
	_debug->print(millis()); _debug->print(F(" check for: ")); _debug->println(outputStr);
#endif	
	while( (NULL == strstr_P(str, (char*)pgm_read_word(&(connectionStatusStrings[i])))) && (i<max) ) {
		i++;
#ifdef DEBUG
		strcpy_P(outputStr, (char*)pgm_read_word(&(connectionStatusStrings[i])));
		_debug->print(millis()); _debug->print(F(" check for: ")); _debug->println(outputStr);
#endif
	}
	return (ConnectionStatus_t)i;
}

ConnectionStatus_t SIM900GPRS::getConnectionStatus()
{
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CIPSTATUS"));
#endif
	_cell->println(F("AT+CIPSTATUS")); 
	if(!readAndCheckResponse(PSTR("OK\r\n"),0)) { // check for the OK
#ifdef DEBUG
		_debug->print(millis()); _debug->println(F(" No OK response"));
#endif		
		return UNKNOWN_GSM_STATUS;
	}
	if(!readAndCheckResponse(PSTR("\r\n"),0)) { // read empty line
#ifdef DEBUG
		_debug->print(millis()); _debug->println(F(" No empty line"));
#endif
			return UNKNOWN_GSM_STATUS;
	}
	if(!readAndCheckResponse(PSTR("\r\n"),0)) { // read the STATE: line
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" No STATE"));
#endif
		return UNKNOWN_GSM_STATUS;
	}
	return parseConnectionStatus(_buffer);
}

char* SIM900GPRS::getIP(char* ip, int length)
{
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CIFSR"));
#endif
	_cell->println(F("AT+CIFSR"));
	readAndCheckResponse(PSTR("\r\n"),0); // just skip the first \r\n
  readAndCheckResponse(PSTR("\r\n"),0); // just read all available bytes
	char* end = strchr(_buffer+2,'\r'); // skip ending \r\n
	end[0] = 0;
	strncpy(ip, _buffer, length);
	return ip;
}

/**
 * 
 * Parameters - 
 * Returns - 
 */
bool SIM900GPRS::activateDateTime()
{
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CLTS=1"));
#endif
	_cell->println(F("AT+CLTS=1")); 
	return successfulResponse();
}

/**
 * 
 * Parameters - 
 * Returns - 
 */
bool SIM900GPRS::deactivateDateTime()
{
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CLTS=0"));
#endif
	_cell->println(F("AT+CLTS=0")); 
	return successfulResponse();
}

/**
 * 
 * Parameters - 
 * Returns - 
 */
char* SIM900GPRS::getDateTime()
{
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CCLK?"));
#endif
	_cell->println(F("AT+CCLK?")); // RETURNS: +CCLK: "15/03/26,17:25:28+04"
	if(!successfulResponse()) { // no response
		return NULL;
	}
	char* end = strstr_P(_buffer, PSTR("OK"));
	end -=8; // end points att O in OK and is preceeded with two \r\n and a +zz"
	end[0] = 0;
	return &_buffer[10]; // start from character after first "
}

/**
 * 
 * Parameters - 
 * Returns - a string with current GMT time
 */
time_t SIM900GPRS::getDateTime2()
{
#ifdef DEBUG
	_debug->print(millis()); _debug->println(F(" AT+CCLK?"));
#endif
	_cell->println(F("AT+CCLK?")); // RETURNS: +CCLK: "15/03/26,17:25:28+04"\r\n\r\nOK\r\n
	if(!successfulResponse()) { // no response
		return NULL;
	}
	_buffer += 10; // start from character after first "
	char* end = strstr_P(_buffer, PSTR("OK"));
	end -=5; // end points at O in OK and is preceeded with two \r\n and an "
	end[0] = 0;
	
	Serial1.print(F("Got time: ")); Serial1.println(_buffer);
	int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
	int tz;
  
  _buffer[2]=0;
  year = atoi(_buffer);
  _buffer += 3;
  
  _buffer[2]=0;
  month = atoi(_buffer);
  _buffer += 3;
  
  _buffer[2]=0;
  day = atoi(_buffer);
  _buffer += 3;
  
  _buffer[2]=0;
  hour = atoi(_buffer);
  _buffer += 3;
  
  _buffer[2]=0;
  min = atoi(_buffer);
  _buffer += 3;
  


	tz = atoi(&_buffer[3]); // parse including the + or - in the timezone
	
	end -=3; // remove the a +zz
	end[0] = 0;
  sec = atoi(_buffer);

	tmElements_t tm;
	
	year += 30; // years since 1970  
	tm.Year = year;
	tm.Month = month;
	tm.Day = day;
	tm.Hour = hour;
	tm.Minute = min;
	tm.Second = sec;
 
	// compensate for time zone so the result will be in GMT
	// tz indicates the difference, expressed in quarters of an hour, between the loacl time and GMT; range -47 ... +48
	return makeTime(tm) - (tz * 15 * 60); 
}


int SIM900GPRS::getSIMStatus(void) {return 0;}

int SIM900GPRS::sendSMS(char* number, char* data) {return 0;}
int SIM900GPRS::readSMS(int messageIndex, char *message, int length) {return 0;}
int SIM900GPRS::deleteSMS(int index) {return 0;}

int SIM900GPRS::callUp(char* number) {return 0;}
int SIM900GPRS::answer(void) {return 0;}
char SIM900GPRS::getPhoneNumber(byte position, char *phone_number) {return 0;}
char SIM900GPRS::writePhoneNumber(byte position, char *phone_number) {return 0;}
char SIM900GPRS::deletePhoneNumber(byte position) {return 0;}