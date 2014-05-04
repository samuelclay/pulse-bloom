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

#include "pulse.h"
#include "sampler.h"
#include "smooth.h"

int leftLedPin = 7;
int rightLedPin = 9;
int leftSensorPin = PINA4;
int rightSensorPin = PINA5;

long timer = 0;
int loops = 0;
int smoothedVoltages[filterSamples];

Sampler sampler = Sampler();
SoftwareSerial mySerial(2, 8); // RX, TX


void setup(){
    analogReference(EXTERNAL);
    pinMode(leftLedPin, OUTPUT);
    pinMode(rightLedPin, OUTPUT);
    sampler.clear();
    mySerial.begin(9600);
}

void loop() {
    printHeader();
    
    int leftSensorVoltage = readSensorValues(leftSensorPin);
    int smoothedLeftVoltage = digitalSmooth(leftSensorVoltage, smoothedVoltages);

    mySerial.print(leftSensorVoltage);
    mySerial.print("   ");
    mySerial.print(smoothedLeftVoltage);
    
    sampler.add(smoothedLeftVoltage);
    bool peaked = sampler.isPeaked();
    digitalWrite(leftLedPin, peaked ? HIGH : LOW);
    mySerial.print("   ");
    mySerial.print(sampler.getPercentile(.9));
    mySerial.println(peaked ? " ***" : "");
}

int readSensorValues(int sensorPin) {
    int sensorVoltage = analogRead(sensorPin);
    
    return sensorVoltage;
}

void printHeader() {
    if (millis() - timer > 1000) {
        mySerial.print("------------------ ");
        mySerial.print(loops);
        mySerial.println(" ------------------");
        timer = millis();
        loops += 1;
    }
}