// SI114.h
// Code for the Modern Device Pulse Sensor
// Based on the SI1143 chip
// Also includes subset of JeeLabs Ports library - thanks JCW!
// paul@moderndevice.com 6-27-2012
// 2009-02-13 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#ifndef SI114_h
#define SI114_h

#if ARDUINO>=100
#include <Arduino.h> // Arduino 1.0
#else
#include <Wprogram.h> // Arduino 0022
#endif

#include <stdint.h>
#include <avr/pgmspace.h>
#include <easing/QuadraticEase.h>
//#include <util/delay.h>

typedef enum
{
    ROLE_PRIMARY = 0,
    ROLE_SECONDARY = 1
} role_t;

class Port {
protected:
    uint8_t portNum;

#if defined(__AVR_ATtiny85__)
    inline uint8_t digiPin() const
        { return 0; }
    inline uint8_t digiPin2() const
        { return 2; }
    static uint8_t digiPin3()
        { return 1; }
    inline uint8_t anaPin() const
        { return 0; }
#else
    inline uint8_t digiPin() const
        { return portNum ? portNum : 18; }
    inline uint8_t digiPin2() const
        { return portNum ? portNum + 1 : 19; }
    static uint8_t digiPin3()
        { return 3; }
    inline uint8_t anaPin() const
        { return portNum - 1; }
#endif

public:
    inline Port (uint8_t num) : portNum (num) {}

    // DIO pin
    inline void mode(uint8_t value) const
        { pinMode(digiPin(), value); }
    inline uint8_t digiRead() const
        { return digitalRead(digiPin()); }
    inline void digiWrite(uint8_t value) const
        { return digitalWrite(digiPin(), value); }
    inline void anaWrite(uint8_t val) const
        { analogWrite(digiPin(), val); }
    inline uint32_t pulse(uint8_t state, uint32_t timeout =1000000L) const
        { return pulseIn(digiPin(), state, timeout); }
    
    // AIO pin
    inline void mode2(uint8_t value) const
        { pinMode(digiPin2(), value); }
    inline uint16_t anaRead() const
        { return analogRead(anaPin()); }        
    inline uint8_t digiRead2() const
        { return digitalRead(digiPin2()); }
    inline void digiWrite2(uint8_t value) const
        { return digitalWrite(digiPin2(), value); }
    inline uint32_t pulse2(uint8_t state, uint32_t timeout =1000000L) const
        { return pulseIn(digiPin2(), state, timeout); }
        
    // IRQ pin (INT1, shared across all ports)
    static void mode3(uint8_t value)
        { pinMode(digiPin3(), value); }
    static uint8_t digiRead3()
        { return digitalRead(digiPin3()); }
    static void digiWrite3(uint8_t value)
        { return digitalWrite(digiPin3(), value); }
    static void anaWrite3(uint8_t val)
        { analogWrite(digiPin3(), val); }
        
    // both pins: data on DIO, clock on AIO
    inline void shift(uint8_t bitOrder, uint8_t value) const
        { shiftOut(digiPin(), digiPin2(), bitOrder, value); }
    uint16_t shiftRead(uint8_t bitOrder, uint8_t count =8) const;
    void shiftWrite(uint8_t bitOrder, uint16_t value, uint8_t count =8) const;
};




class PortI2C : public Port {
    uint8_t uswait;
#if 0
// speed test with fast hard-coded version for Port 1:
    inline void hold() const
        { _delay_us(1); }
    inline void sdaOut(uint8_t value) const
        { bitWrite(DDRD, 4, !value); bitWrite(PORTD, 4, value); }
    inline uint8_t sdaIn() const
        { return bitRead(PORTD, 4); }
    inline void sclHi() const
        { hold(); bitWrite(PORTC, 0, 1); }
    inline void sclLo() const
        { hold(); bitWrite(PORTC, 0, 0); }
public:
    enum { KHZMAX, KHZ400, KHZ100, KHZ_SLOW };
#else
    inline void hold() const
        { delayMicroseconds(uswait); }
    inline void sdaOut(uint8_t value) const
        { mode(!value); digiWrite(value); }
    inline uint8_t sdaIn() const
        { return digiRead(); }
    inline void sclHi() const
        { hold(); digiWrite2(1); }
    inline void sclLo() const
        { hold(); digiWrite2(0); }
public:
    enum { KHZMAX = 1, KHZ400 = 2, KHZ100 = 9 };
#endif
    
