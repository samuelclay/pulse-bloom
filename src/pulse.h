#ifndef PULSE_H
#define PULSE_H

// Modes
void determinePlayerMode();
void determineFingerMode(int sensor1On, int sensor2On);
void resetStem(PulsePlug *pulse);
uint8_t adjustBpm(PulsePlug *pulse);

// State: resting
void runResting();
void runRestStem();
void runRestStem(PulsePlug *pulse, int16_t currentLed);
void clearStemLeds(PulsePlug *pulse);

// State: end resting
void beginSplittingStem();
void runSplittingStem();
void runSplittingStem(PulsePlug *pulse, int16_t currentLed);

// State: stem rising
bool runStemRising(PulsePlug *pulse, PulsePlug *shadowPulse);

// State: petal rising
void beginPetalRising(PulsePlug *pulse);
bool runPetalRising(PulsePlug *pulse);

// State: petal falling
void beginPetalFalling(PulsePlug *pulse);
bool runPetalFalling(PulsePlug *pulse);

// State: petal rest states
void beginPetalResting();
bool runPetalResting();

// Debugging
void printHeader();
void blink(int loops, int loopTime, bool half);
int freeRam ();

extern "C" void __cxa_pure_virtual() { }

#endif