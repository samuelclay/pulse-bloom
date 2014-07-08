/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#ifndef __C5237486_C431_43ee_AC67_9C773711B326
#define __C5237486_C431_43ee_AC67_9C773711B326

#include "EasingBase.h"


/*
 * Back easing function
 */

class BackEase : public EasingBase
{
private:
	NUMBER _overshoot;

public:
	// constructor - sets a default value for the overshoot
	BackEase();

	// Starts the motion by backtracking, then reversing
	// direction and moving toward the target
	virtual NUMBER easeIn(NUMBER time_) const;

	// Starts the motion by moving towards the target, overshooting
	// it slightly, and then reversing direction back toward the target
	virtual NUMBER easeOut(NUMBER time_) const;

	// Combines the motion of the easeIn and easeOut methods to
	// start the motion by backtracking, then reversing direction
	// and moving toward target, overshooting target slightly,
	// reversing direction again, and then moving back toward the target
	virtual NUMBER easeInOut(NUMBER time_) const;

	// set the overshoot value. The higher the value the
	// greater the overshoot.
	void setOvershoot(NUMBER overshoot_);
};


#endif
