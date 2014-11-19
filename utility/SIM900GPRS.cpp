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

SIM900GPRS::SIM900GPRS(long baudrate, int gprsBoardRXPin, int gprsBoardTXPin) {
	_cell = SoftwareSerial(gprsBoardRXPin, gprsBoardTXPin);
	_cell.begin(baudrate);
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
	_cell.println(F("ATE0"));
	if(!successfulResponse()) {
		return ERROR;
	} 

	_cell.println(F("ATV1"));
	if(!successfulResponse()) { //set verbose mode
		return ERROR;
	}	
	
	_cell.println(F("AT+IPR=0"));
	if(!successfulResponse()) { //set autoBaud (default not really needed)
		return ERROR;
	}
	
	_cell.println(F("AT+CMEE=1"));
	if(!successfulResponse()) { //set extended error report
		// CMEE=0 just ERROR
		// CMEE=1 just error number
		// CMEE=2 full error
		return ERROR;	
	}
	
	return GSM_READY;
}

/**
 * Obtain modem IMEI (command AT)
 * Parameters - imei string to put imei number into, length - length of imei string 
 * Returns - string with modem IMEI number or NULL is failed
 */
char* SIM900GPRS::getIMEI(char* imei, int length)
{
	_cell.println(F("AT+GSN")); //RETURNS: 0000646714 OK
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
	_cell.println(F("AT+CGATT?")); // check if GPRS is activated
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
	// close down any left GPRS connections or similar for comming commands to work, brings chip back to state IP INITIAL
	detachGPRS();
	
	// we only want a single IP Connection at a time.
	_cell.println(F("AT+CIPMUX=0"));
	if(!successfulResponse()) {
		_status = GPRS_FAILED_SINGLE_CONNECT;
		return _status;
	}

	// we want non-transparent mode
	_cell.println(F("AT+CIPMODE=0"));
	if(!successfulResponse()) {
		_status = GPRS_NO_TRANSPARENT_MODE;
		return _status;
	}
	
	// start GPRS task and set APN, username and password
	_cell.print(F("AT+CSTT=\""));
	_cell.print(domain);
	_cell.print(F("\",\""));
	if(NULL != username) _cell.print(username);
	_cell.print(F("\",\""));
	if(NULL != password) _cell.print(password);
	_cell.println(F("\""));
	if(!successfulResponse()) {
		_status = GPRS_FAILED_START_GPRS; // we didnt get an OK back
		return _status;
	}
	
	// chip is in state IP START
	
	// bring up wireless connection with GPRS or CSD
	_cell.println(F("AT+CIICR"));
	if(!successfulResponse(5000)) {
		_status = GPRS_FAILED_GPRS_CONNECT;
		return _status;
	}
	
	// chip is in state IP CONFIG or IP GPRSACT
	_status = GPRS_READY;
	return GPRS_READY;
}

void SIM900GPRS::detachGPRS() {
	_cell.println(F("AT+CIPSHUT")); // deactivate GPRS PDP context
	if(!readAndCheckResponse(PSTR("SHUT OK"))) { // no response
		return;
	}
	_status = IDLE;
}

void SIM900GPRS::activateGPRS() {
	_cell.println(F("AT+CGATT=1")); // activate GPRS Service
	if(!successfulResponse()) {
		return;
	}
	_status = GPRS_ACTIVE;
}

void SIM900GPRS::deactivateGPRS() {
	_cell.println(F("AT+CGATT=0")); // deactivate GPRS Service
	if(!successfulResponse()) {
	}
	_status = GPRS_DEACTIVATED;
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
	_cell.println(F("AT+CSQ")); 
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
		return false;
	}
	end -=1;
	end[0] = 0;
	char* gprs = strchr(_buffer,':');
	return atoi(gprs);
}

void cyclePowerOnOff() {
	pinMode(GSM_ON_PIN, OUTPUT); 
	digitalWrite(GSM_ON_PIN,LOW);
	delay(1000);
	digitalWrite(GSM_ON_PIN,HIGH);
	delay(2000); //2500 
	digitalWrite(GSM_ON_PIN,LOW);
	delay(3000); // 3500
}

bool SIM900GPRS::turnOn(){
	// check if already on, if so return true
	_cell.println(F("AT+CREG?"));
	if(successfulResponse()){
		return true;
	}
	
	cyclePowerOnOff();
	
	// try to send AT and get OK back
/*	int i = 0;
	_cell.println(F("AT"));
	while(!successfulResponse() && (i < 5)) {
		delay(500);
		i++;
		_cell.println(F("AT"));
	}
	*/
	
	_cell.println(F("AT"));
	// if we cannot talk to the modem, try to power cycle and talk again
	if(!successfulResponse()) {
		cyclePowerOnOff();
		_cell.println(F("AT"));
		// we cannot talk to the modem, something is really wrong
		if(!successfulResponse()) {
			_status = ERROR;
			return false;
		}
	}
	_status = CONNECTING;
	return true;
}


/**
 * Disconnects from the GSM network identified on the SIM card by powering the modem off.
 * Syntax - gsm.shutdown()
 * Returns - boolean : true when successful
 */
bool SIM900GPRS::shutdown(){
	_cell.println(F("AT+CPOWD=1"));
	if(readAndCheckResponse(PSTR("NORMAL POWER DOWN"))) {
		_status = DEVICE_OFF;
		return true;
	} else {
		return false;
	}
}

ConnectionStatus_t SIM900GPRS::parseConnectionStatus(char * str)
{
	int i = 0;
	int max = sizeof(connectionStatusStrings)/sizeof(*connectionStatusStrings);
	char * ptr = (char *) pgm_read_word (&connectionStatusStrings[i]);
	while( (NULL == strstr_P(str, ptr)) && (i<max) ) {
		i++;
		ptr = (char *) pgm_read_word (&connectionStatusStrings[i]);
	}
	return (ConnectionStatus_t)i;
}

ConnectionStatus_t SIM900GPRS::getConnectionStatus()
{
	_cell.println(F("AT+CIPSTATUS")); 
	if(!successfulResponse()) {
		return UNKNOWN_GSM_STATUS;
	}
	return parseConnectionStatus(_buffer);
}

char* SIM900GPRS::getIP(char* ip, int length)
{
	_cell.println(F("AT+CIFSR"));
	readAndCheckResponse(PSTR("\r")); // just read all available bytes
	char* end = strchr(_buffer+2,'\r'); // skip ending \r\n
	end[0] = 0;
	strncpy(ip, _buffer+2, length); // the response we get starts with \r\n, skip those
	return ip;
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