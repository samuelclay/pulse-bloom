/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#include "EasingBase.h"


/*
 * Default constructor
 */

EasingBase::EasingBase()
{
	_change=0;
}


/*
 * Set the duration
 */

void EasingBase::setDuration(NUMBER duration_)
{
	_duration=duration_;
}


/*
 * Set the total change in position
 */

void EasingBase::setTotalChangeInPosition(NUMBER totalChangeInPosition_)
{
	_change=totalChangeInPosition_;
}
