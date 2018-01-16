#include "Arduino.h"
#include <TimerOne.h>

//some code from https://playground.arduino.cc/Main/Readingrps
//strobe modified from TimerOne examples

//rps reading settingsS
//read rps and calculate average every second
const int numreadings = 5;
volatile unsigned long readings[numreadings];
unsigned long average = 0;
unsigned long period1 = 0;
volatile int index = 0;
unsigned long total;

volatile bool strobing = false;

int rpscount = 0;
unsigned long rps = 0;
unsigned long lastmillis = 0;

//Strobe settings
const int strobePin = 9;
float dutyCycle = 5.0;  //percent
float dutyCycle_use = (dutyCycle / 100) * 1023;
float strobe_period_float = 10000000;
unsigned long period_microsec = 10000000;
const unsigned long non_strobe_period = 1000; //stobe period when not yet in strobe mode - just a light. 1000us == 1000hz

float stobes_per_rot = 13.646;

void rps_counter(); // forward declare

void setup(){
 Serial.begin(9600);
 attachInterrupt(0, rps_counter, FALLING); //interrupt 0 is pin 2

 Timer1.initialize(period_microsec);  // 10 000 000 us = 10 Hz
}

void loop(){


 if (millis() - lastmillis >= 1000){  /*Update every one second, this will be equal to reading frecuency (Hz).*/

     detachInterrupt(0);    //Disable interrupt when calculating
     total = 0;
     for (int x=numreadings-1; x>=1; x--){ //count DOWN though the array, every 70 min this might make one glitch as the timer rolls over
       period1 = readings[x]-readings[x-1];
       total = total + period1;
     }

      average = total / numreadings; //average period
      if(average>=1000000){ //if spinning at less than 1hz, then stop strobe mode
        strobing=false;
      }

     if(strobing){
      strobe_period_float = average / stobes_per_rot ;

      period_microsec = (unsigned int) strobe_period_float;
      Timer1.setPeriod(period_microsec);
      Timer1.pwm(strobePin, dutyCycle_use);
    }

    else{ //not strobing
      Timer1.setPeriod(non_strobe_period);
      Timer1.pwm(strobePin, dutyCycle_use);
    }
    lastmillis = millis(); // Update lasmillis
    attachInterrupt(0, rps_counter, FALLING); //enable interrupt
    }
}


void rps_counter(){ /* this code will be executed every time the interrupt 0 (pin2) gets low.*/
  //this is really a bit much code for an ISR, but we only have ~3 rps so it should be ok
  readings[index] = micros();
  index++;
  if(index >= numreadings){
   index=0;
   strobing = true;  //we have enough for an average, start strobing
  }

}
