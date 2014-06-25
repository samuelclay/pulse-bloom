#ifndef PULSE_H
#define PULSE_H

void setupPulseSensor(PulsePlug *pulse);
int readPulseSensor(PulsePlug *pulse);
void newHeartbeat(PulsePlug *pulse);
void clearStemLeds(PulsePlug *pulse);
bool runStemRising(PulsePlug *pulse);
void beginLedRising(PulsePlug *pulse);
bool runLedRising(PulsePlug *pulse);
void beginLedFalling(PulsePlug *pulse);
bool runLedFalling(PulsePlug *pulse);
void printHeader();
void blink(int loops, int loopTime, bool half);
void resetArduino();

#endif