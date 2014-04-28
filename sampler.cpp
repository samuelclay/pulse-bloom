#include "sampler.h"

Sampler::Sampler() {
    clear();
}

void Sampler::clear() {
    _sum = 0;
    _period = 0;
}

void Sampler::add(float f) {
    
}

float Sampler::count() {
    return _sum;
}