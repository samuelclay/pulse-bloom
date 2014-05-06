#include <tiny/wiring.h>
#include "sampler.h"

Sampler::Sampler() {
    _size = SAMPLER_SIZE;
    clear();
}

void Sampler::clear() {
    _period = DEFAULT_HEARTBEAT_PERIOD;
    _peakedTime = 0;
    _count = 0;
    _idx = 0;
    _max = 0;
    _min = 0;
    _hitBottom = false;
    _hitTop = false;
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

bool Sampler::isPeaked() {
    uint16_t p90 = getPercentile(.9);
    uint16_t p10 = getPercentile(.1);
    uint8_t i = _idx >= 1 ? (_idx - 1) : 0;
    bool peaked = false;
    uint16_t diffTime = millis() - _peakedTime;
    bool hitThreshold = false;
    
    if (!_hitTop && _ar[i] < p10) {
        _hitBottom = true;
    } else if (_hitBottom && !_hitTop &&
               _ar[i] > p90) {
        _hitTop = true;
        _hitBottom = false;
        hitThreshold = true;
        diffTime = _period + 1;
    }
    
    if (hitThreshold || diffTime > _period) {
        if (hitThreshold) {
            _period = millis() - _peakedTime;
        } else {
            _period += 50;
        }
        _peakedTime = millis();
        peaked = true;
    } else if (diffTime < HEARTBEAT_DURATION) {
        peaked = true;
    } else if (_hitTop &&
               diffTime > 500 &&
               diffTime > _period/3){
        _hitTop = false;
    }
    
    return peaked;
}