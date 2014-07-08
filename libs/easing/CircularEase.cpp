/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#include <math.h>
#include "CircularEase.h"


/*
 * Ease in
 */

NUMBER CircularEase::easeIn(NUMBER time_) const
{
	time_/= _duration;

	return -_change*(sqrt(1-time_*time_)-1);
}


/*
 * Ease out
 */

NUMBER CircularEase::easeOut(NUMBER time_) const
{
	time_= time_/_duration-1;
	return _change*sqrt(1-time_*time_);
}


/*
 * Ease in and out
 */

NUMBER CircularEase::easeInOut(NUMBER time_) const
{
	time_/=_duration/2;

	if(time_<1)
		return -_change/2*(sqrt(1-time_*time_)-1);

	time_-=2;
	return _change/2*(sqrt(1-time_*time_)+1);
}


