#include <avr/io.h>
#include <util/delay.h>     /* for _delay_ms() */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <wdt.h>
#include <WString.h>
#if defined (__AVR_ATtiny84__)
#include <tiny/wiring.h>
#include <SoftwareSerial/SoftwareSerial.h>
#include <easing/QuinticEase.h>
#else
#include <HardwareSerial.h>
#endif
#include <neopixel/Adafruit_NeoPixel.h>
#include <si1143/si1143.h>

#include "pulse.h"
#include "sensor.h"
#include "smooth.h"

#define USE_SERIAL

// ===================
// = Pin Definitions =
// ===================

#if defined (__AVR_ATtiny84__)
const uint8_t stemLedPin = PB2;
const uint8_t serialPin = 7;
#elif defined (__AVR_ATmega328P__)
// See si1143.h:digiPin for sensor pins
const uint8_t sensor1Pin = 0; // SCL=18, SDA=19
const uint8_t sensor2Pin = 14; // SCL=A0, SDA=A1
const uint8_t stem1LedPin = 8;
const uint8_t stem2LedPin = 7;
const uint8_t petalRedPin = 6;
const uint8_t petalGreenPin = 5;
const uint8_t petalBluePin = 3;
const uint8_t petalWhitePin = 9;
#endif

// ===========
// = Globals =
// ===========

long timer = 0;
long loops = 0;

// Serial
#if defined (__AVR_ATtiny84__)
SoftwareSerial Serial(0, serialPin); // RX, TX
#endif

// Stem
const int NUMBER_OF_STEM1_LEDS = 300;
const int NUMBER_OF_STEM2_LEDS = 300;
const int8_t STEM1_PULSE_WIDTH = 32;
const int8_t STEM2_PULSE_WIDTH = 8;
volatile int strip1CurrentLed = 0;
volatile int strip2CurrentLed = 0;
const int REST_PULSE_WIDTH = 4;

// Petals
unsigned long beginLedRiseTime = 0;
unsigned long endLedRiseTime = 0;
unsigned long beginLedFallTime = 0;
unsigned long endLedFallTime = 0;
const int16_t PETAL_DECAY_TIME = 1600;

// Pulse sensor
PortI2C myBus(sensor1Pin);
PulsePlug pulseA(myBus); 
PortI2C myBus2(sensor2Pin);
PulsePlug pulseB(myBus2); 

// States
typedef enum
{
    STATE_RESTING = 0,
    STATE_STEM_RISING = 1,
    STATE_STEM_FALLING = 2,
    STATE_LED_RISING = 3,
    STATE_LED_FALLING = 4
} state_app_t;
state_app_t app1State;
state_app_t app2State;
state_app_t restState;
state_app_t petalState;

typedef enum
{
    MODE_NONE = 0,
    MODE_SINGLE_A = 1,
    MODE_SINGLE_B = 2,
    MODE_DOUBLE = 3
} player_mode_t;
player_mode_t playerMode;
unsigned long lastSensorActiveA = 0;
unsigned long lastSensorActiveB = 0;
const int MILLISECONDS_SENSOR_DECAY = 5000;

// ============
// = Routines =
// ============

void setup(){
    int startRam = freeRam();
    analogReference(EXTERNAL);
    pinMode(petalRedPin, OUTPUT);
    pinMode(petalGreenPin, OUTPUT);
    pinMode(petalBluePin, OUTPUT);
    pinMode(petalWhitePin, OUTPUT);
    analogWrite(petalRedPin, 8);
    analogWrite(petalGreenPin, 0);
    analogWrite(petalBluePin, 0);
    analogWrite(petalWhitePin, 0);
    
    app1State = STATE_RESTING;
    app2State = STATE_RESTING;
    petalState = STATE_RESTING;
    playerMode = MODE_NONE;
    
    delay(50);
#ifdef USE_SERIAL
    Serial.begin(115200);
    Serial.flush();
    
    Serial.print(F(" ---> Free RAM: "));
    Serial.print(startRam);
    Serial.print(F(" - "));
    Serial.println(freeRam());
#endif
    printHeader();
    
    pinMode(stem1LedPin, OUTPUT);
    digitalWrite(stem1LedPin, LOW);
    pinMode(stem2LedPin, OUTPUT);
    digitalWrite(stem2LedPin, LOW);
    clearStemLeds(&pulseA);
    clearStemLeds(&pulseB);
    pulseA.role = ROLE_PRIMARY;
    pulseB.role = ROLE_SECONDARY;
    setupPulseSensor(&pulseA);
    setupPulseSensor(&pulseB);

    blink(3, 25, true);
}

