// Pulse & Bloom - Pulse Sensor and Lighting
// 2014 - Samuel Clay, samuel@ofbrooklyn.com

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
#else
#include <HardwareSerial.h>
#endif
#include <neopixel/Adafruit_NeoPixel.h>
#include <si1143/si1143.h>
#include <QuadraticEase.h>

#include "pulse.h"
#include "sensor.h"
#include "smooth.h"

// #define USE_SERIAL

// ===================
// = Pin Definitions =
// ===================

#if defined (__AVR_ATtiny84__)
const uint8_t stemLedPin          = PB2;
const uint8_t serialPin           = 7;
#elif defined (__AVR_ATmega328P__)
// See si1143.h:digiPin for sensor pins
const uint8_t sensorAPin          = 0;  // SCL=18, SDA=19
const uint8_t sensorBPin          = 14; // SCL=A0, SDA=A1
const uint8_t stemALedPin         = 8;
const uint8_t stemBLedPin         = 7;
const uint8_t petalRedPin         = 3;
const uint8_t petalGreenPin       = 6;
const uint8_t petalBluePin        = 5;
const uint8_t petalWhitePin       = 9;
#endif

// ===========
// = Globals =
// ===========

// Stem
const int NUMBER_OF_STEM_LEDS     = 300;
const int REST_PULSE_WIDTH        = 8;
const int8_t STEMA_PULSE_WIDTH    = 16;
const int8_t STEMB_PULSE_WIDTH    = 16;
volatile int16_t stripACurrentLed = 0;
volatile int16_t stripBCurrentLed = 0;
volatile int16_t splitPivotPoint  = 0;
unsigned long beginSplitTime      = 0;
unsigned long endSplitTime        = 0;
const int STEM_SPLIT_MS           = 800;
const double STEM_DURATION_PCT    = 0.65;

QuadraticEase splitEase;

// Petals
unsigned long beginLedRiseTime    = 0;
unsigned long endLedRiseTime      = 0;
unsigned long beginLedFallTime    = 0;
unsigned long endLedFallTime      = 0;
unsigned long beginLedRestTime    = 0;
unsigned long endLedRestTime      = 0;
uint8_t ledBrightness             = 0;
uint8_t lastBpm                   = 75;
const int16_t PETAL_DECAY_MS      = 850;
const int16_t PETAL_DELAY_MS      = 60;

// Pulses
unsigned long lastPulseATime      = 0;
unsigned long lastPulseBTime      = 0;
unsigned long nextPulseATime      = 0;
unsigned long nextPulseBTime      = 0;
unsigned int bpmPulseA            = lastBpm;
unsigned int bpmPulseB            = lastBpm;
unsigned long lastSensorActiveA   = 0;
unsigned long lastSensorActiveB   = 0;
unsigned long lastFingerSeenA     = 0;
unsigned long lastFingerSeenB     = 0;
const int SENSOR_DECAY_MS         = 60000;
const int FINGERLESS_DECAY_MS     = 1500;
const double SHOW_HEARTBEAT_PCT   = 0.35;

// Pulse sensor
PortI2C myBus(sensorAPin);
PulsePlug pulseA(myBus); 
PortI2C myBus2(sensorBPin);
PulsePlug pulseB(myBus2); 

// States
typedef enum
{
    STATE_RESTING       = 0,
    STATE_AWAKE         = 1,
    STATE_STEM_RISING   = 2,
    STATE_STEM_FALLING  = 3,
    STATE_PETAL_RISING  = 4,
    STATE_PETAL_FALLING = 5,
    STATE_PETAL_LANDING = 6
} state_app_t;
state_app_t app1State;
state_app_t app2State;
state_app_t restState;
state_app_t petalState;

typedef enum
{
    MODE_NONE     = 0,
    MODE_SINGLE_A = 1,
    MODE_SINGLE_B = 2,
    MODE_DOUBLE   = 3
} player_mode_t;
player_mode_t playerMode;
player_mode_t fingerMode;

// Serial
#if defined (__AVR_ATtiny84__)
SoftwareSerial Serial(0, serialPin); // RX, TX
#endif

// Debugging
long debuggingTimer   = 0;
long debuggingSeconds = 0;

// ========
// = Init =
// ========