    PortI2C (uint8_t num, uint8_t rate =KHZMAX);
    
    uint8_t start(uint8_t addr) const;
    void stop() const;
    uint8_t write(uint8_t data) const;
    uint8_t read(uint8_t last) const;
};

class DeviceI2C {
    const PortI2C& port;
    uint8_t addr;
    
public:
    DeviceI2C(const PortI2C& p, uint8_t me) : port (p), addr (me << 1) {}
    
    bool isPresent() const;
    
    uint8_t send() const
        { return port.start(addr); }
    uint8_t receive() const
        { return port.start(addr | 1); }
    void stop() const
        { port.stop(); }
    uint8_t write(uint8_t data) const
        { return port.write(data); }
    uint8_t read(uint8_t last) const
        { return port.read(last); }
        
    void setAddress(uint8_t me)
        { addr = me << 1; }
};

// The millisecond timer can be used for timeouts up to 60000 milliseconds.
// Setting the timeout to zero disables the timer.
//
// for periodic timeouts, poll the timer object with "if (timer.poll(123)) ..."
// for one-shot timeouts, call "timer.set(123)" and poll as "if (timer.poll())"

class MilliTimer {
    word next;
    byte armed;
public:
    MilliTimer () : armed (0) {}
    
    byte poll(word ms =0);
    word remaining() const;
    byte idle() const { return !armed; }
    void set(word ms);
};

// Low-power utility code using the Watchdog Timer (WDT). Requires a WDT interrupt handler, e.g.
// EMPTY_INTERRUPT(WDT_vect);

// simple task scheduler for times up to 6000 seconds
class Scheduler {
    word* tasks;
    byte maxTasks;
    word remaining;
    MilliTimer ms100;
public:
    // initialize for a specified maximum number of tasks
    Scheduler (byte max);
    Scheduler (word* buf, byte max);

    // return next task to run, -1 if there are none ready to run, but there are tasks waiting, or -2 if there are no tasks waiting (i.e. all are idle)
    char poll();
    // same as poll, but wait for event in power-down mode.
    // Uses Sleepy::loseSomeTime() - see comments there re requiring the watchdog timer. 
    char pollWaiting();
    
    // set a task timer, in tenths of seconds
    void timer(byte task, word tenths);
    // cancel a task timer
    void cancel(byte task);
    
    // return true if a task timer is not running
    byte idle(byte task) { return tasks[task] == (unsigned)~0; }
};


class PulsePlug : public DeviceI2C {
public:
    enum {     // register values
        /* 0x00 */        PART_ID, REV_ID, SEQ_ID, INT_CFG,
        /* 0x04 */        IRQ_ENABLE, IRQ_MODE1, IRQ_MODE2, HW_KEY,
        /* 0x08 */        MEAS_RATE, ALS_RATE, PS_RATE, ALS_LOW_TH,
        /* 0x0C */        RESERVED_0C, ALS_HI_TH, RESERVED_0E, PS_LED21,
        /* 0x10 */        PS_LED3, PS1_TH, RESERVED_12, PS2_TH,
        /* 0x14 */        RESERVED_14, PS3_TH, RESERVED_16, PARAM_WR,
        /* 0x18 */        COMMAND, /* gap: 0x19..0x1F */ RESERVED_1F = 0x1F,
        /* 0x20 */        RESPONSE, IRQ_STATUS, ALS_VIS_DATA0, ALS_VIS_DATA1,
        /* 0x24 */        ALS_IR_DATA0, ALS_IR_DATA1, PS1_DATA0, PS1_DATA1,
        /* 0x28 */        PS2_DATA0, PS2_DATA1, PS3_DATA0, PS3_DATA1,
        /* 0x2C */        AUX_DATA0, AUX_DATA1, PARAM_RD, RESERVED_2F,
        /* 0x30 */        CHIP_STAT, /* gap: 0x31..0x3A */ RESERVED_3A = 0x3A,
        /* 0x3B */        ANA_IN_KEY1, ANA_IN_KEY2, ANA_IN_KEY3, ANA_IN_KEY4,
    };

