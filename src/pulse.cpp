#include <avr/io.h>
#include <util/delay.h>     /* for _delay_ms() */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#if defined (__AVR_ATtiny84__)
#include <tiny/wiring.h>
#include <SoftwareSerial/SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#endif
#include <neopixel/Adafruit_NeoPixel.h>

#include "pulse.h"
#include "sampler.h"
#include "smooth.h"

// ===================
// = Pin Definitions =
// ===================

#if defined (__AVR_ATtiny84__)
uint8_t leftLedPin = 5;
uint8_t leftSensorPin = PA1;
uint8_t leftRealLedPin = PB2;
uint8_t serialPin = 7;
#elif defined (__AVR_ATmega328P__)
uint8_t leftLedPin = 10;
uint8_t leftSensorPin = A0;
uint8_t leftRealLedPin = 6;
#endif

// ===========
// = Globals =
// ===========

#define USE_SERIAL 1
long timer = 0;
long loops = 0;
int numPixels;
uint16_t smoothedVoltages[filterSamples];
int16_t ledBrightness = 0;

Sampler sampler = Sampler();
#if defined (__AVR_ATtiny84__)
SoftwareSerial Serial(0, serialPin); // RX, TX
#endif
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, leftRealLedPin, NEO_GRB + NEO_KHZ800);

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
    analogReference(EXTERNAL);
    // analogReference(DEFAULT);
    pinMode(leftLedPin, OUTPUT);
    analogWrite(leftLedPin, ledBrightness);
    appState = STATE_RESTING;
    sampler.clear();
#ifdef USE_SERIAL
    Serial.begin(9600);
    Serial.flush();
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
        smoothedLeftVoltage = digitalSmooth(leftSensorVoltage, smoothedVoltages);
        sampler.add(smoothedLeftVoltage);
        peaked = sampler.isPeaked();
#ifdef USE_SERIAL
        if (peaked) {
            // Serial.println(peaked < 0 ? "---" : peaked > 0 ? "***" : "...");
        }
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
        strip.setPixelColor(min(numPixels-1, pixel + 2), strip.Color(6, 0, 0));
        strip.setPixelColor(min(numPixels-1, pixel + 1), strip.Color(80, 0, 0));
        strip.setPixelColor(pixel, strip.Color(255, 0, 0));
        strip.setPixelColor(max(0, pixel - 1), strip.Color(80, 0, 0));
        strip.setPixelColor(max(0, pixel - 2), strip.Color(6, 0, 0));
        strip.show();
        // delay(1);
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
        // delay(1);
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
    
#ifdef USE_SERIAL
    Serial.print(leftSensorVoltage, DEC);
    Serial.print("   smooth=");
    Serial.print(smoothedLeftVoltage, DEC);    
    Serial.print("   p50=");
    Serial.print(sampler.getPercentile(.5), DEC);
    Serial.print("   p70=");
    Serial.print(sampler.getPercentile(.7), DEC);
    Serial.print("   period=");
    Serial.print(sampler.getPeriod(), DEC);
    Serial.print(peaked ? " *** " : " ");
    Serial.print("   state=");
    Serial.print(appState, DEC);
    Serial.print("   brightness=");
    Serial.println(ledBrightness, DEC);
#endif
}

uint16_t readSensorValues(uint8_t sensorPin) {
    uint16_t sensorVoltage = analogRead(sensorPin);
    
    return sensorVoltage;
}

void printHeader() {
    if (millis() - timer > 1000) {
        sampler.calculateStats();
#ifdef USE_SERIAL
        Serial.print("------------------ ");
        Serial.print(loops);
        Serial.println(" ------------------");
#endif
        timer = millis();
        loops += 1;
    }
}