void setup(){
    analogReference(EXTERNAL);
    pinMode(petalRedPin, OUTPUT);
    pinMode(petalGreenPin, OUTPUT);
    pinMode(petalBluePin, OUTPUT);
    pinMode(petalWhitePin, OUTPUT);
    
    analogWrite(petalRedPin, 0);
    analogWrite(petalGreenPin, 0);
    analogWrite(petalBluePin, 0);
    analogWrite(petalWhitePin, 0);
    
    app1State  = STATE_RESTING;
    app2State  = STATE_RESTING;
    petalState = STATE_RESTING;
    restState  = STATE_RESTING;
    playerMode = MODE_NONE;
    fingerMode = MODE_NONE;
    
    delay(50);
#ifdef USE_SERIAL
    Serial.begin(115200);
    Serial.flush();
    
    Serial.print(F(" ---> Free RAM: "));
    Serial.println(freeRam());
#endif
    printHeader();
    
    pinMode(stemALedPin, OUTPUT);
    digitalWrite(stemALedPin, LOW);
    pinMode(stemBLedPin, OUTPUT);
    digitalWrite(stemBLedPin, LOW);
    clearStemLeds(&pulseA);
    clearStemLeds(&pulseB);
    pulseA.role = ROLE_PRIMARY;
    pulseB.role = ROLE_SECONDARY;
    setupPulseSensor(&pulseA);
    setupPulseSensor(&pulseB);

    for (int i=0; i < 255; i++) {
        analogWrite(petalRedPin, i);
        delay(1);
    }
    for (int i=255; i > 0; i--) {
        analogWrite(petalRedPin, i);
        delay(1);
    }
    for (int i=0; i < 255; i++) {
        analogWrite(petalGreenPin, i);
        delay(1);
    }
    for (int i=255; i > 0; i--) {
        analogWrite(petalGreenPin, i);
        delay(1);
    }
    for (int i=0; i < 255; i++) {
        analogWrite(petalBluePin, i);
        delay(1);
    }
    for (int i=255; i > 0; i--) {
        analogWrite(petalBluePin, i);
        delay(1);
    }


    analogWrite(petalRedPin, 0);
    analogWrite(petalGreenPin, 0);
    analogWrite(petalBluePin, 255);
    analogWrite(petalWhitePin, 0);
    
    // blink(3, 25, true);
}

