#include "Arduino.h"
#include "TimerOne.h"

const int numreadings = 5;
volatile unsigned long readings[numreadings];
volatile int index = 0;
unsigned long average = 0;
unsigned long period1 = 0;
unsigned long total=0;

volatile unsigned block_timer = 0;
volatile bool block=false;

volatile bool calcnow = false;

//Strobe settings
const int strobePin = 9;
float dutyCycle = 5.0;  //percent
float dutyCycle_use = (dutyCycle / 100) * 1023;
float strobe_period_float = 0;
unsigned long period_microsec = 0;
const unsigned long non_strobe_period = 1000; //stobe period when not yet in strobe mode - just a light. 1000us == 1000hz

float stobes_per_rot = 1;

void rps_counter(); // forward declare

void setup(){
 Serial.begin(115200);
 //initial light mode
 Timer1.initialize(non_strobe_period);  // 10 000 000 us = 10 Hz
 Timer1.pwm(strobePin, dutyCycle_use);

 attachInterrupt(0, rps_counter, FALLING); //interrupt 0 is pin 2
}

void loop(){

  //Check hall effect block timer
  if(block){
    detachInterrupt(0);    //Disable interrupt for ignoring multitriggers
    if(micros()-block_timer >= 100000){
      block=false;
      attachInterrupt(0, rps_counter, FALLING); //enable interrupt
    }
  }

  if(calcnow){
    detachInterrupt(0);    //Disable interrupt when printing
    total = 0;
    for (int x=numreadings-1; x>=1; x--){ //count DOWN though the array, every 70 min this might make one glitch as the timer rolls over
      period1 = readings[x]-readings[x-1];
      total = total + period1;
    }
    average = total / (numreadings-1); //average period

    Serial.println(average);

    if(average<1000000){ //ie, more than 1hz of rotation.
      strobe_period_float = average / stobes_per_rot ;
      period_microsec = (unsigned int) strobe_period_float;
      Timer1.setPeriod(period_microsec);
      Timer1.pwm(strobePin, dutyCycle_use);
    }
    else{
      Timer1.setPeriod(non_strobe_period);
      Timer1.pwm(strobePin, dutyCycle_use);
    }
    calcnow = false;
    attachInterrupt(0, rps_counter, FALLING); //enable interrupt
  }
}

void rps_counter(){ /* this code will be executed every time the interrupt 0 (pin2) gets low.*/
  //this is really a bit much code for an ISR, but we only have ~3 rps so it should be ok
  readings[index] = micros();
  index++;

  //Double triggering block
  block_timer = micros();
  block=true;

  if(index >= numreadings){
   index=0;
   calcnow=true;
  }
}
