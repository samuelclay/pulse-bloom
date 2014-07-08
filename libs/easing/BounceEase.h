/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#ifndef __C39EDBEE_E658_41f3_B5A5_7ABE5F798138
#define __C39EDBEE_E658_41f3_B5A5_7ABE5F798138

#include "EasingBase.h"


/*
 * Bouncing easing function
 */

class BounceEase : public EasingBase
{
public:
	// starts the bounce motion slowly,
  // then accelerates motion as it executes
	virtual NUMBER easeIn(NUMBER time_) const;

	// starts the bounce motion fast,
  // and then decelerates motion as it executes.
	virtual NUMBER easeOut(NUMBER time_) const;

	// combines the motion of the easeIn and easeOut
	// methods to start the bounce motion slowly,
	// accelerate motion, then decelerate.
	virtual NUMBER easeInOut(NUMBER time_) const;
};


#endif
