/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#ifndef __EB8DFB1F_5506_4a49_BD8E_B0A1E0C82891
#define __EB8DFB1F_5506_4a49_BD8E_B0A1E0C82891

#include "EasingBase.h"


/*
 *  the motion is defined by an exponentially decaying sine wave
 */

class ExponentialEase : public EasingBase
{
public:

	//starts motion slowly, and then accelerates motion as it executes
	virtual NUMBER easeIn(NUMBER time_) const;

	// starts motion fast, and then decelerates motion as it executes
	virtual NUMBER easeOut(NUMBER time_) const;

	//combines the motion of the easeIn and easeOut methods to start the
	// motion slowly, accelerate motion, then decelerate
	virtual NUMBER easeInOut(NUMBER time_) const;
};


#endif
