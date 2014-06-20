#ifndef PULSE_H
#define PULSE_H

void setupPulseSensor();
void readPulseSensor();
void printHeader();
void blink(int loops, int loopTime, bool half);

#endif