void loop() {
    bool heartbeat1 = lastPulseATime && nextPulseATime < millis();
    bool heartbeat2 = lastPulseBTime && nextPulseBTime < millis();
    int sensor1On = readPulseSensor(&pulseA);
    int sensor2On = readPulseSensor(&pulseB);
    PulsePlug *pulse1 = &pulseA;
    PulsePlug *pulse2 = &pulseB;
    
#ifdef USE_SERIAL
    if (true && (heartbeat1 || heartbeat2 || 
                 sensor1On >= 0 || sensor2On >= 0 || 
                 app1State || app2State)) {
        Serial.print(F(" ---> ["));
        Serial.print(millis());
        Serial.print(F("] mode: "));        Serial.print(playerMode);
        Serial.print(F(" heartbeat: "));    Serial.print(heartbeat1);
        Serial.print(F("/"));               Serial.print(heartbeat2);
        Serial.print(F(" sensorOn: "));     Serial.print(sensor1On);
        Serial.print(F("/"));               Serial.print(sensor2On);
        Serial.print(F(" appState: "));     Serial.print(app1State);
        Serial.print(F("/"));               Serial.print(app2State);
        Serial.print(F(" restState: "));    Serial.print(restState);
        Serial.print(F("/"));               Serial.print(app2State);
        Serial.print(F(" petalState: "));   Serial.print(petalState);
        Serial.println();
    }
#endif    
    
    // Found fingers, now in transitory state, just skip everything until done
    if (restState == STATE_STEM_RISING) {
        runSplittingStem();
        return;
    }
    if (playerMode == MODE_NONE) {
        lastPulseATime = 0;
        lastPulseBTime = 0;
        nextPulseATime = 0;
        nextPulseBTime = 0;
        app1State = STATE_RESTING;
        app2State = STATE_RESTING;
        if (fingerMode == MODE_NONE) {
            runResting();
            if (petalState != STATE_RESTING) {
                petalState = STATE_PETAL_LANDING;
            }
        }
    }
    
    // Check for real heartbeat, adjust fake heartbeat
    if (sensor1On > 0) {
        bpmPulseA = adjustBpm(pulse1);
        float progress = 1.0;
        if (lastPulseATime) {
            progress = ((float)(millis()-lastPulseATime))/
                       ((float)(nextPulseATime-lastPulseATime));
        }
        lastSensorActiveA = millis();
        if (progress > SHOW_HEARTBEAT_PCT) {
            // If half-way to next heartbeat, immediately show heartbeat
            heartbeat1 = true;
        } else {
            // If not half-way to next heartbeat, push out next heartbeat to align
            // with this real heartbeat.
            Serial.print(" ---> Saw heartbeatA, delaying: ");
            Serial.println(progress);

            nextPulseATime = millis() + 60000.0/bpmPulseA;
        }
    }
    if (sensor2On > 0) {
        float progress = 1.0;
        if (lastPulseBTime) {
            progress = ((float)(millis()-lastPulseBTime))/((float)(nextPulseBTime-lastPulseBTime));
        }
        lastSensorActiveB = millis();
#ifdef USE_SERIAL
        Serial.print(" ---> Progress: ");
        Serial.print(progress);
        Serial.print(" lastbpm: ");
        Serial.print(lastBpm);
        Serial.print(" bpm: ");
        Serial.print(pulse2->latestBpm);
        Serial.print(" newbpm: ");
        Serial.println(adjustBpm(pulse2));
#endif
        bpmPulseB = adjustBpm(pulse2);
        if (progress > SHOW_HEARTBEAT_PCT) {
            heartbeat2 = true;
        } else {
            Serial.print(" ---> Saw heartbeatB, delaying: ");
            Serial.println(progress);
            nextPulseBTime = millis() + 60000.0/bpmPulseB;
        }
    }

    // If finger's gone, ensure no faked heartbeat
    if (sensor1On < 0) {
        heartbeat1 = false;
        lastPulseATime = 0;
        nextPulseATime = 0;
    } else {
        lastFingerSeenA = millis();
    }
    if (sensor2On < 0) {
        heartbeat2 = false;
        lastPulseBTime = 0;
        nextPulseBTime = 0;
    } else {
        lastFingerSeenB = millis();
    }
    
    // Check for faked/real heartbeat, turn on stem
    if (heartbeat1) {
        lastPulseATime = millis();
        nextPulseATime = lastPulseATime + 60000.0/bpmPulseA;
        if (app1State == STATE_STEM_RISING && petalState != STATE_PETAL_RISING) {
            // If stem is already rising and is about to be cut off, pulse petals
            petalState = STATE_PETAL_RISING;
            beginPetalRising();
        }
        app1State = STATE_STEM_RISING;
        resetStem(pulse1);
        // Use both stems if in single player mode
        if (playerMode == MODE_SINGLE_A) resetStem(pulse2);
    } else if (app1State == STATE_STEM_RISING ||
               (playerMode == MODE_SINGLE_B && app2State == STATE_STEM_RISING)) {
        // Run stem, move to petal when complete
        bool stem1Done = runStemRising(pulse1, playerMode == MODE_SINGLE_B ? pulse2 : pulse1);
        if (stem1Done && app1State == STATE_STEM_RISING) {
            app1State = STATE_RESTING;
            resetStem(pulse1);
            if (playerMode == MODE_SINGLE_A) resetStem(pulse2);
            if (petalState != STATE_PETAL_RISING) {
                petalState = STATE_PETAL_RISING;
                beginPetalRising();
            }
        }
    } 
    
    if (heartbeat2) {
        lastPulseBTime = millis();
        nextPulseBTime = lastPulseBTime + 60000.0/bpmPulseB;
        if (app2State == STATE_STEM_RISING && petalState != STATE_PETAL_RISING) {
            // If stem is already rising and is about to be cut off, pulse petals
            petalState = STATE_PETAL_RISING;
            beginPetalRising();
        }
        app2State = STATE_STEM_RISING;
        resetStem(pulse2);
        if (playerMode == MODE_SINGLE_B) {
            resetStem(pulse1);
        }
    } else if (app2State == STATE_STEM_RISING ||
               (playerMode == MODE_SINGLE_A && app1State == STATE_STEM_RISING)) {
        bool stem2Done = runStemRising(pulse2, playerMode == MODE_SINGLE_A ? pulse1 : pulse2);
        if (stem2Done && app2State == STATE_STEM_RISING) {
            app2State = STATE_RESTING;
            if (playerMode == MODE_SINGLE_B) resetStem(pulse1);
            resetStem(pulse2);
            if (petalState != STATE_PETAL_RISING) {
                petalState = STATE_PETAL_RISING;
                beginPetalRising();
            }
        }
    } 
    
    if (petalState == STATE_PETAL_LANDING) {
        bool ledDone = runPetalResting();
        if (ledDone) {
            petalState = STATE_RESTING;
        }
    } else if (petalState == STATE_PETAL_RISING) {
        bool ledDone = runPetalRising();
        if (ledDone) {
            if (app1State == STATE_PETAL_RISING) app1State = STATE_PETAL_FALLING;
            if (app2State == STATE_PETAL_RISING) app2State = STATE_PETAL_FALLING;
            petalState = STATE_PETAL_FALLING;
            beginPetalFalling();
        }
    } else if (petalState == STATE_PETAL_FALLING) {
        bool ledDone = runPetalFalling();
        if (ledDone) {
            if (app1State == STATE_PETAL_FALLING) app1State = STATE_RESTING;
            if (app2State == STATE_PETAL_FALLING) app2State = STATE_RESTING;
            // Serial.println(" ---> PETAL DONE, RESTING");
            petalState = STATE_AWAKE;
        }
    }

    // Turn off inactive sensors
    determinePlayerMode();
    determineFingerMode(sensor1On, sensor2On);
}

