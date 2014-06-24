#ifndef PULSE_H
#define PULSE_H

void setupPulseSensor(PulsePlug pulse);
int readPulseSensor(PulsePlug pulse);
void newHeartbeat();
void clearStemLeds();
bool runStemRising();
void beginLedRising();
bool runLedRising();
void beginLedFalling();
bool runLedFalling();
void printHeader();
void blink(int loops, int loopTime, bool half);
void resetArduino();

#endif