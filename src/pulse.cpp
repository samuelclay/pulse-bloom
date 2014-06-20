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
#include <si1143/si1143.h>

#include "pulse.h"
#include "smooth.h"

#define USE_SERIAL
// #define PRINT_LED_VALS

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
const int SI114Pin = 0; // SCL=18, SDA=19
#endif

// ===========
// = Globals =
// ===========

PortI2C myBus(SI114Pin);
PulsePlug pulse(myBus); 
long timer = 0;
long loops = 0;
int numPixels;
uint16_t ledBrightness = 0;
const int SAMPLES_TO_AVERAGE = 5;             // samples for smoothing 1 to 10 seem useful 5 is default

#if defined (__AVR_ATtiny84__)
SoftwareSerial Serial(0, serialPin); // RX, TX
#endif
Adafruit_NeoPixel strip = Adafruit_NeoPixel(300, leftRealLedPin, NEO_GRB + NEO_KHZ800);

int binOut;             // 1 or 0 depending on state of heartbeat
int BPM;                
unsigned long red;      // read value from visible red LED
unsigned long IR1;      // read value from infrared LED1
unsigned long IR2;      // read value from infrared LED2
unsigned long IR_total; // all three LED reads added together

typedef enum
{
    STATE_RESTING = 0,
    STATE_STEM_RISING = 1,
    STATE_LED_RISING = 2,
    STATE_LED_FALLING = 3
} state_t;
state_t appState;

// ============
// = Routines =
// ============