// =================
// = State Machine =
// =================

void determinePlayerMode() {
    unsigned long decay = millis();
    unsigned long fingerDecay;
    decay = decay > (unsigned long)SENSOR_DECAY_MS ? decay - SENSOR_DECAY_MS : 0;
    bool activeA = lastSensorActiveA > decay;
    bool activeB = lastSensorActiveB > decay;
    
    if (activeA && lastFingerSeenA) {
        fingerDecay = millis();
        fingerDecay = fingerDecay > (unsigned long)FINGERLESS_DECAY_MS ? 
                      fingerDecay - FINGERLESS_DECAY_MS : 0;
        activeA = lastFingerSeenA > fingerDecay;
    }
    if (activeB && lastFingerSeenB) {
        fingerDecay = millis();
        fingerDecay = fingerDecay > (unsigned long)FINGERLESS_DECAY_MS ? 
                      fingerDecay - FINGERLESS_DECAY_MS : 0;
            activeB = lastFingerSeenB > fingerDecay;
    }

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

void determineFingerMode(int sensor1On, int sensor2On) {
    player_mode_t originalFingerMode = fingerMode;
    
    if (sensor1On >= 0 && sensor2On >= 0) {
        fingerMode = MODE_DOUBLE;
    } else if (sensor1On >= 0) {
        fingerMode = MODE_SINGLE_A;
    } else if (sensor2On >= 0) {
        fingerMode = MODE_SINGLE_B;
    } else {
        fingerMode = MODE_NONE;
    }

#ifdef USE_SERIAL
    // Serial.print(" ---> Finger mode: ");
    // Serial.print(fingerMode);
    // Serial.print("/");
    // Serial.println(originalFingerMode);
#endif

    if (fingerMode != originalFingerMode) {
        if (originalFingerMode == MODE_NONE && restState == STATE_STEM_FALLING) {
            // When moving to a finger from rest, split stem.
            beginSplittingStem();
        } else if (fingerMode == MODE_DOUBLE &&
                   (originalFingerMode == MODE_SINGLE_A || 
                    originalFingerMode == MODE_SINGLE_B)) {
            // When moving to double fingers, possibly freezing of stem leds.
            clearStemLeds(&pulseA);
            clearStemLeds(&pulseB);
        }
    }
}

void resetStem(PulsePlug *pulse) {
    clearStemLeds(pulse);

    if (pulse->role == ROLE_PRIMARY) {
        stripACurrentLed = 0;
    } else if (pulse->role == ROLE_SECONDARY) {
        stripBCurrentLed = 0;
    }
}

void clearStemLeds(PulsePlug *pulse) {
    // Reset stem LEDs
    uint8_t pulseWidth = STEMA_PULSE_WIDTH;
    uint8_t bpm = bpmPulseA;
    unsigned long lastBeat = lastPulseATime;
    if (pulse->role == ROLE_SECONDARY) {
        pulseWidth = STEMB_PULSE_WIDTH;
        bpm = bpmPulseB;
        lastBeat = lastPulseBTime;
    }
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_STEM_LEDS, 
                                                pulse->role == ROLE_PRIMARY ? 
                                                stemALedPin : stemBLedPin, 
                                                NEO_GRB + NEO_KHZ800);
    strip.begin();
    for (int i=0; i < NUMBER_OF_STEM_LEDS; i++) {
        strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
#ifdef USE_SERIAL
    // Serial.print(F(" ---> Clearing stem #"));
    // Serial.println(pulse->role);
#endif
    unsigned long now = millis();
    unsigned long nextBeat = lastBeat + (int)floor(60000.0/(bpm/STEM_DURATION_PCT));
    unsigned long millisToNextBeat = (now > nextBeat) ? 0 : (nextBeat - now);

    pulse->ease.setDuration(millisToNextBeat);
    pulse->ease.setTotalChangeInPosition(NUMBER_OF_STEM_LEDS + 2*pulseWidth);
}

uint8_t adjustBpm(PulsePlug *pulse) {
    uint8_t bpm = pulse->latestBpm;
    bpm = (uint8_t)floor(smooth((float)bpm, .33, (float)lastBpm));

    if (bpm < 45) bpm = 75;
    if (bpm > 105) bpm = 75;

    lastBpm = bpm;

    return bpm;
}

// ==========
// = States =
// ==========

void runResting() {
#ifdef USE_SERIAL
    // Serial.print(F(" ---> Run resting, current LED: "));
    // Serial.print(stripACurrentLed);
    // Serial.print(F(", rest state: "));
    // Serial.println(restState);
#endif
    if (restState == STATE_RESTING) {
        resetStem(&pulseA);
        resetStem(&pulseB);
#ifdef USE_SERIAL
        Serial.println(" ---> RESETTING Strip current led, due to start resting");
#endif
        stripACurrentLed = NUMBER_OF_STEM_LEDS + REST_PULSE_WIDTH;
        restState = STATE_STEM_FALLING;
        runRestStem();
        beginPetalResting();
    } else if (restState == STATE_STEM_FALLING) {
        runRestStem();
    }
}

void runRestStem() {
    int16_t currentLed = stripACurrentLed - 1;
    if (currentLed < -1*REST_PULSE_WIDTH) {
        currentLed = NUMBER_OF_STEM_LEDS + REST_PULSE_WIDTH;
    }
    stripACurrentLed = currentLed;
    stripBCurrentLed = NUMBER_OF_STEM_LEDS - stripACurrentLed;
    runRestStem(&pulseA, stripACurrentLed);
    runRestStem(&pulseB, stripBCurrentLed);
}

void runRestStem(PulsePlug *pulse, int16_t currentLed) {
    double progress = min(max(0, (float)currentLed/(float)NUMBER_OF_STEM_LEDS), 1.0);
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_STEM_LEDS, 
                                                pulse->role == ROLE_PRIMARY ? 
                                                stemALedPin : stemBLedPin,
                                                NEO_GRB + NEO_KHZ800);
    // Serial.print(" Colors ");
    // Serial.print((int)floor(100*progress));
    // Serial.print("%: ");
    int head = pulse->role == ROLE_PRIMARY ? 0 : (-1*REST_PULSE_WIDTH);
    int tail = pulse->role == ROLE_PRIMARY ? (REST_PULSE_WIDTH) : 1;
                                            
    for (int i=head; i < tail; i++) {
        // Serial.print((int)floor((progress)*255.0/(float)max(abs(i), 1)));
        // Serial.print("/");
        // Serial.print((int)floor(1-progress*255.0/(float)max(abs(i), 1)));
        // Serial.print(" ");
        strip.setPixelColor(currentLed+i, 0, 
                            progress*255.0, 
                            (int)floor(255.0/(float)max(abs(i*3), 1)));
    }
    // Serial.println(currentLed);
    strip.show();
}

