#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <tiny/wiring.h>
#include <SoftwareSerial/SoftwareSerial.h>

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>
#include <avr/pgmspace.h>   /* required by usbdrv.h */

#include "sampler.h"

int leftLed = 7;
int rightLed = 9;
int leftSensor = 6;
int rightSensor = 5;

Sampler sampler;
void checkPulse();

void setup(){
    analogReference(EXTERNAL);
    pinMode(leftLed, OUTPUT);
    pinMode(rightLed, OUTPUT);
    sampler.clear();
}

void loop(){
    checkPulse();
}

void checkPulse() {
    int leftSensorOn = analogRead(leftSensor);
    int rightSensorOn = analogRead(rightSensor);
    
    digitalWrite(leftLed, leftSensorOn > 300);
    digitalWrite(rightLed, rightSensorOn > 400);
    
    sampler.add(leftSensorOn);
}