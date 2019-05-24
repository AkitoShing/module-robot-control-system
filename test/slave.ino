
// Written by Nick Gammon
// February 2012

#include <Wire.h>

const byte MY_ADDRESS = 42;
const byte OTHER_ADDRESS = 25;

void setup()
{
    Wire.begin(MY_ADDRESS);
    for (byte i = 2; i <= 7; i++)
        pinMode(i, OUTPUT);
    Wire.onReceive(receiveEvent);
} // end of setup

void loop()
{
    int v = analogRead(0);
    Wire.beginTransmission(OTHER_ADDRESS);
    Wire.write(v < 512);
    Wire.endTransmission();
    delay(20);
} // end of loop

// called by interrupt service routine when incoming data arrives
void receiveEvent(int numBtyes)
{
    for (int i = 0; i < numBtyes; i++)
    {
        byte c = Wire.read();
        // toggle requested LED
        if (digitalRead(c) == LOW)
            digitalWrite(c, HIGH);
        else
            digitalWrite(c, LOW);
    } // end of for loop
} // end of receiveEvent