void loop() {
    // printHeader();
    int sensor1On = readPulseSensor(&pulseA);
    int sensor2On = readPulseSensor(&pulseB);
    PulsePlug *pulse1 = &pulseA;
    PulsePlug *pulse2 = &pulseB;
    
#ifdef USE_SERIAL
    if (false && (sensor1On || sensor2On || app1State || app2State)) {
        Serial.print(F(" ---> Player mode: "));
        Serial.print(playerMode);
        Serial.print(F(", sensor1On: "));
        Serial.print(sensor1On);
        Serial.print(F(", sensor2On: "));
        Serial.print(sensor2On);
        Serial.print(F(", app1State: "));
        Serial.print(app1State);
        Serial.print(F(", app2State: "));
        Serial.println(app2State);
    }
#endif

    if (playerMode == MODE_NONE) {
        runResting();
    }
    
    if (sensor1On > 0) {
        lastSensorActiveA = millis();
        app1State = STATE_STEM_RISING;
        newHeartbeat(pulse1);
        if (playerMode == MODE_SINGLE_A) {
            newHeartbeat(pulse2);
        }
    } else if (app1State == STATE_STEM_RISING ||
               (playerMode == MODE_SINGLE_B && app2State == STATE_STEM_RISING)) {
        bool stem1Done = runStemRising(pulse1, playerMode == MODE_SINGLE_B ? pulse2 : pulse1);
        if (stem1Done && app1State == STATE_STEM_RISING) {
            app1State = STATE_RESTING;
            petalState = STATE_LED_RISING;
            if (playerMode == MODE_SINGLE_A) newHeartbeat(pulse2);
            newHeartbeat(pulse1);
            beginLedRising(pulse1);
        }
    } 
    
    if (sensor2On > 0) {
        lastSensorActiveB = millis();
        app2State = STATE_STEM_RISING;
        newHeartbeat(pulse2);
        if (playerMode == MODE_SINGLE_B) {
            newHeartbeat(pulse1);
        }
    } else if (app2State == STATE_STEM_RISING ||
               (playerMode == MODE_SINGLE_A && app1State == STATE_STEM_RISING)) {
        bool stem2Done = runStemRising(pulse2, playerMode == MODE_SINGLE_A ? pulse1 : pulse2);
        if (stem2Done && app2State == STATE_STEM_RISING) {
            app2State = STATE_RESTING;
            petalState = STATE_LED_RISING;
            if (playerMode == MODE_SINGLE_B) newHeartbeat(pulse1);
            newHeartbeat(pulse2);
            beginLedRising(pulse2);
        }
    } 
    
    if (petalState == STATE_LED_RISING) {
        bool ledDone = runLedRising(pulse1);
        if (ledDone) {
            if (app1State == STATE_LED_RISING) app1State = STATE_LED_FALLING;
            petalState = STATE_LED_FALLING;
            beginLedFalling(pulse1);
        }
    } else if (petalState == STATE_LED_FALLING) {
        bool ledDone = runLedFalling(pulse1);
        if (ledDone) {
            if (app1State == STATE_LED_FALLING) app1State = STATE_RESTING;
            petalState = STATE_RESTING;
        }
    }
    
    
    // Turn off inactive sensors
    determinePlayerMode();
}

void determinePlayerMode() {
    long decay = millis();
    decay = decay > MILLISECONDS_SENSOR_DECAY ? decay - MILLISECONDS_SENSOR_DECAY : 0;
    bool activeA = lastSensorActiveA > decay;
    bool activeB = lastSensorActiveB > decay;
    if (playerMode == MODE_NONE) {
        if (activeA && activeB) {
            playerMode = MODE_DOUBLE;
        } else if (activeA) {
            playerMode = MODE_SINGLE_A;
        } else if (activeB) {
            playerMode = MODE_SINGLE_B;
        }
    } else if (playerMode == MODE_DOUBLE || 
               playerMode == MODE_SINGLE_A || 
               playerMode == MODE_SINGLE_B) {
        if (activeA && activeB) {
            playerMode = MODE_DOUBLE;
        } else if (activeA && !activeB) {
            playerMode = MODE_SINGLE_A;
        } else if (!activeA && activeB) {
            playerMode = MODE_SINGLE_B;
        } else if (!activeA && !activeB) {
            playerMode = MODE_NONE;
            restState = STATE_RESTING;
        }
    }
}

