//#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include "lmic.h"
#include <hal/hal.h>
#include <SPI.h>
#include <Arduino.h>
#include <DHT.h>;
#include "LowPower.h"
#include "BMP085.h"
#include <Wire.h>
#include <EEPROM.h>

//Constants

#define DHTPIN 10     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 22  (AM2302)
#define offsetEEPROM 0x0    //offset config
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
BMP085 myBarometer;

float temperature;
float pressure;

bool joined = false;

#define LedPin	13
#define sendPin	5
#define powerPin	12
#define VBATPIN A9
const int wakeUpPin = 1;
const int maxLoop = 4;

bool txComplete=false;
bool moveDetected = false;

#define LPP_TMP 103
#define LPP_HUM 104
#define LPP_BAR 115
#define LPP_DIG 0

// LoRaWAN NwkSKey, network session key
static const u1_t PROGMEM NWKSKEY[16] = { 0x3F, 0x79, 0xC4, 0xDB, 0xC4, 0xBF, 0xF3, 0xC6, 0x05, 0x01, 0xE6, 0x1F, 0xE0, 0x12, 0xF5, 0x6A };

// LoRaWAN AppSKey, application session key
static const u1_t PROGMEM APPSKEY[16] = { 0xD7, 0xAF, 0x9B, 0x3A, 0x08, 0xB9, 0xFC, 0xDB, 0x1A, 0x58, 0x45, 0x3D, 0x9A, 0xB8, 0x10, 0x48 };


// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = { 0x26011E7E };

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static void initfunc (osjob_t*);

int32_t lat;
int32_t lon;
int32_t alt=1;
bool fix=1;

static osjob_t sendjob;
static osjob_t initjob;

char receivedString[28];
char chkGS[3] = "GS";

struct StoreStruct {
	byte chkDigit;
	byte aprsChannel;
	byte rxChannel;
	byte txChannel;
	byte rxTone;
	byte txTone;
	char dest[8];
};

StoreStruct storage = {
		"#",
		64,
		140,
		92,
		8,
		8,
		"APZRAZ"
};

// Pin mapping is hardware specific.
// Pin mapping

const lmic_pinmap lmic_pins = {
		.nss = 8,
		.rxtx = LMIC_UNUSED_PIN,
		.rst = 4, // Needed on RFM92/RFM95? (probably not) D0/GPIO16
		.dio = {7, 6, LMIC_UNUSED_PIN}, // Specify pin numbers for DIO0, 1, 2
		// connected to D7, D6, -
};

void onEvent (ev_t ev) {
	if (ev == EV_TXCOMPLETE){
		Serial.println(F("TX Complete"));
		digitalWrite(sendPin, HIGH);
		txComplete = true;
		delay(50);
	}
}

void wakeUp()
{
	// Just a handler for the pin interrupt.
	moveDetected = true;
}

