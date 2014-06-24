#ifndef PULSE_H
#define PULSE_H

void setupPulseSensor();
int readPulseSensor();
void newHeartbeat();
void clearStemLeds();
bool beginStemRising();
void beginLedRising();
bool runLedRising();
void beginLedFalling();
bool runLedFalling();
void printHeader();
void blink(int loops, int loopTime, bool half);
void resetArduino();

#endif