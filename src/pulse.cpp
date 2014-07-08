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
#include "smooth.h"

#define USE_SERIAL
// #define PRINT_LED_VALS

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
const int SAMPLES_TO_AVERAGE = 3;

// Serial
#if defined (__AVR_ATtiny84__)
SoftwareSerial Serial(0, serialPin); // RX, TX
#endif

// Stem
const int NUMBER_OF_STEM1_LEDS = 300;
const int NUMBER_OF_STEM2_LEDS = 60;
const int8_t STEM1_PULSE_WIDTH = 32;
const int8_t STEM2_PULSE_WIDTH = 8;
volatile int strip1CurrentLed = 0;
volatile int strip2CurrentLed = 0;

// Petals
unsigned long beginLedRiseTime = 0;
unsigned long endLedRiseTime = 0;
unsigned long beginLedFallTime = 0;
unsigned long endLedFallTime = 0;

// Pulse sensor
PortI2C myBus(sensor1Pin);
PulsePlug pulse1(myBus); 
PortI2C myBus2(sensor2Pin);
PulsePlug pulse2(myBus2); 

// States
typedef enum
{
    STATE_RESTING = 0,
    STATE_STEM_RISING = 1,
    STATE_LED_RISING = 2,
    STATE_LED_FALLING = 3
} state_app_t;
state_app_t app1State;
state_app_t app2State;
state_app_t petalState;

// ============
// = Routines =
// ============

void setup(){
    // int startRam = freeRam();
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
    delay(50);
#ifdef USE_SERIAL
    Serial.begin(115200);
    Serial.flush();
    
    // Serial.print(F(" ---> Free RAM: "));
    // Serial.print(startRam);
    // Serial.print(F(" - "));
    // Serial.println(freeRam());
#endif
    printHeader();
    
    pinMode(stem1LedPin, OUTPUT);
    digitalWrite(stem1LedPin, LOW);
    pinMode(stem2LedPin, OUTPUT);
    digitalWrite(stem2LedPin, LOW);
    clearStemLeds(&pulse1);
    clearStemLeds(&pulse2);
    pulse1.role = ROLE_PRIMARY;
    pulse2.role = ROLE_SECONDARY;
    setupPulseSensor(&pulse1);
    setupPulseSensor(&pulse2);
}