// ======================
// = States - Splitting =
// ======================

void beginSplittingStem() {
#ifdef USE_SERIAL
    Serial.print(F(" ---> Run splitting, current LED: "));
    Serial.print(stripACurrentLed);
    Serial.print(F(", rest state: "));
    Serial.println(restState);
#endif
    if (restState == STATE_RESTING || restState == STATE_STEM_FALLING) {
        splitEase.setDuration(STEM_SPLIT_MS);
        splitEase.setTotalChangeInPosition(NUMBER_OF_STEM_LEDS + STEMA_PULSE_WIDTH);
        beginSplitTime = millis();
        splitPivotPoint = stripACurrentLed;
        restState = STATE_STEM_RISING;
    }
}

void runSplittingStem() {
    int progress = (int)ceil(splitEase.easeIn(millis()-beginSplitTime));
    int total = NUMBER_OF_STEM_LEDS + STEMA_PULSE_WIDTH - splitPivotPoint;
#ifdef USE_SERIAL
    Serial.print(" --> Splitting progress: ");
    Serial.print(progress);
    Serial.print(" pivoting: ");
    Serial.println(splitPivotPoint);
#endif
    
    if (splitPivotPoint == 0) {
#ifdef USE_SERIAL
        Serial.println(" ---> Ignoring split");
#endif
        restState = STATE_RESTING;
        return;
    }
    
    if (progress >= total) {
#ifdef USE_SERIAL
        Serial.println(" --> Done splitting!");
#endif
        restState = STATE_RESTING;
        return;
    }
    int16_t currentLed = splitPivotPoint + progress;
    if (currentLed < -1*REST_PULSE_WIDTH) {
        currentLed = NUMBER_OF_STEM_LEDS + REST_PULSE_WIDTH;
    }
    stripACurrentLed = currentLed;
    stripBCurrentLed = NUMBER_OF_STEM_LEDS - stripACurrentLed;
    runSplittingStem(&pulseA, stripACurrentLed);
    runSplittingStem(&pulseB, stripBCurrentLed);
    
    double progressPct = (double)progress / total;
    analogWrite(petalRedPin, 8*progressPct);
    analogWrite(petalGreenPin, 8*progressPct);
    analogWrite(petalBluePin, 255*(1-progressPct));
}

