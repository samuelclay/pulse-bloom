/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution_ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by_sa/3.0/
 */

#ifndef __CD7D63F1_57BA_4f1b_A832_758091A49A7A
#define __CD7D63F1_57BA_4f1b_A832_758091A49A7A

#include "EasingConstants.h"


// base class for easing functions

class EasingBase
{
protected:
	NUMBER _change;
	NUMBER _duration;

public:
	// default constructor
	EasingBase();

	// easing API methods
	virtual NUMBER easeIn(NUMBER time_) const=0;
	virtual NUMBER easeOut(NUMBER time_) const=0;
	virtual NUMBER easeInOut(NUMBER time_) const=0;

	// common properties
	void setDuration(NUMBER duration_);
	void setTotalChangeInPosition(NUMBER totalChangeInPosition_);
};


#endif
