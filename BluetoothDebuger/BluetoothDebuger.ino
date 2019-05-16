#include "SoftwareSerial.h"

#define BLUETOOTH_TX 2
#define BLUETOOTH_RX 3
#define BLUETOOTH_STATE 4
#define BLUETOOTH_EN 5

SoftwareSerial Bluetooth(BLUETOOTH_TX, BLUETOOTH_RX);

void setup() {
  Serial.begin(115200);
  Bluetooth.begin(9600);

  pinMode(BLUETOOTH_STATE, INPUT);
  pinMode(BLUETOOTH_EN, OUTPUT);

  Serial.println("Bluetooth Debugger v0.0");
  Serial.println("Ready...");
}

void loop() {
  if (false) {
    Serial.println("Bluetooth not connected");
    delay(1000);
  } else {
    if (Bluetooth.available()) {
      while (Bluetooth.available()) {
        Serial.print((char)Bluetooth.read());
      }
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
  delay(5);
}
