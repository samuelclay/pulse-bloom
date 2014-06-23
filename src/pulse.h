#ifndef PULSE_H
#define PULSE_H

void setupPulseSensor();
int readPulseSensor();
void newHeartbeat();
void clearStemLeds();
bool beginStemRising();
void printHeader();
void blink(int loops, int loopTime, bool half);

#endif