void newHeartbeat(PulsePlug *pulse) {
    clearStemLeds(pulse);

    if (pulse->role == ROLE_PRIMARY) {
        strip1CurrentLed = 0;
    } else if (pulse->role == ROLE_SECONDARY) {
        strip2CurrentLed = 0;
    }
}

// ==========
// = States =
// ==========

void runResting() {
#ifdef USE_SERIAL
    // Serial.print(" ---> Run resting, current LED: ");
    // Serial.print(strip1CurrentLed);
    // Serial.print(", rest state: ");
    // Serial.println(restState);
#endif
    if (restState == STATE_RESTING) {
        clearStemLeds(&pulseA);
        clearStemLeds(&pulseB);
        restState = STATE_STEM_FALLING;
    } else if (restState == STATE_STEM_FALLING) {
        runRestStem();
    }
}

void runRestStem() {
    int currentLed = strip1CurrentLed - 1;
    if (currentLed < -1*REST_PULSE_WIDTH*2) {
        currentLed = NUMBER_OF_STEM1_LEDS + REST_PULSE_WIDTH*2;
    }
    strip1CurrentLed = currentLed;
    strip2CurrentLed = NUMBER_OF_STEM1_LEDS - strip1CurrentLed;
    runRestStem(&pulseA, strip1CurrentLed);
    runRestStem(&pulseB, strip2CurrentLed);
}

void runRestStem(PulsePlug *pulse, int16_t currentLed) {
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_STEM1_LEDS, 
                                                pulse->role == ROLE_PRIMARY ? stem1LedPin : stem2LedPin,
                                                NEO_GRB + NEO_KHZ800);
    // Serial.print(" Colors: ");
    for (int i=(-1*REST_PULSE_WIDTH); i <= REST_PULSE_WIDTH; i++) {
        // Serial.print((int)floor(255.0/(float)max(abs(i), 1)));
        // Serial.print(" ");
        strip.setPixelColor(currentLed+i, 0, 0, (int)floor(255.0/(float)max(abs(i), 1)));
    }
    // Serial.println(currentLed);
    strip.show();
}

void beginStateOn(PulsePlug *pulse) {
    
}

void clearStemLeds(PulsePlug *pulse) {
    // Reset stem LEDs
    uint8_t pulseWidth;
    if (pulse->role == ROLE_PRIMARY) {
        pulseWidth = STEM1_PULSE_WIDTH;
    } else if (pulse->role == ROLE_SECONDARY) {
        pulseWidth = STEM2_PULSE_WIDTH;
    }
    int ledCount = pulse->role == ROLE_PRIMARY ? NUMBER_OF_STEM1_LEDS : NUMBER_OF_STEM2_LEDS;
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledCount, 
                                             pulse->role == ROLE_PRIMARY ? stem1LedPin : stem2LedPin, 
                                             NEO_GRB + NEO_KHZ800);
    strip.begin();
    for (int i=0; i < ledCount; i++) {
        strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
#ifdef USE_SERIAL
    // Serial.print(F(" ---> Clearing stem #"));
    // Serial.println(pulse->role);
#endif
    unsigned long now = millis();
    int8_t bpm = max(min(pulse->latestBpm, 100), 45);
    unsigned long nextBeat = pulse->lastBeat + (int)floor(60000.0/(bpm/.5));
    unsigned long millisToNextBeat = (now > nextBeat) ? 0 : (nextBeat - now);

    pulse->ease.setDuration(millisToNextBeat);
    pulse->ease.setTotalChangeInPosition(ledCount + 2*pulseWidth);
}