void setupPulseSensor(PulsePlug *pulse) {
// #ifdef USE_SERIAL
//     if (pulse->isPresent()) {
//         Serial.println("SI1143 Pulse Sensor found OK. Let's roll!");
//     } else {
//         Serial.println("No SI1143 found!");
//     }
// #endif

    pulse->setReg(PulsePlug::HW_KEY, 0x17);  
    
    pulse->setReg(PulsePlug::INT_CFG, 0x03);       // turn on interrupts
    pulse->setReg(PulsePlug::IRQ_ENABLE, 0x10);    // turn on interrupt on PS3
    pulse->setReg(PulsePlug::IRQ_MODE2, 0x01);     // interrupt on ps3 measurement
    pulse->setReg(PulsePlug::MEAS_RATE, 0x84);     // wake up every 10ms
    pulse->setReg(PulsePlug::ALS_RATE, 0x08);      // take measurement every wakeup
    pulse->setReg(PulsePlug::PS_RATE, 0x08);       // take measurement every wakeup
    
    pulse->setReg(PulsePlug::PS_LED21, 0x39);      // LED current for 2 (IR1 - high nibble) & LEDs 1 (red - low nibble) 
    pulse->setReg(PulsePlug::PS_LED3, 0x02);       // LED current for LED 3 (IR2)
/*  debug infor for the led currents
#ifdef USE_SERIAL
    Serial.print( "PS_LED21 = ");                                         
    Serial.println(pulse->getReg(PulsePlug::PS_LED21), BIN);                                          
    Serial.print("CHLIST = ");
    Serial.println(pulse->readParam(0x01), BIN);
#endif
*/

    pulse->writeParam(PulsePlug::PARAM_CH_LIST, 0x77);         // all measurements on
    // increasing PARAM_PS_ADC_GAIN will increase the LED on time and ADC window
    // you will see increase in brightness of visible LED's, ADC output, & noise 
    // datasheet warns not to go beyond 4 because chip or LEDs may be damaged
    pulse->writeParam(PulsePlug::PARAM_PS_ADC_GAIN, 0x00);
    // You can select which LEDs are energized for each reading.
    // The settings below (in the comments)
    // turn on only the LED that "normally" would be read
    // ie LED1 is pulsed and read first, then LED2 & LED3.
    pulse->writeParam(PulsePlug::PARAM_PSLED12_SELECT, 0x21);  // 21 select LEDs 2 & 1 (red) only                                                               
    pulse->writeParam(PulsePlug::PARAM_PSLED3_SELECT, 0x04);   // 4 = LED 3 only

    // Sensors for reading the three LEDs
    // 0x03: Large IR Photodiode
    // 0x02: Visible Photodiode - cannot be read with LEDs on - just for ambient measurement
    // 0x00: Small IR Photodiode
    pulse->writeParam(PulsePlug::PARAM_PS1_ADCMUX, 0x03);      // PS1 photodiode select 
    pulse->writeParam(PulsePlug::PARAM_PS2_ADCMUX, 0x03);      // PS2 photodiode select 
    pulse->writeParam(PulsePlug::PARAM_PS3_ADCMUX, 0x03);      // PS3 photodiode select  

    pulse->writeParam(PulsePlug::PARAM_PS_ADC_COUNTER, B01110000);    // B01110000 is default                                   
    pulse->setReg(PulsePlug::COMMAND, PulsePlug::PSALS_AUTO_Cmd);     // starts an autonomous read loop
}

