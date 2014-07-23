/*
 * EL_Blink
 * Copyright (c) 2009 SparkFun Electronics.  All right reserved.
 * Written by Ryan Owens
 *
 * This code was written to utilize the EL Escudo Shield from SparkFun Electronics
 * 
 * This is a basic example which will turn each EL string on for 1 second (one at a time). 
 * After cycling through each string, the sketch will turn all of the strings on for 1 second.
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
  //Turn on the EL string on Channel A for 1 second
  EL.on(A);
  delay(1000);
  EL.off(A);

  //Turn on the EL string on Channel B for 1 second
  EL.on(B);
  delay(1000);
  EL.off(B);
  
  //Turn on the EL string on Channel C for 1 second
  EL.on(C);
  delay(1000);
  EL.off(C);  
  
  //Turn on the EL string on Channel D for 1 second
  EL.on(D);
  delay(1000);
  EL.off(D);  
  
  //Turn on the EL string on Channel E for 1 second
  EL.on(E);
  delay(1000);
  EL.off(E);

  //Turn on the EL string on Channel F for 1 second
  EL.on(F);
  delay(1000);
  EL.off(F);   
  
  //Turn on the EL string on Channel G for 1 second
  EL.on(G);
  delay(1000);
  EL.off(G);
  
  //Turn on the EL string on Channel H for 1 second
  EL.on(H);
  delay(1000);
  EL.off(H);
  
  //Turn on the EL string on all channels for 1 second
  EL.all_on();
  delay(1000);
  EL.all_off();    
}