void runSplittingStem(PulsePlug *pulse, int16_t currentLed) {
    double progress = min(max(0, (float)currentLed/(float)NUMBER_OF_STEM_LEDS), 1.0);
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_STEM_LEDS, 
                                                pulse->role == ROLE_PRIMARY ? stemALedPin : stemBLedPin,
                                                NEO_GRB + NEO_KHZ800);
    // Serial.print(" Colors: ");
    int head = pulse->role == ROLE_PRIMARY ? 0 : (-1*REST_PULSE_WIDTH);
    int tail = pulse->role == ROLE_PRIMARY ? (REST_PULSE_WIDTH) : 1;
                                            
    for (int i=head; i < tail; i++) {
        // Serial.print((int)floor((progress)*255.0/(float)max(abs(i), 1)));
        // Serial.print("/");
        // Serial.print((int)floor(1-progress*255.0/(float)max(abs(i), 1)));
        // Serial.print(" ");
        strip.setPixelColor(currentLed+i, 0, 
                            progress*255.0, 
                            (int)floor(255.0/(float)max(abs(i*3), 1)));
    }
    // Serial.println(currentLed);
    strip.show();
}

// ========================
// = States - Stem Rising =
// ========================

bool runStemRising(PulsePlug *pulse, PulsePlug *shadowPulse) {
    unsigned long now = millis();
    unsigned long shadowLastBeat = shadowPulse->role == ROLE_PRIMARY ? lastPulseATime : lastPulseBTime;
    unsigned long millisFromLastBeat = now - shadowLastBeat;

    Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_STEM_LEDS, 
                                                pulse->role == ROLE_PRIMARY ? stemALedPin : stemBLedPin, 
                                                NEO_GRB + NEO_KHZ800);

    uint8_t pulseWidth = STEMA_PULSE_WIDTH;
    if (pulse->role == ROLE_SECONDARY) {
        pulseWidth = STEMB_PULSE_WIDTH;
    }

    int16_t currentLed = pulse->role == ROLE_PRIMARY ? stripACurrentLed : stripBCurrentLed;
    int16_t newLed = (int16_t)floor(shadowPulse->ease.easeOut(millisFromLastBeat)) - pulseWidth;
    
#ifdef USE_SERIAL
    // Serial.print(" ---> Strip: ");
    // Serial.print(pulse->role==shadowPulse->role ? "" :  "(shadowing) ");
    // Serial.print(pulse->role == ROLE_PRIMARY ? "primary" : "secondary");
    // Serial.print(" (");
    // Serial.print(millisFromLastBeat);
    // Serial.print("ms): ");
    // Serial.print(NUMBER_OF_STEM_LEDS);
    // Serial.print(" LEDs, currently at ");
    // Serial.print(currentLed);
    // Serial.print(" -> ");
    // Serial.println(newLed);
