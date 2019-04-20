#include "SoftwareSerial.h"

#define BLUETOOTH_TX 2
#define BLUETOOTH_RX 3

SoftwareSerial Bluetooth(BLUETOOTH_TX, BLUETOOTH_RX);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Bluetooth.begin(38400);

  Serial.println("Bluetooth Debugger v0.0");
  Serial.println("Ready...");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Bluetooth.available()) {
    while (Bluetooth.available()) {
      Serial.print((char)Bluetooth.read());
    }

  }

  if (Serial.available()) {
    //    int i = Serial.available();
    //    Serial.print(i);
    while (Serial.available()) {
      char c = Serial.read();
      Bluetooth.write(c);
      Serial.print(c);
    }
  }

  delay(10);
}