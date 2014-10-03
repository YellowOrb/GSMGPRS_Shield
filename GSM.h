#ifndef gsm_h
#define gsm_h

class GSM {

public:
	GSM();

  virtualint init(void);
	virtual int begin(long baud_rate); 
	
	virtual int readIMEI(char* imei);
  virtual int getSIMStatus(void);
 	virtual int getNetworkStatus(void);
	// returns registration state
	byte IsRegistered(void);
	// returns whether complete initialization was made
	byte IsInitialized(void);


  virtual int sendSMS(char* number, char* data);
  virtual int readSMS(int messageIndex, char *message, int length);
  virtual int deleteSMS(int index);

  virtual int callUp(char* number);
  virtual int answer(void);
	virtual char getPhoneNumber(byte position, char *phone_number);
	virtual char writePhoneNumber(byte position, char *phone_number);
	virtual char deletePhoneNumber(byte position);



};

#endif