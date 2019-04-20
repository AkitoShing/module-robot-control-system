#include "SoftwareSerial.h"

#define BLUETOOTH_TX 2
#define BLUETOOTH_RX 3

SoftwareSerial Bluetooth(BLUETOOTH_TX, BLUETOOTH_RX);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115400);
  Bluetooth.begin(9600);

  Serial.println("Bluetooth Debugger v0.0");
  Serial.println("Ready...");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Bluetooth.available()) {
    Serial.print((char)Bluetooth.read());

  }

  if (Serial.available()) {
    Bluetooth.print((char)Serial.read());
  }

  if (Serial.available()) {
    Serial.print((char)Serial.read());
  }
}
