#include <tiny/wiring.h>
#include "sampler.h"
#include "smooth.h"

uint16_t smoothedPeriods[filterSamples];

Sampler::Sampler() {
    _size = SAMPLER_SIZE;
    clear();
}

void Sampler::clear() {
    _period = DEFAULT_HEARTBEAT_PERIOD;
    _peakedTime = millis();
    _fakePeakedTime = _peakedTime;
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

int Sampler::isPeaked(SoftwareSerial mySerial) {
    uint16_t pTop = getPercentile(.5);
    uint16_t pBottom = getPercentile(.3);
    uint8_t i = _idx >= 1 ? (_idx - 1) : 0;
    bool isSpread = (pTop - pBottom) > 20;
    bool peaked = false;
    long currentTime = millis();
    uint16_t diffTime = currentTime - _peakedTime;
    uint16_t fakeDiffTime = _fakePeakedTime > currentTime ? 0 : 
                            max(0, currentTime - _fakePeakedTime);
    bool hitThreshold = false;
    
    if (!_hitTop && _ar[i] < pBottom && isSpread) {
        _hitBottom = true;
    } else if (_hitBottom && !_hitTop &&
               _ar[i] > pTop &&
               _ar[i] > pBottom*1.2 &&
               diffTime > MIN_HEARTBEAT_PERIOD) {
        _hitTop = true;
        _hitBottom = false;
        hitThreshold = true;
    } else if (diffTime > MAX_HEARTBEAT_PERIOD) {
        _hitTop = true;
        _hitBottom = false;
        hitThreshold = true;
    }
    
    if (hitThreshold) {
        uint16_t unsmoothPeriod = millis() - _peakedTime;
        _period = digitalSmooth(unsmoothPeriod, smoothedPeriods, mySerial);
        _peakedTime = millis();
        _fakePeakedTime = _peakedTime;
        return 1;
    } else if (diffTime < HEARTBEAT_DURATION) {
        return 1;
    } else if (fakeDiffTime < HEARTBEAT_DURATION) {
        return -1;
    } else if (_hitTop &&
               diffTime > _period/3) {
        _hitTop = false;
    } else if (fakeDiffTime > (_peakedTime + _period + 100)) {
        _fakePeakedTime = millis() + 100;
        return -1;
    }
    
    return 0;
}