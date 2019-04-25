#include <SoftwareSerial.h>
#include <Wire.h>
#include <Data.h>
#include "timer.h"

//H-bridge
#define HBRIDGE_IN1 8
#define HBRIDGE_IN2 9
#define LEFT_MOTOR 10
#define HBRIDGE_IN3 12
#define HBRIDGE_IN4 13
#define RIGHT_MOTOR 11

//Bluetooth Communi cation
#define BLUETOOTH_BAUDRATE 9600
#define BLUETOOTH_TX 2
#define BLUETOOTH_RX 3
#define BLUETOOTH_STATE 4
#define BLUETOOTH_POWER 5

Timer routine;

SoftwareSerial Bluetooth(BLUETOOTH_TX, BLUETOOTH_RX);
char BluetoothBuffer[9];        //Empty BluetoothBuffer for data communication
String BluetoothData = "";

String robotStatus = "";                                    //Status of robot
bool robotStatusUpdated = false;
String moduleStatus = "";                                   //Status of robot
bool moduleStatusUpdated = false;
String moduleName = "";                                     //The name of attached module
bool moduleNameUpdated = false;
String moduleCreator = "";                                  //The creatot of attached module
bool moduleCreatorUpdated = false;

void setup() {
  setPinMode();
  digitalWrite(BLUETOOTH_POWER, HIGH);

  Serial.begin(115200);

  Bluetooth.begin(BLUETOOTH_BAUDRATE);

  Wire.begin(MASTER_ADDRESS);
  Wire.onReceive(ResponeReceived);

  routine.setInterval(1000);
  routine.setCallback(routineTask);
  routine.start();

  if (moduleEnabled()) {
    Serial.println("Module Detacted ");
    Serial.println("Requesting Module Info");
    requestModuleInfo();
    delay(2000);
  }
}

void loop() {
  getBlueToothData();
  routine.update();
}

void setPinMode() {
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);
  pinMode(HBRIDGE_IN1, OUTPUT);
  pinMode(HBRIDGE_IN2, OUTPUT);
  pinMode(HBRIDGE_IN3, OUTPUT);
  pinMode(HBRIDGE_IN4, OUTPUT);
  pinMode(MODULE_ENABLE_PIN, INPUT);
  pinMode(BLUETOOTH_STATE, INPUT);
  pinMode(BLUETOOTH_POWER, OUTPUT);
}

void getBlueToothData() {
  int datasize = 0;
  if (Bluetooth.available()) {
    delay(2);
    BluetoothBuffer[datasize] = Bluetooth.read();
    datasize++;
    if (BluetoothBuffer[0] == '{' ||
        BluetoothBuffer[0] == '(' ||
        BluetoothBuffer[0] == '<' ) {
      while (Bluetooth.available())  {
        delay(1);
        BluetoothBuffer[datasize] = Bluetooth.read();
        datasize++;
        if (BluetoothBuffer[datasize] > 127 || datasize > 8) {
          break;     // Communication error
        }
        if (BluetoothBuffer[datasize - 1] == '}' ||
            BluetoothBuffer[datasize - 1] == ')' ||
            BluetoothBuffer[datasize - 1] == '>') {
          break;     // Finish receive
        }
      }
    }
    for (int pointer = 0; pointer < datasize; pointer++) {
      Serial.print((char)BluetoothBuffer[pointer]);
    }
    Serial.print("\n");
    if (BluetoothBuffer[1] == '{') {
      setRobotControl(BluetoothBuffer);
    }
    if ((BluetoothBuffer[1] == DATA_TYPE_REQUEST || BluetoothBuffer[1] == MODULE_DATA_MODULE_ACTION ) && datasize == 4) { //Module Control
      sendModuleRequest(BluetoothBuffer[1], BluetoothBuffer[2]);
    }
  }
}



void sendBluetoothData(String data) { //TODO:
  String BluetoothData = "";
  BluetoothData += ROBOT_START_TRANSMIT;
  BluetoothData += data;
  BluetoothData += ROBOT_END_TRANSMIT;
  Bluetooth.print(BluetoothData);
  Serial.print("Sended Bluetooth Data: ");
  Serial.println(data);
  delay(2);
}

