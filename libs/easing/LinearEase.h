/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#ifndef __76A3B25E_C909_4183_B13C_72AACD322E69
#define __76A3B25E_C909_4183_B13C_72AACD322E69

#include "EasingBase.h"


/*
 * Linear ease
 */

class LinearEase : public EasingBase
{
public:
	// no acceleration
	virtual NUMBER easeIn(NUMBER time_) const;

	// no acceleration
	virtual NUMBER easeOut(NUMBER time_) const;

	// no acceleration
	virtual NUMBER easeInOut(NUMBER time_) const;
};


#endif
