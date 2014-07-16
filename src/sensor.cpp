#include <si1143/si1143.h>
#include "smooth.h"
#include "sensor.h"

#define USE_SERIAL
// #define PRINT_LED_VALS

const int SAMPLES_TO_AVERAGE = 3;

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
/*  debug infor for the led currents */
    // Serial.print( "PS_LED21 = ");                                         
    // Serial.println(pulse->getReg(PulsePlug::PS_LED21), BIN);                                          
    // Serial.print("CHLIST = ");
    // Serial.println(pulse->readParam(0x01), BIN);

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
        delay(100);
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
    } else if (total < 5000L) { // finger's gone
// #ifdef USE_SERIAL
//         Serial.print(F(" ---> Finger's gone - "));
//             if (pulse->role == ROLE_PRIMARY) {
//                 Serial.println(F("primary"));
//             } else if (pulse->role == ROLE_SECONDARY) {
//                 Serial.println(F("secondary"));
//             }
// #endif
        return -1;
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
            return 0;
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

void resetArduino() {
  asm volatile ("  jmp 0");  
}
