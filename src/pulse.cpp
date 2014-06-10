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

// ===================
// = Pin Definitions =
// ===================

uint8_t leftLedPin = PB2;
uint8_t leftSensorPin = PA1;
uint8_t leftRealLedPin = 5;
uint8_t serialPin = 7;
int16_t ledBrightness = 0;

// ===========
// = Globals =
// ===========

long timer = 0;
long loops = 0;
uint16_t smoothedVoltages[filterSamples];

Sampler sampler = Sampler();
SoftwareSerial mySerial(0, serialPin); // RX, TX

typedef enum
{
    STATE_RESTING = 0,
    STATE_ON = 1,
    STATE_LED_RISING = 2,
    STATE_LED_FALLING = 3
} state_t;
state_t appState;

// ============
// = Routines =
// ============

void setup(){
    // analogReference(EXTERNAL);
    analogReference(DEFAULT);
    pinMode(leftLedPin, OUTPUT);
    pinMode(leftRealLedPin, OUTPUT);
    analogWrite(leftLedPin, ledBrightness);
    appState = STATE_RESTING;
    sampler.clear();
    mySerial.begin(9600);
    printHeader();
}

void loop() {
    // printHeader();
    uint16_t leftSensorVoltage = 0;
    uint16_t smoothedLeftVoltage = 0;
    int peaked = 0;

    if (appState == STATE_RESTING || appState == STATE_ON) {
        leftSensorVoltage = readSensorValues(leftSensorPin);
        smoothedLeftVoltage = digitalSmooth(leftSensorVoltage, smoothedVoltages, mySerial);
        sampler.add(smoothedLeftVoltage);
        peaked = sampler.isPeaked(mySerial);
        mySerial.print(peaked < 0 ? "---" : peaked > 0 ? "***" : "...");
        digitalWrite(leftRealLedPin, peaked > 0 ? HIGH : LOW);
    }
    
    if (peaked != 0 || appState == STATE_LED_RISING) {
        appState = STATE_LED_RISING;
        ledBrightness = min(ledBrightness + 2, 255);
        analogWrite(leftLedPin, ledBrightness);
        delay(1);
        if (ledBrightness >= 255) {
            appState = STATE_ON;
        }
    } else if ((peaked == 0 && appState == STATE_ON) || 
               appState == STATE_LED_FALLING) {
        appState = STATE_LED_FALLING;
        ledBrightness = max(0, ledBrightness - 1);
        analogWrite(leftLedPin, ledBrightness);
        delay(1);
        if (ledBrightness <= 0) {
            appState = STATE_RESTING;
        }
    }
    
    if (leftSensorVoltage == 0 ||
        appState == STATE_LED_RISING || 
        appState == STATE_LED_FALLING) {
        return;
    }
    
    mySerial.print(leftSensorVoltage, DEC);
    mySerial.print("   smooth=");
    mySerial.print(smoothedLeftVoltage, DEC);    
    mySerial.print("   p50=");
    mySerial.print(sampler.getPercentile(.5), DEC);
    mySerial.print("   p70=");
    mySerial.print(sampler.getPercentile(.7), DEC);
    mySerial.print("   period=");
    mySerial.print(sampler.getPeriod(), DEC);
    mySerial.print(peaked ? " *** " : " ");
    mySerial.print("   state=");
    mySerial.print(appState, DEC);
    mySerial.print("   brightness=");
    mySerial.println(ledBrightness, DEC);
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