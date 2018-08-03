#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2018-08-03 17:05:21

#include "Arduino.h"
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

void os_getArtEui (u1_t* buf) ;
void os_getDevEui (u1_t* buf) ;
void os_getDevKey (u1_t* buf) ;
void onEvent (ev_t ev) ;
void wakeUp() ;
void do_send(osjob_t* j) ;
static void initfunc (osjob_t* j) ;
void setup() ;
void loop() ;

#include "LPWeather.ino"


#endif
