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

uint8_t leftLedPin = 7;
uint8_t rightLedPin = 9;
uint8_t leftSensorPin = PINA4;
uint8_t rightSensorPin = PINA5;

long timer = 0;
long loops = 0;
uint16_t smoothedVoltages[filterSamples];

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
    
    uint16_t leftSensorVoltage = readSensorValues(leftSensorPin);
    uint16_t smoothedLeftVoltage = digitalSmooth(leftSensorVoltage, smoothedVoltages);

    mySerial.print(leftSensorVoltage, DEC);
    mySerial.print("   ");
    mySerial.print(smoothedLeftVoltage, DEC);
    
    sampler.add(smoothedLeftVoltage);
    bool peaked = sampler.isPeaked();
    digitalWrite(leftLedPin, peaked ? HIGH : LOW);
    mySerial.print("   ");
    mySerial.print(sampler.getPercentile(.9), DEC);
    mySerial.print("   ");
    mySerial.print(sampler.getPeriod(), DEC);
    mySerial.println(peaked ? " ***" : "");
}

uint16_t readSensorValues(uint8_t sensorPin) {
    uint16_t sensorVoltage = analogRead(sensorPin);
    
    return sensorVoltage;
}

void printHeader() {
    if (millis() - timer > 1000) {
        sampler.calculateStats();
        mySerial.print("------------------ ");
        mySerial.print(loops);
        mySerial.println(" ------------------");
        timer = millis();
        loops += 1;
    }
}