void do_send(osjob_t* j) {
	byte buffer[22];
	int32_t i;
	int32_t h;
	float measuredvbat = analogRead(VBATPIN);
	measuredvbat *= 2;    // we divided by 2, so multiply back
	measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
	measuredvbat /= 1024; // convert to voltage

	digitalWrite(powerPin, HIGH);

	float hum = dht.readHumidity();
	float temp= dht.readTemperature();

	temperature = myBarometer.bmp085GetTemperature(myBarometer.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
	pressure = myBarometer.bmp085GetPressure(myBarometer.bmp085ReadUP())/100;//Get the temperature

	digitalWrite(powerPin, LOW);
	delay(100);

	//Print temp and humidity values to serial monitor
	Serial.print("temp180: ");
	Serial.print(temperature);
	Serial.print(" Pressure: ");
	Serial.print(pressure);
	Serial.print(" Battery: ");
	Serial.print(measuredvbat);
	Serial.print(" Humidity: ");
	Serial.print(hum);
	Serial.print(" %, Temp: ");
	Serial.print(temp);
	Serial.println(" Celsius");

	i = 0;
	Serial.println(F("Process data"));

	h = measuredvbat * 100;
	buffer[i++] = 0x02;
	buffer[i++] = 0x02;
	buffer[i++] = (h >> 8) & 0xff;
	buffer[i++] = h & 0xff;

	h = temp * 10;

	buffer[i++] = 0x03;
	buffer[i++] = LPP_TMP;
	buffer[i++] = (h >> 8) & 0xff;
	buffer[i++] = h & 0xff;

	h = hum*2;

	buffer[i++] = 0x04;
	buffer[i++] = LPP_HUM;
	buffer[i++] = h & 0xff;

	h = temperature * 10;

	buffer[i++] = 0x05;
	buffer[i++] = LPP_TMP;
	buffer[i++] = (h >> 8) & 0xff;
	buffer[i++] = h & 0xff;

	h=pressure * 10;

	buffer[i++] = 0x06;
	buffer[i++] = LPP_BAR;
	buffer[i++] = (h >> 8) & 0xff;
	buffer[i++] = h & 0xff;

	buffer[i++] = 0x07;
	buffer[i++] = LPP_DIG;
	if (moveDetected==true) buffer[i++] = 1; else buffer[i++] = 0;

	if (LMIC.opmode & OP_TXRXPEND) {
		Serial.println(F("OP_TXRXPEND, not sending"));
	} else {
		// Prepare upstream data transmission at the next possible time.
		LMIC_setTxData2(1, (uint8_t*) buffer, i , 0);
		Serial.println(F("Sending"));
	}
}

// initial job
static void initfunc (osjob_t* j) {
	// reset MAC state
	LMIC_reset();
	LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
	// start joining
	// LMIC_startJoining();
	// init done - onEvent() callback will be invoked...
}

void forceTxSingleChannelDr(int channel) {
	for(int i=0; i<9; i++) { // For EU; for US use i<71
		if(i != channel) {
			LMIC_disableChannel(i);
		}
	}
	// Set data rate (SF) and transmit power for uplink
	LMIC_setDrTxpow(DR_SF9, 14);
}

void setup()
{
	// Disable GPS power
	pinMode(wakeUpPin, INPUT);
	pinMode(sendPin, OUTPUT);
	pinMode(powerPin, OUTPUT);
	digitalWrite(powerPin, HIGH);

	delay(5000);
	Serial.begin(115200);
	Serial.println(F("Starting"));

	if (EEPROM.read(offsetEEPROM) != storage.chkDigit){
		//if (EEPROM.read(offsetEEPROM) != storage.chkDigit){
		Serial.println(F("Writing defaults"));
		saveConfig();
	}

	loadConfig();
	printConfig();

	Serial.println(F("Type GS to enter setup:"));
	delay(5000);

	//if (Serial.available()>0) {
	//	Serial.println(F("Check for setup"));
		if (Serial.find(chkGS)) {
			Serial.println(F("Setup entered..."));
			setSettings(1);
			delay(2000);
		}
	//}

	delay(2000);

	myBarometer.init();

	// LMIC init
	os_init();

	// Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();

	// Set static session parameters. Instead of dynamically establishing a session
	// by joining the network, precomputed session parameters are be provided.
	uint8_t appskey[sizeof(APPSKEY)];
	uint8_t nwkskey[sizeof(NWKSKEY)];
	memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
	memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
	LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);

	// Select frequencies range
	//LMIC_selectSubBand(0);

	// Disable link check validation
	LMIC_setLinkCheckMode(1);

	// TTN uses SF9 for its RX2 window.
	LMIC.dn2Dr = DR_SF9;

	// Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
	LMIC_setDrTxpow(DR_SF9,14);
	//forceTxSingleChannelDr(1);

	digitalWrite(sendPin, LOW);
	digitalWrite(powerPin, LOW);

	//do_send(&sendjob);
}

int x=0;
void loop()
{
	digitalWrite(LedPin, HIGH);
	do_send(&sendjob);    // Send sensor values
	while (txComplete==false){
		os_runloop_once();
	}
	txComplete=false;
	delay(50);
	digitalWrite(LedPin, LOW);

	moveDetected = false;
	//attachInterrupt(3, wakeUp, HIGH);
	for(int x=0;x<maxLoop;x++){
		digitalWrite(LedPin, LOW);
		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
		digitalWrite(LedPin, HIGH);
		LowPower.powerDown(SLEEP_60MS, ADC_OFF, BOD_OFF);
		digitalWrite(LedPin, LOW);
		if (moveDetected==true) x = maxLoop;
	}
	//LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
	//detachInterrupt(3);
	digitalWrite(powerPin, HIGH);
	digitalWrite(LedPin, HIGH);
	delay(5000);
}

void saveConfig() {
	for (unsigned int t = 0; t < sizeof(storage); t++)
		EEPROM.write(offsetEEPROM + t, *((char*)&storage + t));
}

