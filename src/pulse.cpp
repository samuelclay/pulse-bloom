#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <tiny/wiring.h>
#include <SoftwareSerial/SoftwareSerial.h>
#include <neopixel/Adafruit_NeoPixel.h>

#include <avr/io.h>
#include <util/delay.h>     /* for _delay_ms() */

#include "pulse.h"
#include "sampler.h"
#include "smooth.h"

// ===================
// = Pin Definitions =
// ===================

uint8_t leftLedPin = 5;
uint8_t leftSensorPin = PA1;
uint8_t leftRealLedPin = PB2;
uint8_t serialPin = 7;
int16_t ledBrightness = 0;

// ===========
// = Globals =
// ===========

// #define USE_SERIAL 1
// #define DEBUG 1
long timer = 0;
long loops = 0;
int numPixels;
uint16_t smoothedVoltages[filterSamples];

Sampler sampler = Sampler();
SoftwareSerial mySerial(0, serialPin); // RX, TX
Adafruit_NeoPixel strip = Adafruit_NeoPixel(68, leftRealLedPin, NEO_GRB + NEO_KHZ800);

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
    // pinMode(leftRealLedPin, OUTPUT);
    analogWrite(leftLedPin, ledBrightness);
    appState = STATE_RESTING;
    sampler.clear();
#ifdef USE_SERIAL || DEBUG
    mySerial.begin(9600);
#endif
    printHeader();
    strip.begin();
    for (int i=0; i < numPixels; i++) {
        strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show(); // Initialize all pixels to 'off'
    numPixels = strip.numPixels();
}

void loop() {
    // printHeader();
    uint16_t leftSensorVoltage = 0;
    uint16_t smoothedLeftVoltage = 0;
    int peaked = 0;

    if (appState == STATE_RESTING || appState == STATE_ON || appState == STATE_LED_FALLING) {
        leftSensorVoltage = readSensorValues(leftSensorPin);
        smoothedLeftVoltage = digitalSmooth(leftSensorVoltage, smoothedVoltages, mySerial);
        sampler.add(smoothedLeftVoltage);
        peaked = sampler.isPeaked(mySerial);
#ifdef DEBUG
        mySerial.print(peaked < 0 ? "---" : peaked > 0 ? "***" : "...");
#endif
        // digitalWrite(leftRealLedPin, peaked > 0 ? HIGH : LOW);
    }
    
    if (peaked != 0 || appState == STATE_LED_RISING) {
        if (appState != STATE_LED_RISING) {
            for (int i=0; i < numPixels; i++) {
                strip.setPixelColor(i, 0, 0, 0);
            }
            strip.show();
        }
        appState = STATE_LED_RISING;
        ledBrightness = min(ledBrightness + 8, 255*4);
        analogWrite(leftLedPin, ledBrightness/4);
        
        int pixel = ledBrightness >> 4;
        // Reset old pixels that won't be refreshed
        strip.setPixelColor(max(0, pixel - 3), 0, 0, 0);
        
        // Fade new pixels
        strip.setPixelColor(pixel, strip.Color(255, 0, 0));
        strip.setPixelColor(max(0, pixel - 1), strip.Color(80, 0, 0));
        strip.setPixelColor(min(numPixels-1, pixel + 1), strip.Color(80, 0, 0));
        strip.setPixelColor(max(0, pixel - 2), strip.Color(6, 0, 0));
        strip.setPixelColor(min(numPixels-1, pixel + 2), strip.Color(6, 0, 0));
        strip.show();
        delay(1);
        if (ledBrightness >= (255*4)) {
            appState = STATE_ON;
            for (int i=0; i < numPixels; i++) {
                strip.setPixelColor(i, 0, 0, 0);
            }
            strip.show();
        }
    } else if ((peaked == 0 && appState == STATE_ON) || 
               appState == STATE_LED_FALLING) {
        appState = STATE_LED_FALLING;
        ledBrightness = max(0, ledBrightness - 16);
        analogWrite(leftLedPin, ledBrightness/4);
        delay(1);
        if (ledBrightness <= 0) {
            appState = STATE_RESTING;
            for (int i=0; i < numPixels; i++) {
                strip.setPixelColor(i, 0, 0, 0);
            }
            strip.show();
        }
    }
        
    if (leftSensorVoltage == 0 ||
        appState == STATE_LED_RISING || 
        appState == STATE_LED_FALLING) {
        return;
    }
    
#ifdef DEBUG
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
#endif
}

uint16_t readSensorValues(uint8_t sensorPin) {
    uint16_t sensorVoltage = analogRead(sensorPin);
    
    return sensorVoltage;
}

void printHeader() {
    if (millis() - timer > 1000) {
        sampler.calculateStats();
#ifdef DEBUG
        mySerial.print("------------------ ");
        mySerial.print(loops);
        mySerial.println(" ------------------");
#endif
        timer = millis();
        loops += 1;
    }
}
