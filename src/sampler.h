#ifndef SAMPLER_H
#define SAMPLER_H

#include <math.h>
#include <SoftwareSerial/SoftwareSerial.h>

#define SAMPLER_SIZE                20
#define DEFAULT_HEARTBEAT_PERIOD    1000 // 60bpm in ms
#define HEARTBEAT_DURATION          250 // ms
#define MAX_HEARTBEAT_PERIOD        1500 // 45bpm in ms isn't possible
#define MIN_HEARTBEAT_PERIOD        500 // 120bpm in ms is too high

class Sampler {
public:
    Sampler();
    void clear();
    void add(uint16_t);
    void calculateStats();
    uint8_t count();
    uint16_t getPercentile(float);
    uint16_t getPeriod();
    int isPeaked(SoftwareSerial);
    
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