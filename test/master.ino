
// Written by Nick Gammon
// February 2012

#include <Wire.h>

const byte MY_ADDRESS = 25;
const byte SLAVE_ADDRESS = 42;
const byte LED = 13;

void setup(){
  Wire.begin (MY_ADDRESS);
  Wire.onReceive (receiveEvent);
  pinMode(LED, OUTPUT);     
  }  // end of setup

void loop(){
    for(int x = 2; x <= 7; x++){  
    Wire.beginTransmission (SLAVE_ADDRESS);
    Wire.write (x);
    Wire.endTransmission ();
    delay (200);
    }  // end of for
}  // end of loop

void receiveEvent(int howMany){
  for(int i = 0; i < howMany; i++){
    byte b = Wire.read ();
    digitalWrite (LED, b); 
    }  // end of for loop
} // end of receiveEvent

