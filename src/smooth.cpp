/* digitalSmooth
 Paul Badger 2007
 A digital smoothing filter for smoothing sensor jitter 
 This filter accepts one new piece of data each time through a loop, which the 
 filter inputs into a rolling array, replacing the oldest data with the latest reading.
 The array is then transferred to another array, and that array is sorted from low to high. 
 Then the highest and lowest %15 of samples are thrown out. The remaining data is averaged
 and the result is returned.

 Every sensor used with the digitalSmooth function needs to have its own array to hold 
 the raw sensor values. This array is then passed to the function, for it's use.
 This is done with the name of the array associated with the particular sensor.
 */

#include <math.h>
#include <tiny/wiring.h>
#include <SoftwareSerial/SoftwareSerial.h>
#include "smooth.h"

uint16_t digitalSmooth(uint16_t rawIn, uint16_t *sensSmoothArray, SoftwareSerial mySerial) {
  uint16_t j, k, temp, top, bottom;
  long total;
  static uint8_t i;
 // static uint8_t raw[filterSamples];
  static uint16_t sorted[filterSamples];
  uint8_t done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  if (false) {
      for (j = 0; j < (filterSamples); j++){    // print the array to debug
        mySerial.print(sorted[j]); 
        mySerial.print("   "); 
      }
      mySerial.println();
  }


  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  // bottom = max(((filterSamples * 15)  / 100), 1); 
  // top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  bottom = 0;
  top = filterSamples;
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
    // Serial.print(sorted[j]); 
    // Serial.print("   "); 
  }

//  Serial.println();
//  Serial.print("average = ");
//  Serial.println(total/k);
  return total / k;    // divide by number of samples
}
