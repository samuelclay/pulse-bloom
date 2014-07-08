/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#ifndef __E3C2602E_AF87_4ca3_9E5B_2F822F390F32
#define __E3C2602E_AF87_4ca3_9E5B_2F822F390F32

#include "EasingBase.h"


/*
 * Circular easing
 */

class CircularEase : public EasingBase
{
public:
	// starts motion slowly, and then accelerates
	// motion as it executes
	virtual NUMBER easeIn(NUMBER time_) const;

	// starts motion fast, and then decelerates
	// motion as it executes.
	virtual NUMBER easeOut(NUMBER time_) const;

	// combines the motion of the easeIn and easeOut
	// methods to start the motion slowly, accelerate
	// motion, then decelerate.
	virtual NUMBER easeInOut(NUMBER time_) const;
};


#endif