bool runStemRising(PulsePlug *pulse, PulsePlug *shadowPulse) {
    unsigned long now = millis();
    int8_t bpm = max(min(shadowPulse->latestBpm, 100), 45);
    unsigned long nextBeat = shadowPulse->lastBeat + (int)floor(60000.0/(bpm/.5));
    unsigned long millisToNextBeat = (now > nextBeat) ? 0 : (nextBeat - now);
    unsigned long millisFromLastBeat = now - shadowPulse->lastBeat;
    double progress = (double)millisFromLastBeat / 
                      (double)(millisFromLastBeat + millisToNextBeat);
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(pulse->role == ROLE_PRIMARY ? NUMBER_OF_STEM1_LEDS : NUMBER_OF_STEM2_LEDS, 
                                             pulse->role == ROLE_PRIMARY ? stem1LedPin : stem2LedPin, 
                                             NEO_GRB + NEO_KHZ800);
    uint8_t pulseWidth;
    if (pulse->role == ROLE_PRIMARY) {
        pulseWidth = STEM1_PULSE_WIDTH;
    } else if (pulse->role == ROLE_SECONDARY) {
        pulseWidth = STEM2_PULSE_WIDTH;
    }

#ifdef USE_SERIAL
    // Serial.print(" ---> Strip: ");
    // Serial.print(pulse->role);
    // Serial.print("=");
    // Serial.print(pulse->role==ROLE_PRIMARY);
    // Serial.print("=");
    // Serial.println(pulse->role==ROLE_SECONDARY);
#endif
    int ledCount = strip.numPixels();
    int currentLed = pulse->role == ROLE_PRIMARY ? strip1CurrentLed : strip2CurrentLed;
    // int newLed = (int)floor(progress * ledCount); // Linear
    int newLed = (int)floor(shadowPulse->ease.easeIn(millisFromLastBeat)) - pulseWidth;
    
#ifdef USE_SERIAL
    // Serial.print("LEDs (");
    // Serial.print(ledCount);
    // Serial.print(") ");
    // Serial.print(currentLed);
    // Serial.print(" -> ");
    // Serial.println(newLed);
#endif
    
    if (currentLed != newLed) {
        // Reset old pixels that won't be refreshed
        for (int i = -1*pulseWidth; i < pulseWidth; i++) {
            if (currentLed + i < 0 || currentLed + i >= ledCount) continue;
            strip.setPixelColor(currentLed + i, strip.Color(0, 0, 0));
        }
        
        currentLed = newLed;
        if (pulse->role == ROLE_PRIMARY) {
            strip1CurrentLed = newLed;
        } else if (pulse->role == ROLE_SECONDARY) {
            strip2CurrentLed = newLed;
        }

        // Fade new pixels
#ifdef USE_SERIAL
        // Serial.print("LEDs (");
        // Serial.print(ledCount);
        // Serial.print(") ");
        // Serial.print(" ---> Pulsing (width: ");
        // Serial.print(pulseWidth);
        // Serial.println(") ");
        // Serial.print(" ---> Free RAM: ");
        // Serial.println(freeRam());
#endif
        for (int i = -1*pulseWidth; i < pulseWidth; i++) {
            if ((currentLed + i < 0) || (currentLed + i >= ledCount)) {
#ifdef USE_SERIAL
                // Serial.print(F(" ---> REJECTED in stem pulse width: "));
                // Serial.print(currentLed + i, DEC);
                // Serial.print(F(" ("));
                // Serial.print(i);
                // Serial.print(F(") / "));
                // Serial.println(ledCount, DEC);
#endif
                continue;
            }
#ifdef USE_SERIAL
            // Serial.print(F(" ---> in stem pulse width: "));
            // Serial.print(currentLed + i);
            // Serial.print(F(" ("));
            // Serial.print(i);
            // Serial.print(F(") / "));
            // Serial.println((int)floor(255.0/(max(abs(i)-1, 1))));
#endif
            uint32_t color;
            if (playerMode == MODE_DOUBLE) {
                if (pulse->role == ROLE_PRIMARY) {
                    color = strip.Color((int)floor(255.0/(float)max(abs(i)-2, 1)), 0, 0);
                } else {
                    color = strip.Color(0, (int)floor(255.0/(float)max(abs(i)-2, 1)), 0);
                }
            } else {
                color = strip.Color((int)floor(255.0/(float)max(abs(i)-2, 1)), 0, 0);
            }
            strip.setPixelColor(currentLed + i, color);
        }
        strip.show();
    }
    
    // At end of stem
    if (currentLed == ledCount || progress >= 1.0) {
        return true;
    }

    return false;
}

