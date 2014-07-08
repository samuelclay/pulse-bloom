/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include <math.h>
#include "ExponentialEase.h"


/*
 * Ease in
 */

NUMBER ExponentialEase::easeIn(NUMBER time_) const
{
	return time_==0 ? 0 : _change*pow(2,10*(time_/_duration-1));
}


/*
 * Ease out
 */

NUMBER ExponentialEase::easeOut(NUMBER time_) const
{
	return time_==_duration ? _change : _change*(-pow(2,-10*time_/_duration)+1);
}


/*
 * Ease in and out
 */

NUMBER ExponentialEase::easeInOut(NUMBER time_) const
{
	if(time_==0)
		return 0;

	if(time_==_duration)
		return _change;

	time_/=_duration/2;

	if(time_<1)
		return _change/2*pow(2,10*(time_-1));

	time_--;
	return _change/2*(-pow(2,-10*time_)+2);
}
