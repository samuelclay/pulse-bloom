/*
 * EL_Fade
 * Copyright (c) 2009 SparkFun Electronics.  All right reserved.
 * Written by Ryan Owens
 *
 * This code was written to utilize the EL Escudo Shield from SparkFun Electronics
 * 
 * This example illustrates how to use the fade_in and fade_out functions of the EL_Escudo library.
 * The sketch will cycle through each string on the EL Escudo shield, fading them in and then out.
 *
 * The EL Escudo shield uses digital pins 2-9 for the EL strings. Be careful if you decide
 * to use these pins for any other purpose Digital pin 10 can be used for the status LED
 * on the shield.
 * 
 * REMEMBER: When using the EL Escudo shield, up to 2 strings can be turned on at once. If more 
 * than two strings are turned on, the shield will malfunction.
 *
 * http://www.sparkfun.com
 */

#include <EL_Escudo.h>
//The EL_Escudo library uses letters A-H to reference each EL string.
//Each EL string output has a corresponding label on the EL Escudo shield.

void setup()                    // run once, when the sketch starts
{
  EL.all_off();
}

void loop()                     // run over and over again
{
  for(int string = A; string <= H; string++)
  {
    EL.fade_in(string);
    delay(200);
    EL.fade_out(string);
  }
}
