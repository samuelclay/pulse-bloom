#ifndef SAMPLER_H
#define SAMPLER_H

#include <math.h>

#define SAMPLER_SIZE                50
#define DEFAULT_HEARTBEAT_PERIOD    1000 // 60bpm in ms
#define HEARTBEAT_DURATION          250 // ms

class Sampler {
public:
    Sampler();
    void clear();
    void add(uint16_t);
    void calculateStats();
    uint8_t count();
    uint16_t getPercentile(float);
    uint16_t getPeriod();
    bool isPeaked();
    
protected:
    long _peakedTime;
    long _fakePeakedTime;
    bool _hitBottom;
    bool _hitTop;
    uint8_t _idx;
    uint16_t _ar[SAMPLER_SIZE];
    uint8_t _size;
    uint8_t _count;
    uint16_t _max;
    uint16_t _min;
    uint16_t _period;

};

#endif