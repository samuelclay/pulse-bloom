/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#ifndef __710BAE70_D63D_48ab_98F4_8421836D9DE5
#define __710BAE70_D63D_48ab_98F4_8421836D9DE5

#include "EasingBase.h"


/*
 * the motion is defined by a sine wave.
 */

class SineEase : public EasingBase
{
public:
	// starts motion from a zero velocity, and then accelerates
	// motion as it executes.
	virtual NUMBER easeIn(NUMBER time_) const;

	// starts motion fast, and then decelerates motion to
	// a zero velocity as it executes
	virtual NUMBER easeOut(NUMBER time_) const;

	// Combines the motion of the easeIn and easeOut methods to
	// to start the motion from a zero velocity, accelerate motion,
	// then decelerate to a zero velocity
	virtual NUMBER easeInOut(NUMBER time_) const;
};


#endif
