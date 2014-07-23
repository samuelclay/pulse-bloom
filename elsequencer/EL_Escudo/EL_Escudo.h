/*
  EL_Escudo.h - EL Escudo library
  Copyright (c) 2009 SparkFun Electronics.  All right reserved.
  Written by Ryan Owens

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EL_Escudo_h
#define EL_Escuod_h

#include <inttypes.h>

#define A  2
#define B  3
#define C  4
#define D  5
#define E  6
#define F  7
#define G  8
#define H  9
#define STATUS  10
#define pulse_width  10

class EL_EscudoClass
{
  public:
    void on(char);
    void off(char);
	void all_on(void);
	void all_off(void);
	void fade_in(char);
	void fade_out(char);
	void pulse(char);
};

extern EL_EscudoClass EL;

#endif

