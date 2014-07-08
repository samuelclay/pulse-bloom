/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#ifndef __1DAE42F8_8F0C_4c42_8F9A_A316F82AB0FA
#define __1DAE42F8_8F0C_4c42_8F9A_A316F82AB0FA

#include "EasingBase.h"


/*
 *  The acceleration of motion for a Cubic easing
 *  equation is greater than for a Quad easing equation.
 */

class CubicEase : public EasingBase
{
public:
	// Starts the motion by backtracking, then reversing
	// direction and moving toward the target
	virtual NUMBER easeIn(NUMBER time_) const;

	// Starts the motion by moving towards the target, overshooting
	// it slightly, and then reversing direction back toward the target
	virtual NUMBER easeOut(NUMBER time_) const;

	// Combines the motion of the easeIn and easeOut methods to
	// start the motion by backtracking, then reversing direction
	// and moving toward target, overshooting target slightly,
	// reversing direction again, and then moving back toward the target
	virtual NUMBER easeInOut(NUMBER time_) const;
};


#endif