#endif
    
    if (currentLed != newLed) {
        // Reset old pixels that won't be refreshed
        for (int i = -1*pulseWidth; i < pulseWidth; i++) {
            if (currentLed + i < 0 || currentLed + i >= NUMBER_OF_STEM_LEDS) continue;
            strip.setPixelColor(currentLed + i, strip.Color(0, 0, 0));
        }
        
        currentLed = newLed;
        if (pulse->role == ROLE_PRIMARY) {
            stripACurrentLed = newLed;
        } else if (pulse->role == ROLE_SECONDARY) {
            stripBCurrentLed = newLed;
        }

        // Fade new pixels
#ifdef USE_SERIAL
        // Serial.print("LEDs (");
        // Serial.print(NUMBER_OF_STEM_LEDS);
        // Serial.print(") ");
        // Serial.print(" ---> Pulsing (width: ");
        // Serial.print(pulseWidth);
        // Serial.println(") ");
        // Serial.print(" ---> Free RAM: ");
        // Serial.println(freeRam());
#endif
        for (int i = -1*pulseWidth; i < pulseWidth; i++) {
            if ((currentLed + i < 0) || (currentLed + i >= NUMBER_OF_STEM_LEDS)) {
#ifdef USE_SERIAL
                // Serial.print(F(" ---> REJECTED in stem pulse width: "));
                // Serial.print(currentLed + i, DEC);
                // Serial.print(F(" ("));
                // Serial.print(i);
                // Serial.print(F(") / "));
                // Serial.println(NUMBER_OF_STEM_LEDS, DEC);
#endif
                continue;
            }
            uint32_t color = 0;
            uint8_t shade = (int)floor(255.0/(float)max(abs(i)-2, 1));
#ifdef USE_SERIAL
            // Serial.print(F(" ---> in stem pulse width: "));
            // Serial.print(currentLed + i);
            // Serial.print(F(" ("));
            // Serial.print(i);
            // Serial.print(F(") / "));
            // Serial.println(shade);
#endif
            if (playerMode == MODE_SINGLE_A || playerMode == MODE_SINGLE_B) {
                color = strip.Color(shade, shade, 0);
            } else if (playerMode == MODE_DOUBLE) {
                if (pulse->role == ROLE_PRIMARY) {
                    color = strip.Color(shade, shade, shade);
                } else {
                    color = strip.Color(shade, shade/3, shade/3);
                }
            }
            strip.setPixelColor(currentLed + i, color);
        }
        strip.show();
    }
    
    // At end of stem
    if (currentLed >= NUMBER_OF_STEM_LEDS) {
        return true;
    }

    return false;
}

// ==================
// = States - Petal =
// ==================

void beginPetalRising() {
    unsigned long nextBeat = 350;
    unsigned long now = millis();
    double progress = 0;

#ifdef USE_SERIAL
    unsigned long nextBeatTime = min(nextPulseATime, nextPulseBTime);
    int16_t nextBeatOffset = nextBeatTime - now;
    Serial.print(F(" ---> Led Rising, "));
    Serial.print(nextBeatOffset);
    Serial.println(F("ms until next beat"));
#endif

    if (now < endLedRiseTime) {
        Serial.print(F(" ---> Ignoring petal rise, end led rising: "));
        Serial.println(endLedRiseTime);
        // Still rising, just ignore
        return;
    } else if (now < endLedFallTime) {
        // Compensate for still falling led
        int16_t remainingFallTime = endLedFallTime - now;
        progress = ((double)PETAL_DECAY_MS - (double)remainingFallTime) / (double)PETAL_DECAY_MS;
        if (progress > 0) {
            beginLedRiseTime = now - ((1-progress)*nextBeat);
            beginLedFallTime = 0;
            endLedFallTime = 0;
        } else {
#ifdef USE_SERIAL
            Serial.println(F(" ---> *** Would've been a bump."));
#endif
        }
#ifdef USE_SERIAL
        Serial.print(F(" ---> Compensating for still falling led: "));
        Serial.print(remainingFallTime, DEC);
        Serial.print(F(" (Progress: "));
        Serial.print(progress);
        Serial.println(F(")"));
#endif
    } else {
        beginLedRiseTime = now;
    }

#ifdef USE_SERIAL
    Serial.print(F(" ---> Rise time: "));
    Serial.print(nextBeat);
    Serial.print(F(" (nextBeat: "));
    Serial.print(nextBeat);
    Serial.print(F(", begin: "));
    Serial.print(beginLedRiseTime);
    Serial.println(F(")"));
#endif
    uint16_t riseTime = nextBeat;
    endLedRiseTime = beginLedRiseTime + riseTime;
}

