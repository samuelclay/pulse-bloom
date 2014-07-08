/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#include "BackEase.h"


/*
 * Constructor
 */

BackEase::BackEase()
{
	// set a sensible default value for the overshoot
	_overshoot=1.70158;
}


/*
 * Set the overshoot value
 */

void BackEase::setOvershoot(NUMBER overshoot_)
{
	_overshoot=overshoot_;
}


/*
 * Ease in
 */

NUMBER BackEase::easeIn(NUMBER time_) const
{
	time_/=_duration;
	return _change*time_*time_*((_overshoot+1)*time_-_overshoot);
}


/*
 * Ease out
 */

NUMBER BackEase::easeOut(NUMBER time_) const
{
	time_=time_/_duration-1;
	return _change*(time_*time_*((_overshoot+1)*time_+_overshoot)+1);
}


/*
 * Ease in and out
 */

NUMBER BackEase::easeInOut(NUMBER time_) const
{
	NUMBER overshoot;

	overshoot=_overshoot*1.525;
	time_/=_duration/2;

	if(time_<1)
		return _change/2*(time_*time_*((overshoot+1)*time_-overshoot));

	time_-=2;
	return _change/2*(time_*time_*((overshoot+1)*time_+overshoot)+2);
}
