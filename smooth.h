#ifndef SMOOTH_H
#define SMOOTH_H

#define filterSamples   7              // filterSamples should  be an odd number, no smaller than 3

int digitalSmooth(int rawIn, int *sensSmoothArray);

#endif