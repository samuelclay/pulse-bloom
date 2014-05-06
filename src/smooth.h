#ifndef SMOOTH_H
#define SMOOTH_H

#define filterSamples   3              // filterSamples should  be an odd number, no smaller than 3

uint16_t digitalSmooth(uint16_t rawIn, uint16_t *sensSmoothArray);

#endif