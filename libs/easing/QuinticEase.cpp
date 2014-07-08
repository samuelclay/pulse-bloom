/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include "QuinticEase.h"


/*
 * Ease in
 */

NUMBER QuinticEase::easeIn(NUMBER time_) const
{
	time_/=_duration;
	return _change*time_*time_*time_*time_*time_;
}


/*
 * Ease out
 */

NUMBER QuinticEase::easeOut(NUMBER time_) const
{
	time_=time_/_duration-1;
	return _change*(time_*time_*time_*time_*time_+1);

}


/*
 * Ease in and out
 */

NUMBER QuinticEase::easeInOut(NUMBER time_) const
{
	time_/=_duration/2;

	if(time_<1)
		return _change/2*time_*time_*time_*time_*time_;

	time_-=2;
	return _change/2*(time_*time_*time_*time_*time_+2);
}
