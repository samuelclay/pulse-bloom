#include <easing/QuinticEase.h>

#ifndef PULSE_H
#define PULSE_H

void setupPulseSensor(PulsePlug *pulse);
int readPulseSensor(PulsePlug *pulse);
void newHeartbeat(PulsePlug *pulse);
void runResting();
void runRestStem();
void clearStemLeds(PulsePlug *pulse);
bool runStemRising(PulsePlug *pulse);
void beginLedRising(PulsePlug *pulse);
bool runLedRising(PulsePlug *pulse);
void beginLedFalling(PulsePlug *pulse);
bool runLedFalling(PulsePlug *pulse);
void printHeader();
void blink(int loops, int loopTime, bool half);
void resetArduino();
int freeRam ();
QuinticEase ease;

extern "C" void __cxa_pure_virtual() { }

#endif