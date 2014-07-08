/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#ifndef __98FC70EC_04B4_4cfd_A4C8_FEEE67F265CB
#define __98FC70EC_04B4_4cfd_A4C8_FEEE67F265CB

#include "EasingBase.h"


/*
 * Elastic easing function. The motion is defined by
 * an exponentially decaying sine wave.
 */

class ElasticEase : public EasingBase
{
private:
	NUMBER _period;
	NUMBER _amplitude;

public:
	// constructor - sets a default value for the overshoot
	ElasticEase();

	// Starts motion slowly, and then accelerates motion as it executes.
	virtual NUMBER easeIn(NUMBER time_) const;

	// Starts motion fast, and then decelerates motion as it executes
	virtual NUMBER easeOut(NUMBER time_) const;

	// combines the motion of the easeIn and easeOut methods
	// to start the motion slowly, accelerate motion, then decelerate
	virtual NUMBER easeInOut(NUMBER time_) const;

	// set the period
	void setPeriod(NUMBER period_);

	// set the amplitude
	void setAmplitude(NUMBER amplitude_);
};


#endif
