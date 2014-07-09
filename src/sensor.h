#ifndef SENSOR_H
#define SENSOR_H

void setupPulseSensor(PulsePlug *pulse);
int readPulseSensor(PulsePlug *pulse);
void resetArduino();

#endif