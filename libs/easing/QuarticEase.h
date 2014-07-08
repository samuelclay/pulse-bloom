/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#ifndef __744A96B2_C5D5_4905_9EB9_7E92543C1B1B
#define __C7168DD6_B2B7_4753_833B_914C84EF332E

#include "EasingBase.h"


/*
 * The acceleration of motion for a Quart easing
 * equation is greater than for a Quad or Cubic.
 */

class QuarticEase : public EasingBase
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
