#if defined (__AVR_ATtiny84__)
#include <tiny/wiring.h>
#include <SoftwareSerial/SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#include <Arduino.h>
#endif
#include <avr/io.h>
#include <util/delay.h>     /* for _delay_ms() */
#include "sampler.h"
#include "smooth.h"

// #define USE_SERIAL 1

uint16_t smoothedPeriods[filterSamples];
const uint8_t maxFakeBeats = 2;
uint8_t fakeBeats = 0;

Sampler::Sampler() {
    _size = SAMPLER_SIZE;
    clear();
}

void Sampler::clear() {
#ifdef USE_SERIAL
    Serial.println(" >>> CLEAR");
#endif
    _period = DEFAULT_HEARTBEAT_PERIOD;
    _peakedTime = millis();
    _fakePeakTime = _peakedTime + _period + 100;
    _fakePeakedTime = 0;
    _count = 0;
    _idx = 0;
    _max = 0;
    _min = 0;
    _hitBottom = false;
    _hitTop = false;
    for (int i=0; i < filterSamples; i++) {
        smoothedPeriods[i] = DEFAULT_HEARTBEAT_PERIOD;
    }
}

void Sampler::add(uint16_t n) {
    // Only keeping track of useful values
    if (n == 0) return;

    _ar[_idx++] = n;
    
    if (_idx >= _count) _idx = 0;
    if (_count < _size) _count++;
    if (n < _min) _min = n;
    if (n > _max) {
        _max = n;
        _min = n;
    }
}

void Sampler::calculateStats() {
    uint16_t maxV = 0;
    uint16_t minV = 0;
    
    for (uint16_t i=0; i < _count; i++) {
        if (!maxV || !minV) {
            maxV = _ar[i];
            minV = _ar[i];
        }
        if (_ar[i] > maxV) maxV = _ar[i];
        if (_ar[i] < minV) minV = _ar[i];
    }
    
    _max = maxV;
    _min = minV;
}

uint8_t Sampler::count() {
    return _count;
}

uint16_t Sampler::getPercentile(float p) {
    if (_count <= 0) return 0;
    uint16_t spread = _max - _min;
    
    return _max - round(spread*(1-p));
}

uint16_t Sampler::getPeriod() {
    return _period;
}

int Sampler::isPeaked() {
    calculateStats();
    uint16_t pTop = getPercentile(.7);
    uint16_t pBottom = getPercentile(.5);
    uint8_t i = _idx >= 1 ? (_idx - 1) : 0;
    bool isSpread = (pTop - pBottom) > 20;
    bool peaked = false;
    long currentTime = millis();
    uint16_t sinceLastRealBeat = currentTime - _peakedTime;
    uint16_t sinceLastFakeBeat = currentTime - _fakePeakedTime;
    bool hitfakeThreshold = currentTime > _fakePeakTime;
    bool hitThreshold = false;
    
    if (!_hitBottom && !_hitTop && _ar[i] < pBottom) {
#ifdef USE_SERIAL
        Serial.println(" >>> Hit bottom");
#endif
        _hitBottom = true;
    } else if (_hitBottom && !_hitTop &&
               _ar[i] > pTop &&
               isSpread &&
               sinceLastRealBeat > MIN_HEARTBEAT_PERIOD) {
#ifdef USE_SERIAL
        Serial.println(" >>> Hit top");
#endif
        _hitTop = true;
        _hitBottom = false;
        hitThreshold = true;
    }
    
    if (hitThreshold) {
#ifdef USE_SERIAL
        Serial.println(" >>> Hit threshold");
#endif
        uint16_t unsmoothPeriod = millis() - _peakedTime;
        _period = min(MAX_HEARTBEAT_PERIOD, digitalSmooth(unsmoothPeriod, smoothedPeriods));
        _peakedTime = millis();
        _fakePeakTime = _peakedTime + _period + 100;
        fakeBeats = 0;
        return 1;
    } else if (sinceLastRealBeat < HEARTBEAT_DURATION) {
#ifdef USE_SERIAL
        Serial.println(" >>> In heartbeat");
#endif
        return 1;
    } else if (fakeBeats < maxFakeBeats && hitfakeThreshold) {
#ifdef USE_SERIAL
        Serial.println(" >>> Faking heartbeat");
#endif        
        fakeBeats++;
        _fakePeakedTime = millis();
        _fakePeakTime = _fakePeakedTime + _period + 100;
        return -1;
    } else if (_hitTop &&
               sinceLastRealBeat > _period/8) {
#ifdef USE_SERIAL
        Serial.println(" >>> Resetting hitTop");
#endif
        _hitTop = false;
    } else if (sinceLastFakeBeat < HEARTBEAT_DURATION) {
#ifdef USE_SERIAL
        Serial.println(" >>> In fake heartbeat");
#endif        
        return -1;
    }
    
    return 0;
}