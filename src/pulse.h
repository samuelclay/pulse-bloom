#include <easing/QuinticEase.h>

#ifndef PULSE_H
#define PULSE_H

void newHeartbeat(PulsePlug *pulse);
void runResting();
void runRestStem();
void runRestStem(PulsePlug *pulse, int16_t currentLed);
void clearStemLeds(PulsePlug *pulse);
bool runStemRising(PulsePlug *pulse, PulsePlug *shadowPulse);
void beginLedRising(PulsePlug *pulse);
bool runLedRising(PulsePlug *pulse);
void beginLedFalling(PulsePlug *pulse);
bool runLedFalling(PulsePlug *pulse);
void printHeader();
void blink(int loops, int loopTime, bool half);
int freeRam ();

extern "C" void __cxa_pure_virtual() { }

#endif