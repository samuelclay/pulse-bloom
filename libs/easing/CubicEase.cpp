/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include "CubicEase.h"


/*
 * Starts motion from zero velocity, and then
 * accelerates motion as it executes.
 */

NUMBER CubicEase::easeIn(NUMBER time_) const
{
	time_/=_duration;

	return _change*time_*time_*time_;
}


/*
 * Starts motion fast, and then decelerates motion
 * to a zero velocity as it executes.
 */

NUMBER CubicEase::easeOut(NUMBER time_) const
{
	time_=time_/_duration-1;

	return _change*(time_*time_*time_+1);
}


/*
 * Combines the motion of the easeIn and easeOut methods
 * to start the motion from zero velocity, accelerates motion,
 * then decelerates back to a zero velocity.
 */

NUMBER CubicEase::easeInOut(NUMBER time_) const
{
	time_/=_duration/2;

	if (time_<1)
		return _change/2*time_*time_*time_;

	time_-=2;
	return _change/2*(time_*time_*time_+2);
}
