#include "Arduino.h"
#include <TimerOne.h>

//some code from https://playground.arduino.cc/Main/Readingrps
//strobe modified from TimerOne examples

//rps reading settingsS
//read rps and calculate average every second
const int numreadings = 10;
int readings[numreadings];
unsigned long average = 0;
int index = 0;
unsigned long total;

volatile int rpscount = 0;
unsigned long rps = 0;
unsigned long lastmillis = 0;

//Strobe settings
const int strobePin = 9;
float dutyCycle = 5.0;  //percent
float dutyCycle_use = (dutyCycle / 100) * 1023;
float period_micro_float = 10000000;
unsigned long period_microsec = 10000000;

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
     readings[index] = rpscount;  /* One interruption per full rotation. For two interrupts per full rotation use rpscount/2.*/

     for (int x=0; x<=9; x++){
       total = total + readings[x];
     }

     average = total / numreadings;
     //rps = average;

     rps = rpscount; //TODO, average not used for now, so just take the current reading

     rpscount = 0; // Restart the rps counter
     index++;
     if(index >= numreadings){
      index=0;
     }

    // if (millis() > 11000){  // wait for rpss average to get stable
    //  Serial.print(" rps = ");
    //  Serial.println(rps);
    // }

    //set Strobe
    //calc frequncy
    if (rps<=1){
      rps=1000;
    }
    period_micro_float = (1000000/rps)  / stobes_per_rot ;

    period_microsec = (unsigned int) period_micro_float;
    Timer1.setPeriod(period_microsec);
    Timer1.pwm(strobePin, dutyCycle_use);

    lastmillis = millis(); // Update lasmillis
    attachInterrupt(0, rps_counter, FALLING); //enable interrupt
    }
}


void rps_counter(){ /* this code will be executed every time the interrupt 0 (pin2) gets low.*/
  rpscount++;
}
