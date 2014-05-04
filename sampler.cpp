#include <tiny/wiring.h>
#include "sampler.h"

Sampler::Sampler() {
    _size = SAMPLER_SIZE;
    clear();
}

void Sampler::clear() {
    _sum = 0;
    _period = 1;
    _peakedTime = 0;
    _sorted = false;
    _count = 0;
    _idx = 0;
}

void Sampler::add(int n) {
    _ar[_idx++] = n;
    _sorted = false;
    
    if (_idx >= _count) _idx = 0;
    if (_count < _size) _count++;
}

int Sampler::count() {
    return _sum;
}

int Sampler::getPercentile(float p) {
    if (_count <= 0) return 0;
    if (!_sorted) sort();
    
    int i = round(_count*p);
    return _as[i];
}

bool Sampler::isPeaked() {
    int p90 = getPercentile(.9);
    int p10 = getPercentile(.1);
    int i = _idx >= 1 ? (_idx - 1) : 0;
    bool peaked = false;
    
    long diffTime = millis() - _peakedTime;
    if (diffTime > 1000*_period) {
        _peakedTime = millis();
        peaked = true;
    } else if (diffTime < 150) {
        peaked = true;
    }
    
    return peaked;
}

void Sampler::sort() {
    for (int i=0; i < _count; i++) _as[i] = _ar[i];

    for (int i=0; i < _count-1; i++) {
        int m = i;
        for (int j=i+1; j < _count; j++) {
            if (_as[j] < _as[m]) m = j;
        }
        if (m != i) {
            int t = _as[m];
            _as[m] = _as[i];
            _as[i] = t;
        }
    }
    
    _sorted = true;
}