void setup(){
    analogReference(EXTERNAL);
    pinMode(leftLedPin, OUTPUT);
    analogWrite(leftLedPin, ledBrightness);
    appState = STATE_RESTING;
#ifdef USE_SERIAL
    Serial.begin(115200);
    Serial.flush();
#endif
    printHeader();
    setupPulseSensor();
    
    strip.begin();
    for (int i=0; i < numPixels; i++) {
        strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show(); // Initialize all pixels to 'off'
    numPixels = strip.numPixels();
}

void setupPulseSensor() {
    pulse.setReg(PulsePlug::HW_KEY, 0x17);  
    
    pulse.setReg(PulsePlug::INT_CFG, 0x03);       // turn on interrupts
    pulse.setReg(PulsePlug::IRQ_ENABLE, 0x10);    // turn on interrupt on PS3
    pulse.setReg(PulsePlug::IRQ_MODE2, 0x01);     // interrupt on ps3 measurement
    pulse.setReg(PulsePlug::MEAS_RATE, 0x84);     // wake up every 10ms
    pulse.setReg(PulsePlug::ALS_RATE, 0x08);      // take measurement every wakeup
    pulse.setReg(PulsePlug::PS_RATE, 0x08);       // take measurement every wakeup
    
    pulse.setReg(PulsePlug::PS_LED21, 0x39);      // LED current for 2 (IR1 - high nibble) & LEDs 1 (red - low nibble) 
    pulse.setReg(PulsePlug::PS_LED3, 0x02);       // LED current for LED 3 (IR2)
/*  debug infor for the led currents
    Serial.print( "PS_LED21 = ");                                         
    Serial.println(pulse.getReg(PulsePlug::PS_LED21), BIN);                                          
    Serial.print("CHLIST = ");
    Serial.println(pulse.readParam(0x01), BIN);
*/

    pulse.writeParam(PulsePlug::PARAM_CH_LIST, 0x77);         // all measurements on
    // increasing PARAM_PS_ADC_GAIN will increase the LED on time and ADC window
    // you will see increase in brightness of visible LED's, ADC output, & noise 
    // datasheet warns not to go beyond 4 because chip or LEDs may be damaged
    pulse.writeParam(PulsePlug::PARAM_PS_ADC_GAIN, 0x00);
    // You can select which LEDs are energized for each reading.
    // The settings below (in the comments)
    // turn on only the LED that "normally" would be read
    // ie LED1 is pulsed and read first, then LED2 & LED3.
    pulse.writeParam(PulsePlug::PARAM_PSLED12_SELECT, 0x21);  // 21 select LEDs 2 & 1 (red) only                                                               
    pulse.writeParam(PulsePlug::PARAM_PSLED3_SELECT, 0x04);   // 4 = LED 3 only

    // Sensors for reading the three LEDs
    // 0x03: Large IR Photodiode
    // 0x02: Visible Photodiode - cannot be read with LEDs on - just for ambient measurement
    // 0x00: Small IR Photodiode
    pulse.writeParam(PulsePlug::PARAM_PS1_ADCMUX, 0x03);      // PS1 photodiode select 
    pulse.writeParam(PulsePlug::PARAM_PS2_ADCMUX, 0x03);      // PS2 photodiode select 
    pulse.writeParam(PulsePlug::PARAM_PS3_ADCMUX, 0x03);      // PS3 photodiode select  

    pulse.writeParam(PulsePlug::PARAM_PS_ADC_COUNTER, B01110000);    // B01110000 is default                                   
    pulse.setReg(PulsePlug::COMMAND, PulsePlug::PSALS_AUTO_Cmd);     // starts an autonomous read loop
}

void loop() {
    printHeader();
    readPulseSensor();
    
    return;
    
    if (appState == STATE_RESTING) {
        if (appState != STATE_STEM_RISING) {
            for (int i=0; i < numPixels; i++) {
                strip.setPixelColor(i, 0, 0, 0);
            }
            strip.show();
            appState = STATE_STEM_RISING;
        }
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
            for (int i=0; i < numPixels; i++) {
                strip.setPixelColor(i, 0, 0, 0);
            }
            strip.show();
        }
    } else if (appState == STATE_LED_RISING) {
                   
    } else if (appState == STATE_LED_RISING) {
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
}

// ==========
// = States =
// ==========

void beginStateOn() {
    
}

void beginStemRising() {
    
}

void beginLedRising() {
    
}

void beginLedFalling() {
    
}

// ===========
// = Sensors =
// ===========


void readPulseSensor() {
    static int foundNewFinger, red_signalSize, red_smoothValley;
    static long red_valley, red_Peak, red_smoothRedPeak, red_smoothRedValley, 
               red_HFoutput, red_smoothPeak; // for PSO2 calc
    static  int IR_valley=0, IR_peak=0, IR_smoothPeak, IR_smoothValley, binOut, lastBinOut, BPM;
    static unsigned long lastTotal, lastMillis, IRtotal, valleyTime, lastValleyTime, peakTime, lastPeakTime, lastBeat, beat;
    static float IR_baseline, red_baseline, IR_HFoutput, IR_HFoutput2, shiftedOutput, LFoutput, hysterisis;
    
    if (!valleyTime) valleyTime = millis();
    if (!lastValleyTime) lastValleyTime = millis();
    if (!peakTime) peakTime = millis();
    if (!lastPeakTime) lastPeakTime = millis();
    
    unsigned long total=0, start;
    int i=0;
    int IR_signalSize;
    red = 0;
    IR1 = 0;
    IR2 = 0;
    total = 0;
    start = millis();
    
    while (i < SAMPLES_TO_AVERAGE){      
        pulse.fetchLedData();
        red += pulse.ps1;
        IR1 += pulse.ps2;
        IR2 += pulse.ps3;
        i++;
    }
    
    red = red / i;  // get averages
    IR1 = IR1 / i;
    IR2 = IR2 / i;
    total =  IR1 + IR2 + red;  // red excluded
    IRtotal = IR1 + IR2;

#ifdef PRINT_LED_VALS

    Serial.print(red);
    Serial.print("\t");
    Serial.print(IR1);
    Serial.print("\t");
    Serial.print(IR2);
    Serial.print("\t");
    Serial.println((long)total);   

#endif

    if (lastTotal < 20000L && total > 20000L) foundNewFinger = 1;  // found new finger!

    lastTotal = total;
     
    // if found a new finger prime filters first 20 times through the loop
    if (++foundNewFinger > 25) foundNewFinger = 25;   // prevent rollover 
    
    if (foundNewFinger < 20) {
        IR_baseline = total - 200;   // take a guess at the baseline to prime smooth filter
        Serial.println("found new finger");     
    } else if (total > 20000L) {    // main running function
        // baseline is the moving average of the signal - the middle of the waveform
        // the idea here is to keep track of a high frequency signal, HFoutput and a 
        // low frequency signal, LFoutput
        // The LF signal is shifted downward slightly downward (heartbeats are negative peaks)
        // The high freq signal has some hysterisis added. 
        // When the HF signal crosses the shifted LF signal (on a downward slope), 
        // we have found a heartbeat.
        IR_baseline = smooth(IRtotal, 0.99, IR_baseline);
        IR_HFoutput = smooth((IRtotal - IR_baseline), 0.2, IR_HFoutput);    // recycling output - filter to slow down response
        
        red_baseline = smooth(red, 0.99, red_baseline); 
        red_HFoutput = smooth((red - red_HFoutput), 0.2, red_HFoutput);
        
        // beat detection is performed only on the IR channel so 
        // fewer red variables are needed
        IR_HFoutput2 = IR_HFoutput + hysterisis;     
        LFoutput = smooth((IRtotal - IR_baseline), 0.95, LFoutput);
        // heartbeat signal is inverted - we are looking for negative peaks
        shiftedOutput = LFoutput - (IR_signalSize * .05);

        if (IR_HFoutput > IR_peak) IR_peak = IR_HFoutput; 
        if (red_HFoutput > red_Peak) red_Peak = red_HFoutput;
        
        // default reset - only if reset fails to occur for 1800 ms
        if (millis() - lastPeakTime > 1800) {  // reset peak detector slower than lowest human HB
            IR_smoothPeak = smooth((float)IR_peak, 0.6, (float)IR_smoothPeak);  // smooth peaks
            IR_peak = 0;
            
            red_smoothPeak = smooth((float)red_Peak, 0.6, (float)red_smoothPeak);  // smooth peaks
            red_Peak = 0;
            
            lastPeakTime = millis();
        }

        if (IR_HFoutput  < IR_valley)   IR_valley = IR_HFoutput;
        if (red_HFoutput  < red_valley)   red_valley = red_HFoutput;
        
        if (millis() - lastValleyTime > 1800) {  // insure reset slower than lowest human HB
            IR_smoothValley =  smooth((float)IR_valley, 0.6, (float)IR_smoothValley);  // smooth valleys
            IR_valley = 0;
            lastValleyTime = millis();           
        }

        hysterisis = constrain((IR_signalSize / 15), 35, 120) ;  // you might want to divide by smaller number
                                                                // if you start getting "double bumps"
            
        if  (IR_HFoutput2 < shiftedOutput) {
            // found a beat - pulses are valleys
            lastBinOut = binOut;
            binOut = 1;
            hysterisis = -hysterisis;
            IR_smoothValley =  smooth((float)IR_valley, 0.99, (float)IR_smoothValley);  // smooth valleys
            IR_signalSize = IR_smoothPeak - IR_smoothValley;
            IR_valley = 0x7FFF;
            
            red_smoothValley =  smooth((float)red_valley, 0.99, (float)red_smoothValley);  // smooth valleys
            red_signalSize = red_smoothPeak - red_smoothValley;
            red_valley = 0x7FFF;
            
            lastValleyTime = millis();
        } else {
            lastBinOut = binOut;
            binOut = 0;
            IR_smoothPeak =  smooth((float)IR_peak, 0.99, (float)IR_smoothPeak);  // smooth peaks
            IR_peak = 0;
            
            red_smoothPeak =  smooth((float)red_Peak, 0.99, (float)red_smoothPeak);  // smooth peaks
            red_Peak = 0;
            lastPeakTime = millis();
        } 

        if (lastBinOut == 1 && binOut == 0) {
            Serial.println(binOut);
        }

        if (lastBinOut == 0 && binOut == 1) {
            lastBeat = beat;
            beat = millis();
            BPM = 60000 / (beat - lastBeat);
            Serial.print(binOut);
            Serial.print("\t BPM ");
            Serial.print(BPM);  
            Serial.print("\t IR ");
            Serial.print(IR_signalSize);
            Serial.print("\t PSO2 ");         
            Serial.println(((float)red_baseline / (float)(IR_baseline/2)), 3);                     
        }

    }
}

// ====================
// = Serial debugging =
// ====================

void printHeader() {
    if (millis() - timer > 1000) {
#ifdef USE_SERIAL
        Serial.print("------------------ ");
        Serial.print(loops);
        Serial.println(" ------------------");
#endif
        blink(3, 25, true);
        timer = millis();
        loops += 1;
    }
}

void blink(int loops, int loopTime, bool half) {
    while (loops--) {
        digitalWrite(leftLedPin, HIGH);
        delay(loopTime);
        digitalWrite(leftLedPin, LOW);
        delay(loopTime / (half ? 2 : 1));
    }
}