void loadConfig() {
	if (EEPROM.read(offsetEEPROM + 0) == storage.chkDigit)
		for (unsigned int t = 0; t < sizeof(storage); t++)
			*((char*)&storage + t) = EEPROM.read(offsetEEPROM + t);
}

void printConfig() {
	if (EEPROM.read(offsetEEPROM + 0) == storage.chkDigit)
		for (unsigned int t = 0; t < sizeof(storage); t++)
			Serial.write(EEPROM.read(offsetEEPROM + t));
	Serial.println();
	setSettings(0);
}

void setSettings(bool doSet) {
	int i = 0;
	byte b;
	receivedString[0] = 'X';

	Serial.print(F("APRS Channel ("));
	Serial.print(storage.aprsChannel);
	Serial.print(F("):"));
	if (doSet == 1) {
		i = getNumericValue();
		if (receivedString[0] != 0) storage.aprsChannel = i;
	}
	Serial.println();

	Serial.print(F("RX Channel ("));
	Serial.print(storage.rxChannel);
	Serial.print(F("):"));
	if (doSet == 1) {
		i = getNumericValue();
		if (receivedString[0] != 0) storage.rxChannel = i;
	}
	Serial.println();

	Serial.print(F("TX Channel ("));
	Serial.print(storage.txChannel);
	Serial.print(F("):"));
	if (doSet == 1) {
		i = getNumericValue();
		if (receivedString[0] != 0) storage.txChannel = i;
	}
	Serial.println();

	Serial.print(F("RX Tone ("));
	Serial.print(storage.rxTone);
	Serial.print(F("):"));
	if (doSet == 1) {
		i = getNumericValue();
		if (receivedString[0] != 0) storage.rxTone = i;
	}
	Serial.println();

	Serial.print(F("TX Tone ("));
	Serial.print(storage.txTone);
	Serial.print(F("):"));
	if (doSet == 1) {
		i = getNumericValue();
		if (receivedString[0] != 0) storage.txTone = i;
	}
	Serial.println();

	Serial.print(F("Dest. ("));
	Serial.print(storage.dest);
	Serial.print(F("):"));
	if (doSet == 1) {
		getStringValue(7,true);
		if (receivedString[0] != 0) {
			storage.dest[0] = 0;
			strcat(storage.dest, receivedString);
		}
	}
	Serial.println();

	if (doSet == 1) {
		saveConfig();
		loadConfig();
	}
}

void getStringValue(int length, bool fixedLen) {
	serialFlush();
	receivedString[0] = 0;
	int i = 0;
	while (receivedString[i] != 13 && i < length) {
		if (Serial.available() > 0) {
			receivedString[i] = Serial.read();
			if (receivedString[i] == 13 || receivedString[i] == 10) {
				i--;
			}
			else {
				Serial.write(receivedString[i]);
			}
			i++;
		}
	}
	if (fixedLen==true && i>0 ){
		while (i<length-1){
			receivedString[i]=' ';
			i++;
		}
	}
	receivedString[i] = 0;
	serialFlush();
}

byte getCharValue() {
	serialFlush();
	receivedString[0] = 0;
	int i = 0;
	while (receivedString[i] != 13 && i < 2) {
		if (Serial.available() > 0) {
			receivedString[i] = Serial.read();
			if (receivedString[i] == 13 || receivedString[i] == 10) {
				i--;
			}
			else {
				Serial.write(receivedString[i]);
			}
			i++;
		}
	}
	receivedString[i] = 0;
	serialFlush();
	return receivedString[i - 1];
}

byte getNumericValue() {
	serialFlush();
	byte myByte = 0;
	byte inChar = 0;
	bool isNegative = false;
	receivedString[0] = 0;

	int i = 0;
	while (inChar != 13) {
		if (Serial.available() > 0) {
			inChar = Serial.read();
			if (inChar > 47 && inChar < 58) {
				receivedString[i] = inChar;
				i++;
				Serial.write(inChar);
				myByte = (myByte * 10) + (inChar - 48);
			}
			if (inChar == 45) {
				Serial.write(inChar);
				isNegative = true;
			}
		}
	}
	receivedString[i] = 0;
	if (isNegative == true) myByte = myByte * -1;
	serialFlush();
	return myByte;
}

void serialFlush() {
	for (int i = 0; i < 10; i++)
	{
		while (Serial.available() > 0) {
			Serial.read();
		}
	}
}


