/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include "LinearEase.h"


/*
 * Ease in
 */

NUMBER LinearEase::easeIn(NUMBER time_) const
{
	return _change*time_/_duration;
}


/*
 * Ease out
 */

NUMBER LinearEase::easeOut(NUMBER time_) const
{
	return easeIn(time_);
}


/*
 * Ease in and out
 */

NUMBER LinearEase::easeInOut(NUMBER time_) const
{
	return easeIn(time_);
}