void loop() {
    // printHeader();
    int sensor1On = readPulseSensor(&pulse1);
    int sensor2On = readPulseSensor(&pulse2);
    
    if (sensor1On > 0) {
        app1State = STATE_STEM_RISING;
        newHeartbeat(&pulse1);
    } else if (app1State == STATE_STEM_RISING) {
        bool stem1Done = runStemRising(&pulse1);
        if (stem1Done) {
            app1State = STATE_LED_RISING;
            petalState = STATE_LED_RISING;
            beginLedRising(&pulse1);
        }
    } 
    
    if (sensor2On > 0) {
        app2State = STATE_STEM_RISING;
        newHeartbeat(&pulse2);
    } else if (app2State == STATE_STEM_RISING) {
        bool stem2Done = runStemRising(&pulse2);
        if (stem2Done) {
            app2State = STATE_LED_RISING;
            petalState = STATE_LED_RISING;
            beginLedRising(&pulse2);
        }
    } 
    
    if (app1State == STATE_LED_RISING || petalState == STATE_LED_RISING) {
        bool ledDone = runLedRising(&pulse1);
        if (ledDone) {
            if (app1State == STATE_LED_RISING) app1State = STATE_LED_FALLING;
            petalState = STATE_LED_FALLING;
            beginLedFalling(&pulse1);
        }
    } else if (app1State == STATE_LED_FALLING || petalState == STATE_LED_FALLING) {
        bool ledDone = runLedFalling(&pulse1);
        if (ledDone) {
            if (app1State == STATE_LED_FALLING) app1State = STATE_RESTING;
            petalState = STATE_RESTING;
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

void beginStateOn(PulsePlug *pulse) {
    
}

void clearStemLeds(PulsePlug *pulse) {
    // Reset stem LEDs
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

    ease.setDuration(millisToNextBeat);
    ease.setTotalChangeInPosition(ledCount);
}

bool runStemRising(PulsePlug *pulse) {
    unsigned long now = millis();
    int8_t bpm = max(min(pulse->latestBpm, 100), 45);
    unsigned long nextBeat = pulse->lastBeat + (int)floor(60000.0/(bpm/.5));
    unsigned long millisToNextBeat = (now > nextBeat) ? 0 : (nextBeat - now);
    unsigned long millisFromLastBeat = now - pulse->lastBeat;
    double progress = (double)millisFromLastBeat / 
                      (double)(millisFromLastBeat + millisToNextBeat);
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(pulse->role == ROLE_PRIMARY ? NUMBER_OF_STEM1_LEDS : NUMBER_OF_STEM2_LEDS, 
                                             pulse->role == ROLE_PRIMARY ? stem1LedPin : stem2LedPin, 
                                             NEO_GRB + NEO_KHZ800);
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
    int newLed = (int)floor(ease.easeIn(millisFromLastBeat));

#ifdef USE_SERIAL
    // Serial.print("LEDs (");
    // Serial.print(ledCount);
    // Serial.print(") ");
    // Serial.print(currentLed);
    // Serial.print(" -> ");
    // Serial.println(newLed);
#endif
    
    if (currentLed != newLed) {
        uint8_t pulseWidth;
        if (pulse->role == ROLE_PRIMARY) {
            pulseWidth = STEM1_PULSE_WIDTH;
        } else if (pulse->role == ROLE_SECONDARY) {
            pulseWidth = STEM2_PULSE_WIDTH;
        }
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
            // Serial.println((int)floor(255.0/(abs(i)+1)));
#endif
            uint32_t color;
            if (abs(i) <= 2) {
                color = strip.Color(255, 0, 0);
            } else {
                color = strip.Color((int)floor(255.0/(abs(i)-1)), 0, 0);
            }
            strip.setPixelColor(currentLed + i, color);
        }
        strip.show();
    }
    
    // At end of stem
    if (currentLed == ledCount || progress >= 1.0) {
        currentLed = 0;
        if (pulse->role == ROLE_PRIMARY) {
            strip1CurrentLed = 0;
        } else if (pulse->role == ROLE_SECONDARY) {
            strip2CurrentLed = 0;
        }
        for (int i=0; i < ledCount; i++) {
            strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        return true;
    }

    return false;
}

void beginLedRising(PulsePlug *pulse) {
    int bpm = max(min(pulse->latestBpm, 100), 45);
    unsigned long nextBeat = pulse->lastBeat + 60000.0/bpm;
    unsigned long now = millis();

#ifdef USE_SERIAL
    // Serial.print(F(" ---> Led Rising: "));
    // Serial.println(nextBeat);    
#endif

    beginLedRiseTime = now;

    if (now < endLedFallTime) {
        // Compensate for still falling led
        unsigned int remainingFallTime = endLedFallTime - now;
#ifdef USE_SERIAL
        // Serial.print(F(" Compensating for still falling led: "));
        // Serial.println(remainingFallTime, DEC);
#endif
        beginLedRiseTime = beginLedRiseTime - (int)floor((400.0*((double)remainingFallTime/800.0)));
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
    // Serial.print(F(" ---> Led Falling: "));
    // Serial.println(nextBeat);    
#endif

    beginLedFallTime = millis();
    // endLedFallTime = beginLedFallTime + 2*(nextBeat - beginLedFallTime);
    endLedFallTime = beginLedFallTime + 800;
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

// ===========
// = Sensors =
// ===========


int readPulseSensor(PulsePlug *pulse) {
    if (!pulse->valleyTime) pulse->valleyTime = millis();
    if (!pulse->lastValleyTime) pulse->lastValleyTime = millis();
    if (!pulse->peakTime) pulse->peakTime = millis();
    if (!pulse->lastPeakTime) pulse->lastPeakTime = millis();
    
    unsigned long total=0;
    int i=0;
    int IR_signalSize;
    pulse->red = 0;
    pulse->IR1 = 0;
    pulse->IR2 = 0;
    total = 0;
    
    while (i < SAMPLES_TO_AVERAGE){      
        pulse->fetchLedData();
        pulse->red += pulse->ps1;
        pulse->IR1 += pulse->ps2;
        pulse->IR2 += pulse->ps3;
        i++;
    }
    
    pulse->red = pulse->red / i;  // get averages
    pulse->IR1 = pulse->IR1 / i;
    pulse->IR2 = pulse->IR2 / i;
    total =  pulse->IR1 + pulse->IR2 + pulse->red;  // red excluded
    pulse->IRtotal = pulse->IR1 + pulse->IR2;
    
    if (pulse->red == 0 && pulse->IR1 == 0 && pulse->IR2 == 0) {
        delay(500);
#ifdef USE_SERIAL
        Serial.println(" ---> Resetting to fix Pulse Sensor");
#endif
        resetArduino();
    }

#ifdef PRINT_LED_VALS
    Serial.print(pulse->red);
    Serial.print(F("\t"));
    Serial.print(pulse->IR1);
    Serial.print(F("\t"));
    Serial.print(pulse->IR2);
    Serial.print(F("\t"));
    Serial.println((long)total);   
#endif

    if (pulse->lastTotal < 20000L && total > 20000L) pulse->foundNewFinger = 1;  // found new finger!

    pulse->lastTotal = total;
    
    // if found a new finger prime filters first 20 times through the loop
    if (++(pulse->foundNewFinger) > 25) pulse->foundNewFinger = 25;   // prevent rollover 
    
    if (pulse->foundNewFinger < 20) {
        pulse->IR_baseline = total - 200;   // take a guess at the baseline to prime smooth filter
#ifdef USE_SERIAL
        if (pulse->foundNewFinger == 2) {
            Serial.print(F(" ---> Found new finger - "));
            if (pulse->role == ROLE_PRIMARY) {
                Serial.println(F("primary"));
            } else if (pulse->role == ROLE_SECONDARY) {
                Serial.println(F("secondary"));
            }
        }
#endif
    } else if (total > 20000L) {    // main running function
        // baseline is the moving average of the signal - the middle of the waveform
        // the idea here is to keep track of a high frequency signal, HFoutput and a 
        // low frequency signal, LFoutput
        // The LF signal is shifted downward slightly downward (heartbeats are negative peaks)
        // The high freq signal has some hysterisis added. 
        // When the HF signal crosses the shifted LF signal (on a downward slope), 
        // we have found a heartbeat.
        pulse->IR_baseline = smooth(pulse->IRtotal, 0.99, pulse->IR_baseline);
        pulse->IR_HFoutput = smooth((pulse->IRtotal - pulse->IR_baseline), 0.2, pulse->IR_HFoutput);    // recycling output - filter to slow down response
        
        pulse->red_baseline = smooth(pulse->red, 0.99, pulse->red_baseline); 
        pulse->red_HFoutput = smooth((pulse->red - pulse->red_HFoutput), 0.2, pulse->red_HFoutput);
        
        // beat detection is performed only on the IR channel so 
        // fewer red variables are needed
        pulse->IR_HFoutput2 = pulse->IR_HFoutput + pulse->hysterisis;     
        pulse->LFoutput = smooth((pulse->IRtotal - pulse->IR_baseline), 0.95, pulse->LFoutput);
        // heartbeat signal is inverted - we are looking for negative peaks
        pulse->shiftedOutput = pulse->LFoutput - (IR_signalSize * .05);

        if (!pulse->IR_peak || pulse->IR_HFoutput > pulse->IR_peak) {
            pulse->IR_peak = pulse->IR_HFoutput; 
        }
        if (pulse->red_HFoutput > pulse->red_Peak) pulse->red_Peak = pulse->red_HFoutput;
        
        // default reset - only if reset fails to occur for 1800 ms
        if (millis() - pulse->lastPeakTime > 1800) {  // reset peak detector slower than lowest human HB
#ifdef USE_SERIAL
            Serial.println(" ---> Reseting peak detector, took too long");
#endif
            pulse->IR_smoothPeak = smooth((float)pulse->IR_peak, 0.6, (float)pulse->IR_smoothPeak);  // smooth peaks
            pulse->IR_peak = 0;
            
            pulse->red_smoothPeak = smooth((float)pulse->red_Peak, 0.6, (float)pulse->red_smoothPeak);  // smooth peaks
            pulse->red_Peak = 0;
            
            pulse->lastPeakTime = millis();
        }

        if (!pulse->IR_valley || pulse->IR_HFoutput < pulse->IR_valley) pulse->IR_valley = pulse->IR_HFoutput;
        if (pulse->red_HFoutput < pulse->red_valley) pulse->red_valley = pulse->red_HFoutput;
        
        if (millis() - pulse->lastValleyTime > 1800) {  // insure reset slower than lowest human HB
#ifdef USE_SERIAL
            Serial.println(" ---> Reseting valley detector, took too long");
#endif
            pulse->IR_smoothValley =  smooth((float)pulse->IR_valley, 0.6, (float)pulse->IR_smoothValley);  // smooth valleys
            pulse->IR_valley = 0;
            pulse->lastValleyTime = millis();           
        }

        pulse->hysterisis = constrain((IR_signalSize / 15), 35, 120) ;  // you might want to divide by smaller number
                                                                // if you start getting "double bumps"
            
        if  (pulse->IR_HFoutput2 < pulse->shiftedOutput) {
            // found a beat - pulses are valleys
            pulse->lastBinOut = pulse->binOut;
            pulse->binOut = 1;
            pulse->hysterisis = -pulse->hysterisis;
            pulse->IR_smoothValley =  smooth((float)pulse->IR_valley, 0.99, (float)pulse->IR_smoothValley);  // smooth valleys
            IR_signalSize = pulse->IR_smoothPeak - pulse->IR_smoothValley;
            pulse->IR_valley = 0x7FFF;
            
            pulse->red_smoothValley =  smooth((float)pulse->red_valley, 0.99, (float)pulse->red_smoothValley);  // smooth valleys
            pulse->red_signalSize = pulse->red_smoothPeak - pulse->red_smoothValley;
            pulse->red_valley = 0x7FFF;
            
            pulse->lastValleyTime = millis();
        } else {
            pulse->lastBinOut = pulse->binOut;
            pulse->binOut = 0;
            pulse->IR_smoothPeak =  smooth((float)pulse->IR_peak, 0.99, (float)pulse->IR_smoothPeak);  // smooth peaks
            pulse->IR_peak = 0;
            
            pulse->red_smoothPeak =  smooth((float)pulse->red_Peak, 0.99, (float)pulse->red_smoothPeak);  // smooth peaks
            pulse->red_Peak = 0;
            pulse->lastPeakTime = millis();
        } 

        if (pulse->lastBinOut == 1 && pulse->binOut == 0) {
#ifdef USE_SERIAL
            // Serial.println(" ---> Heartbeat finished");
#endif
            return -1;
        }

        if (pulse->lastBinOut == 0 && pulse->binOut == 1) {
            pulse->previousBeat = pulse->lastBeat;
            pulse->lastBeat = millis();
            pulse->latestBpm = 60000 / (pulse->lastBeat - pulse->previousBeat);
#ifdef USE_SERIAL
            Serial.print(F(" ---> Heartbeat started on "));
            Serial.print(pulse->role);
            Serial.print(F(": "));
            Serial.print(F("\t BPM "));
            Serial.print(pulse->latestBpm);  
            Serial.print(F("\t IR "));
            Serial.print(IR_signalSize);
            Serial.print(F("\t PSO2 "));         
            Serial.println(((float)pulse->red_baseline / (float)(pulse->IR_baseline/2)), 3);                     
#endif

            return 1;
        }

    }
    
    return 0;
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
        // blink(3, 25, true);
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

void resetArduino() {
  asm volatile ("  jmp 0");  
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