bool runPetalRising() {
    unsigned long now = millis();
    unsigned long millisToNextBeat = (now > endLedRiseTime) ? 0 : (endLedRiseTime - now);
    unsigned long millisFromLastBeat = now - beginLedRiseTime;
    double progress = (double)millisFromLastBeat / (double)(millisFromLastBeat + millisToNextBeat);

#ifdef USE_SERIAL
    Serial.print(F(" ---> STATE: Led Rising ("));
    Serial.print(progress);
    Serial.print(F(") from:"));
    Serial.print(millisFromLastBeat, DEC);
    Serial.print(F(" to:"));
    Serial.print(millisToNextBeat, DEC);
    Serial.print(F(" Brightness: "));
    Serial.println(max(8, min((int)floor(255 * progress), 255)), DEC);
#endif

    // Set Lotus LED brightness
    ledBrightness = max(8, min((int)floor(255 * progress), 255));
    if (playerMode == MODE_DOUBLE) {
        if (ledBrightness <= 8) {
            Serial.print(F(" ---> LED Brightness: "));
            Serial.println(ledBrightness);
        }
        analogWrite(petalRedPin, ledBrightness);
        analogWrite(petalGreenPin, ledBrightness);
        analogWrite(petalBluePin, ledBrightness);
    } else {
        analogWrite(petalRedPin, ledBrightness);
        analogWrite(petalGreenPin, ledBrightness/2);
        analogWrite(petalBluePin, 0);
    }
    if (progress >= 1.0) {
        return true;
    }

    return false;
}


void beginPetalFalling() {
#ifdef USE_SERIAL
    Serial.print(F(" ---> Led begin falling"));
#endif

    beginLedFallTime = PETAL_DELAY_MS + millis();
    // endLedFallTime = beginLedFallTime + 2*(nextBeat - beginLedFallTime);
    endLedFallTime = PETAL_DELAY_MS + beginLedFallTime + PETAL_DECAY_MS;
}

bool runPetalFalling() {
    unsigned long now = millis();
    if (now < beginLedFallTime) {
        // Petal fall is delayed, don't start fall yet.
        return false;
    }
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
    if (playerMode == MODE_DOUBLE) {
        analogWrite(petalRedPin, ledBrightness);
        analogWrite(petalGreenPin, ledBrightness);
        analogWrite(petalBluePin, ledBrightness);
    } else {
        analogWrite(petalRedPin, ledBrightness);
        analogWrite(petalGreenPin, ledBrightness/2);
        analogWrite(petalBluePin, 0);
    }

    if (progress >= 1.0) {
        return true;
    }
    
    return false;
}

void beginPetalResting() {
#ifdef USE_SERIAL
    Serial.print(F(" ---> Begin petal resting: "));
    Serial.println(restState);
#endif
    beginLedRestTime = PETAL_DELAY_MS + millis();
    endLedRestTime = PETAL_DELAY_MS + beginLedRestTime + PETAL_DECAY_MS;
    petalState = STATE_PETAL_LANDING;
}

bool runPetalResting() {
    unsigned long now = millis();
    unsigned long millisToNextBeat = (now > endLedRestTime) ? 
                                     0 : (endLedRestTime - now);
    unsigned long millisFromLastBeat = now - beginLedRestTime;
    double progress = (double)millisFromLastBeat / 
                      (double)(millisFromLastBeat + millisToNextBeat);

    if (now < beginLedRestTime) {
        // Petal land is delayed, don't start fall yet.
        return false;
    }

    
#ifdef USE_SERIAL
    Serial.print(F(" ---> Run petal resting: "));
    Serial.print(playerMode);
    Serial.print(" - ");
    Serial.print(beginLedRestTime);
    Serial.print(" - ");
    Serial.print(endLedRestTime);
    Serial.print(" - ");
    Serial.println(progress);
#endif

    analogWrite(petalRedPin, 8*(1-progress));
    analogWrite(petalGreenPin, 8*(1-progress));
    analogWrite(petalBluePin, 255*progress);

    if (progress >= 1.0) {
        return true;
    }
    
    return false;
}

// ====================
// = Serial debugging =
// ====================

void printHeader() {
    if (millis() - debuggingTimer > 1000) {
#ifdef USE_SERIAL
        Serial.print(F("------------------ "));
        Serial.print(debuggingSeconds);
        Serial.println(" ------------------");
#endif
        debuggingTimer = millis();
        debuggingSeconds += 1;
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