void beginLedRising(PulsePlug *pulse) {
    int bpm = max(min(pulse->latestBpm, 100), 45);
    unsigned long nextBeat = pulse->lastBeat + 60000.0/bpm;
    unsigned long now = millis();

#ifdef USE_SERIAL
    Serial.print(F(" ---> Led Rising: "));
    Serial.println(nextBeat);    
#endif

    beginLedRiseTime = now;

    if (now < endLedFallTime) {
        // Compensate for still falling led
        unsigned int remainingFallTime = endLedFallTime - now;
#ifdef USE_SERIAL
        Serial.print(F(" ---> Compensating for still falling led: "));
        Serial.println(remainingFallTime, DEC);
#endif
        beginLedRiseTime = beginLedRiseTime - (int)floor((400.0*((double)remainingFallTime/PETAL_DECAY_TIME)));
        beginLedFallTime = 0;
        endLedFallTime = 0;
    }
    endLedRiseTime = beginLedRiseTime + 0.6*(nextBeat - beginLedRiseTime);
    // endLedRiseTime = beginLedRiseTime + 400;
}

bool runLedRising(PulsePlug *pulse) {
    uint16_t ledBrightness;
    unsigned long now = millis();
    unsigned long millisToNextBeat = (now > endLedRiseTime) ? 0 : (endLedRiseTime - now);
    unsigned long millisFromLastBeat = now - beginLedRiseTime;
    double progress = (double)millisFromLastBeat / (double)(millisFromLastBeat + millisToNextBeat);

#ifdef USE_SERIAL
    // Serial.print(" ---> STATE: Led Rising - from:");
    // Serial.print(millisFromLastBeat, DEC);
    // Serial.print(" to:");
    // Serial.print(millisToNextBeat, DEC);
    // Serial.print(" Brightness: ");
    // Serial.println(max(8, min((int)floor(255 * progress), 255)), DEC);
#endif

    // Set Lotus LED brightness
    ledBrightness = max(8, min((int)floor(255 * progress), 255));
    analogWrite(petalRedPin, ledBrightness);
    if (progress >= 1.0) {
        return true;
    }

    return false;
}

void beginLedFalling(PulsePlug *pulse) {
    int bpm = max(min(pulse->latestBpm, 100), 45);
    unsigned long nextBeat = pulse->lastBeat + 60000.0/bpm;
#ifdef USE_SERIAL
    Serial.print(F(" ---> Led Falling: "));
    Serial.println(nextBeat);    
#endif

    beginLedFallTime = millis();
    // endLedFallTime = beginLedFallTime + 2*(nextBeat - beginLedFallTime);
    endLedFallTime = beginLedFallTime + PETAL_DECAY_TIME;
}

bool runLedFalling(PulsePlug *pulse) {
    uint16_t ledBrightness;
    unsigned long now = millis();
    unsigned long millisToNextBeat = (now > endLedFallTime) ? 0 : (endLedFallTime - now);
    unsigned long millisFromLastBeat = now - beginLedFallTime;
    double progress = (double)millisFromLastBeat / (double)(millisFromLastBeat + millisToNextBeat);

#ifdef USE_SERIAL
    // Serial.print(" ---> STATE: Led Falling - from:");
    // Serial.print(millisFromLastBeat, DEC);
    // Serial.print(" to:");
    // Serial.print(millisToNextBeat, DEC);
    // Serial.print(" Brightness: ");
    // Serial.println(max(255 - (int)floor(255 * progress), 8), DEC);
#endif
    ledBrightness = max(255 - (int)floor(255 * progress), 8);
    analogWrite(petalRedPin, ledBrightness);
    
    if (progress >= 1.0) {
        return true;
    }
    
    return false;
}

// ====================
// = Serial debugging =
// ====================

void printHeader() {
    if (millis() - timer > 1000) {
#ifdef USE_SERIAL
        Serial.print(F("------------------ "));
        Serial.print(loops);
        Serial.println(" ------------------");
#endif
        timer = millis();
        loops += 1;
    }
}

void blink(int loops, int loopTime, bool half) {
    while (loops--) {
        digitalWrite(petalRedPin, HIGH);
        delay(loopTime);
        digitalWrite(petalRedPin, LOW);
        delay(loopTime / (half ? 2 : 1));
    }
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
