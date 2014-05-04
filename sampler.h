#ifndef SAMPLER_H
#define SAMPLER_H

#include <math.h>

#define SAMPLER_SIZE 50

class Sampler {
public:
    Sampler();
    void clear();
    void add(int);
    int count();
    int getPercentile(float);
    bool isPeaked();
    
protected:
    bool _sorted;
    float _sum;
    float _period;
    long _peakedTime;
    int _size;
    int _count;
    int _idx;
    int _ar[SAMPLER_SIZE];
    int _as[SAMPLER_SIZE];

    void sort();
};

#endif