    enum {       // Parmeter RAM values
        // Parameter Offsets
        PARAM_I2C_ADDR     =       0x00,
        PARAM_CH_LIST      =       0x01,
        PARAM_PSLED12_SELECT  =    0x02,
        PARAM_PSLED3_SELECT   =    0x03,
        PARAM_FILTER_EN       =    0x04,
        PARAM_PS_ENCODING     =    0x05,
        PARAM_ALS_ENCODING    =    0x06,
        PARAM_PS1_ADCMUX      =    0x07,
        PARAM_PS2_ADCMUX      =    0x08,
        PARAM_PS3_ADCMUX      =    0x09,
        PARAM_PS_ADC_COUNTER  =    0x0A,
        PARAM_PS_ADC_CLKDIV   =    0x0B,
        PARAM_PS_ADC_GAIN     =    0x0B,
        PARAM_PS_ADC_MISC     =    0x0C,
        PARAM_ALS1_ADC_MUX    =    0x0D,
        PARAM_ALS2_ADC_MUX    =    0x0E,
        PARAM_ALS3_ADC_MUX    =    0x0F,
        PARAM_ALSVIS_ADC_COUNTER = 0x10,
        PARAM_ALSVIS_ADC_CLKDIV =  0x11,
        PARAM_ALSVIS_ADC_GAIN   =  0x11,
        PARAM_ALSVIS_ADC_MISC   =  0x12,
        PARAM_ALS_HYST          =  0x16,
        PARAM_PS_HYST           =  0x17,
        PARAM_PS_HISTORY        =  0x18,
        PARAM_ALS_HISTORY       =  0x19,
        PARAM_ADC_OFFSET        =  0x1A,
        PARAM_SLEEP_CTRL        =  0x1B,
        PARAM_LED_RECOVERY      =  0x1C,
        PARAM_ALSIR_ADC_COUNTER =  0x1D,
        PARAM_ALSIR_ADC_CLKDIV  =  0x1E,
        PARAM_ALSIR_ADC_GAIN    =  0x1E,
        PARAM_ALSIR_ADC_MISC    =  0x1F
    };  


    enum{      // Command Register Values
        NOP_cmd     =     B00000000,    // Forces a zero into the RESPONSE register
        RESET_cmd   =     B00000001,    // Performs a software reset of the firmware
        BUSADDR_cmd =     B00000010,    // Modifies I2C address
        PS_FORCE_cmd =    B00000101,    // Forces a single PS measurement
        PSALS_FORCE_cmd = B00000111,    // Forces a single PS and ALS measurement
        PS_PAUSE_cmd   =  B00001001,    // Pauses autonomous PS
        ALS_PAUSE_cmd  =  B00001010,    // Pauses autonomous ALS
        PSALS_PAUSE_cmd = B00001011,    // Pauses PS and ALS
        PS_AUTO_cmd     = B00001101,    // Starts/Restarts an autonomous PS Loop
        ALS_AUTO_cmd    = B00001110,    // Starts/Restarts an autonomous ALS Loop
        PSALS_AUTO_Cmd  = B00001111     // Starts/Restarts autonomous ALS and PS loop
    };     

    PulsePlug (PortI2C& port) : 
    DeviceI2C (port, 0x5A) {
    }

    byte getReg (byte reg);
    void setReg (byte reg, byte val);
    void fetchData ();
    void fetchLedData();
    byte readParam (byte addr);
    void writeParam (byte addr, byte val);

    // variables for output
    unsigned int resp, als_vis, als_ir, ps1, ps2, ps3, aux, more; // keep in this order!
    
    // Pulse values
    int foundNewFinger, red_signalSize, red_smoothValley;
    long red_valley, red_Peak, red_HFoutput, red_smoothPeak; // for PSO2 calc
    int IR_valley, IR_peak, IR_smoothPeak, IR_smoothValley, binOut, lastBinOut;
    unsigned long lastTotal, IRtotal, valleyTime, lastValleyTime, peakTime, lastPeakTime;
    float IR_baseline, red_baseline, IR_HFoutput, IR_HFoutput2, shiftedOutput, LFoutput, hysterisis;
    
    // Pulse globals
    unsigned long red;      // read value from visible red LED
    unsigned long IR1;      // read value from infrared LED1
    unsigned long IR2;      // read value from infrared LED2
    unsigned long lastBeat;
    unsigned long previousBeat;
    unsigned int latestBpm;
    
    role_t role;
    QuadraticEase ease;
};


#endif
