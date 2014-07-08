/*
 * Easing Functions: Copyright (c) 2010 Andy Brown
 * http://www.andybrown.me.uk
 *
 * This work is licensed under a Creative Commons
 * Attribution-ShareAlike 3.0 Unported License.
 * http://creativecommons.org/licenses/by-sa/3.0/
 */

#include <math.h>
#include "ElasticEase.h"


/*
 * Constructor
 */

ElasticEase::ElasticEase()
{
// set amplitude and period to zero to trigger use of defaults

	_amplitude=_period=0;
}


/*
 * Ease in
 */

NUMBER ElasticEase::easeIn(NUMBER time_) const
{
	NUMBER p,a,s;

	if(time_==0)
		return 0;

	time_/=_duration;

	if(time_==1)
		return _change;

	if(_period==0)
		p=_duration*0.3;
	else
		p=_period;

	a=_amplitude;
	if(a==0 || a<fabs(_change))
	{
		a=_change;
	  s=p/4;
	}
	else
		s=p/(2*M_PI)*asin(_change/a);

	time_-=1;
	return -(a*pow(2,10*time_)*sin((time_*_duration-s)*(2*M_PI)/p));
}


/*
 * Ease out
 */

NUMBER ElasticEase::easeOut(NUMBER time_) const
{
	NUMBER p,a,s;

	if(time_==0)
		return 0;

	time_/=_duration;
	if(time_==1)
		return _change;

	if(_period==0)
		p=_duration*0.3;
	else
		p=_period;

	a=_amplitude;
	if(a==0 || a<fabs(_change))
	{
		a=_change;
		s=p/4;
	}
	else
		s=p/(2*M_PI)*asin(_change/a);

  return a*pow(2,-10*time_)*sin((time_*_duration-s)*(2*M_PI)/p)+_change;
}


/*
 * Ease in/out
 */

NUMBER ElasticEase::easeInOut(NUMBER time_) const
{
	NUMBER p,a,s;

	if(time_==0)
		return 0;

	time_/=_duration/2;
	if (time_==2)
		return _change;

	if(_period==0)
		p=_duration*(0.3*1.5);
	else
		p=_period;

	a=_amplitude;
	if(a==0 || a<fabs(_change))
	{
		a=_change;
	  s=p/4;
	}
	else
		s=p/(2*M_PI)*asin(_change/a);

  if(time_<1)
  {
  	time_-=1;
	  return -0.5*(a*pow(2,10*time_)*sin((time_*_duration-s)*(2*M_PI)/p));
	}

  time_-=1;
	return a*pow(2,-10*time_)*sin((time_*_duration-s)*(2*M_PI)/p)*0.5+_change;
}


/*
 * Set the period
 */

void ElasticEase::setPeriod(NUMBER period_)
{
	_period=period_;
}


/*
 * Set the amplitude
 */

void ElasticEase::setAmplitude(NUMBER amplitude_)
{
	_amplitude=amplitude_;
}
