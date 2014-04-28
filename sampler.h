#ifndef Sampler_h
#define Sampler_h

#include <math.h>

class Sampler {
public:
    Sampler();
    void clear();
    void add(float);
    float count();
    
protected:
    float _sum;
    float _period;
};

#endif