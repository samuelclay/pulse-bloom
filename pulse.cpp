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
int leftSensor = PINA4;
int rightSensor = PINA5;

long timer = 0;
int loops = 0;

Sampler sampler;
SoftwareSerial mySerial(2, 8); // RX, TX

void checkPulse();

void setup(){
    analogReference(EXTERNAL);
    pinMode(leftLed, OUTPUT);
    pinMode(rightLed, OUTPUT);
    sampler.clear();
    mySerial.begin(9600);
}

void loop(){
    checkPulse();
}

void checkPulse() {
    int leftSensorOn = analogRead(leftSensor);
    int rightSensorOn = analogRead(rightSensor);
    
    digitalWrite(leftLed, leftSensorOn > 200);
    digitalWrite(rightLed, rightSensorOn > 200);
    
    sampler.add(leftSensorOn);
    mySerial.print(leftSensorOn);
    mySerial.println("mV");
    if (millis() - timer > 1000) {
        mySerial.print("------------------ ");
        mySerial.print(loops);
        mySerial.println(" ------------------");
        timer = millis();
        loops += 1;
    }
}