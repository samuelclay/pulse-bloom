/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#include "BounceEase.h"


/*
 * Ease in
 */

NUMBER BounceEase::easeIn(NUMBER time_) const
{
	return _change-easeOut(_duration-time_);
}


/*
 * Ease out
 */

NUMBER BounceEase::easeOut(NUMBER time_) const
{
	time_/=_duration;

	if(time_<(1/2.75))
		return _change*(7.5625*time_*time_);

	if(time_<(2/2.75))
	{
		time_-=1.5/2.75;
	  return _change*(7.5625*time_*time_+0.75);
	}

	if(time_<(2.5/2.75))
	{
		time_-=2.25/2.75;
		return _change*(7.5625*time_*time_+0.9375);
	}

	time_-=2.625/2.75;
	return _change*(7.5625*time_*time_+0.984375);
}


/*
 * Ease in and out
 */

NUMBER BounceEase::easeInOut(NUMBER time_) const
{
	if(time_<_duration/2)
		return easeIn(time_*2)*0.5;
	else
		return easeOut(time_*2-_duration)*0.5+_change*0.5;
}
