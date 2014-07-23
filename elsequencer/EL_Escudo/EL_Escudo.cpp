/*
  EL_Escudo.cpp - EL Escudo library
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "WConstants.h"
#include "EL_Escudo.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/

void EL_EscudoClass::on(char channel)
{
	pinMode(channel, OUTPUT);
	digitalWrite(channel, LOW); 
}

void EL_EscudoClass::off(char channel)
{
	pinMode(channel, INPUT);
}

void EL_EscudoClass::all_on(void)
{
	for(int i=0; i<4; i++){
	  EL.on(i*2+A);
	  EL.on(i*2+1+A);
	  delayMicroseconds(20);
	  EL.off(i*2+A);
	  EL.off(i*2+1+A);   
	}
}

void EL_EscudoClass::all_off(void)
{
	for(int i=A; i<10; i++)EL.off(i);
}

void EL_EscudoClass::fade_in(char channel)
{
	for(int brightness=0; brightness<=pulse_width; brightness++){
		for(int duration=0; duration<5; duration++){
			EL.on(channel);
			delay(brightness);
			EL.off(channel);
			delay(pulse_width-brightness);
		}
	}
	EL.on(channel);
}

void EL_EscudoClass::fade_out(char channel)
{
	for(int brightness=pulse_width; brightness>=0; brightness--){
		for(int duration=0; duration<5; duration++){
			EL.on(channel);
			delay(brightness);
			EL.off(channel);
			delay(pulse_width-brightness);
		}
	}
}

void EL_EscudoClass::pulse(char channel)
{
	EL.fade_in(channel);
	EL.fade_out(channel);
}

EL_EscudoClass EL;