void setRobotControl(char data[9]) {
  Serial.println("set Robot control");
  int angle     = (data[2] - 48) * 100 + (data[3] - 48) * 10 + (data[4] - 48); // obtain the Int from the ASCII representation
  int amplitube = (data[5] - 48) * 100 + (data[6] - 48) * 10 + (data[7] - 48);

  Serial.print("Joystick Control received: ");
  Serial.print("Angle: ");
  Serial.print(angle);
  Serial.print(" Amplitube: ");
  Serial.println(amplitube);

  if (angle < 0 || angle > 360 || amplitube < 0 || amplitube > 255) {
    return; //Data Error
  }

  byte leftMix;
  byte rightMix;

  if (angle > 170 && angle < 190) {
    // Serial.print("Left spin ");
    digitalWrite(HBRIDGE_IN1, LOW);
    digitalWrite(HBRIDGE_IN2, HIGH);
    digitalWrite(HBRIDGE_IN3, HIGH);
    digitalWrite(HBRIDGE_IN4, LOW);
    leftMix = 255;
    rightMix = 255;
  }
  if (angle >= 0 && angle < 11 || angle > 350 && angle < 360) {
    // Serial.print("Right spin");
    digitalWrite(HBRIDGE_IN1, HIGH);
    digitalWrite(HBRIDGE_IN2, LOW);
    digitalWrite(HBRIDGE_IN3, LOW);
    digitalWrite(HBRIDGE_IN4, HIGH);
    leftMix = 255;
    rightMix = 255;
  }
  if (angle > 10 && angle < 171) {
    // Serial.print("Forward ");
    digitalWrite(HBRIDGE_IN1, HIGH);
    digitalWrite(HBRIDGE_IN2, LOW);
    digitalWrite(HBRIDGE_IN3, HIGH);
    digitalWrite(HBRIDGE_IN4, LOW);
    leftMix = map(angle, 90, 170, 255, 0);
    if (angle < 90) leftMix = 255;
    rightMix = map(angle, 11, 90, 0, 255);
    if (angle > 90) rightMix = 255;
  }
  if (angle > 189 && angle < 351) {
    // Serial.print("Backward ");
    digitalWrite(HBRIDGE_IN1, LOW);
    digitalWrite(HBRIDGE_IN2, HIGH);
    digitalWrite(HBRIDGE_IN3, LOW);
    digitalWrite(HBRIDGE_IN4, HIGH);
    leftMix = map(angle, 270, 351, 0, 255);
    if (angle > 270) leftMix = 255;
    rightMix = map(angle, 189, 270, 255, 0);
    if (angle < 270) rightMix = 255;
  }

  Serial.print("Left Motor Mix: ");
  Serial.print(leftMix);
  Serial.print(" Right Motor Mix: ");
  Serial.println(rightMix);

  //TODO: Test the new calculation
  float leftMotorPower = ((float)leftMix * (float)amplitube) / (float)255;
  float rightMotorPower = ((float)rightMix * (float)amplitube) / (float)255;

  Serial.print("Left Motor Power: ");
  Serial.print((float)leftMotorPower);
  Serial.print(" Right Motor Power: ");
  Serial.println((float)rightMotorPower);

  analogWrite(LEFT_MOTOR, (float)leftMotorPower);
  analogWrite(RIGHT_MOTOR, (float)rightMotorPower);
}

void sendModuleRequest(char requestType, char controlChar) {
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(requestType);
  Wire.write(controlChar);
  Wire.endTransmission();

  Serial.print("Sending Module Request Type: ");
  Serial.print(requestType);
  Serial.print(" Data: ");
  Serial.println(controlChar);
}

void ResponeReceived(int count) {
  char responeType;
  String data = "";
  if (Wire.available()) {
    responeType = Wire.read();

    Serial.print("Respone recevied: ");
    Serial.println(responeType);
  }
  while (Wire.available()) {
    data += (char)Wire.read();
  }
  switch (responeType) {
    case MODULE_DATA_MODULE_INFO_NAME:
      moduleName = data;
      Serial.print("Name recevied: ");
      Serial.println(moduleName);
      moduleNameUpdated = true;
      break;

    case MODULE_DATA_MODULE_INFO_CREATOR:
      moduleCreator = data;
      Serial.print("Creator recevied: ");
      Serial.println(moduleCreator);
      moduleCreatorUpdated = true;
      break;

    case MODULE_DATA_MODULE_STATUS:
      moduleStatus = data;
      Serial.print("Status recevied: ");
      Serial.println(moduleStatus);
      moduleStatusUpdated = true;
      break;
  }
  while (Wire.available()) {
    Serial.print(Wire.read());
  }
}

void requestModuleInfo() {
  sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_NAME);
  Serial.println("Name requested");
  sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_CREATOR);
  Serial.println("Creator requested");
}

void requestModuleStatus() {
  sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_STATUS);
  Serial.println("Module Status requested");
}

bool moduleEnabled() {
  return digitalRead(MODULE_ENABLE_PIN);
}

void routineTask() {
  if (!moduleEnabled()) { //TODO: add timeout to this condition
    String data = "";
    data = MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_STATUS;
    data += MODULE_DATA_MODULE_STATUS_MODULE_DISABLE;
    sendBluetoothData(data);
    Serial.println(data);
  }
  if (!moduleEnabled()) {
    if (moduleName = "") {
      Serial.println("No name recevied!");
      sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_NAME);
      Serial.println("Name requested");
    }
    if (moduleCreator = "") {
      Serial.println("No creator recevied!");
      sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_CREATOR);
      Serial.println("Creator requested");
    }
  }
  if (moduleNameUpdated) {
    String data = "";
    data += MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_INFO_NAME;
    data += moduleName;
    sendBluetoothData(data);
    moduleNameUpdated = false;
  }
  if (moduleCreatorUpdated) {
    String data = "";
    data += MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_INFO_CREATOR;
    data += moduleCreator;
    sendBluetoothData(data);
    moduleCreatorUpdated = false;
  }
  if (moduleStatusUpdated) {
    String data = "";
    data = MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_STATUS;
    data += moduleStatus;
    sendBluetoothData(data);
    moduleStatusUpdated = false;
  }
}

void asciiToChar(char asciiInt) {

}
