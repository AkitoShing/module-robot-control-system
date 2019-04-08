#include <SoftwareSerial.h>
#include <Wire.h>
#include <Data.h>

//Data Communication
#define STX 0x02 //ASSCII representation of "Start of text"
#define ETX 0x03 //ASSCII representation of "End of text"
#define DC1 0x11
#define DC2 0x12
#define DC3 0x13

//H-bridge
#define IN1 8
#define IN2 9
#define LEFT_MOTOR 10
#define IN3 12
#define IN4 13

#define RIGHT_MOTOR 11

//blueTooth Communication
#define BLUE_TOOTH_BAUDRATE 9600
#define BLUE_TOOTH_TX 2
#define BLUE_TOOTH_RX 3

SoftwareSerial blueTooth(BLUE_TOOTH_TX, BLUE_TOOTH_RX);
byte blueToothBuffer[8] ;         //Empty blueToothBuffer for data communication

byte moduleControlStatus = 0;                               //Store moduleControl status
String robotStatus = "";                                    //Status of robot
String moduleStatus = "";                                   //Status of robot
String moduleName = "";                                     //The name of attached module
String moduleCreator = "";                                  //The creatot of attached module
String blueToothData = "";

void setup() {
  setPinMode();

  Serial.begin(9600);

  blueTooth.begin(BLUE_TOOTH_BAUDRATE);

  Wire.begin(MASTER_ADDRESS);
  Wire.onReceive(ResponeReceived);

  requestModuleInfo();
  delay(2000);
}

void loop(){
  
}

void setPinMode() {
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void getBlueToothData() {
  if (blueTooth.available()) { // data received from smartphone
    int size = 0;
    while (blueTooth.available()) {
      blueToothBuffer[size] = blueTooth.read();
      size++;
    }
    if (blueToothBuffer[0] == ROBOT_JOYSTICK_CONTROL && size == 7) { //joystick Control
      setRobotControl(blueToothBuffer);
    }
    if ((blueToothBuffer[0] == MODULE_DATA_TYPE_REQUEST || blueToothBuffer[0] == MODULE_DATA_TYPE_REQUEST ) && i == 2) { //Module Control
      sendModuleRequest(MODULE_DATA_TYPE_CONTROL, blueToothBuffer[1]);
    }
    while(blueTooth.available()){
      blueTooth.read();
    }
  }
}

void sendblueToothData(String data) { //TODO:
  blueTooth.print(data);
  Serial.print("Sended Bluetooth Data: ");
  Serial.println(data);
  delay(2);
}

void setRobotControl(byte data[8]) {
  int angle     = (data[1] - 48) * 100 + (data[2] - 48) * 10 + (data[3] - 48); // obtain the Int from the ASCII representation
  int amplitube = (data[4] - 48) * 100 + (data[5] - 48) * 10 + (data[6] - 48);

  if (angle < 0 || angle > 360 || amplitube < 0 || amplitube > 100) {
    return; //Data Error
  }

  if (angle > 170 && angle < 190) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    // Serial.print("Left spin ");
  }
  if (angle >= 0 && angle < 11 || angle > 350 && angle < 360) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    // Serial.print("Right spin");
  }
  if (angle > 10 && angle < 171) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    // Serial.print("Forward ");
  }
  if (angle > 189 && angle < 351) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    // Serial.print("Backward ");
  }
  //FIXME: fix the calculation
  float leftMotorPower  = (float)amplitube * (abs((float)180 - (float)angle) / (float)180);
  float rightMotorPower = (float)amplitube * (float)1 - (abs((float)angle - (float)180) / (float)180);

  int int_leftMotorPower  = map(leftMotorPower, 0, 50, 0, 255);
  int int_rightMotorPower = map(rightMotorPower, 0, 50, 0, 255);

  // Serial.print("Left Motor Power: ");
  // Serial.print(int_leftMotorPower);
  // Serial.print(" ");
  // Serial.print("Right Motor Power: ");
  // Serial.println(int_rightMotorPower);

  analogWrite(LEFT_MOTOR, leftMotorPower);
  analogWrite(RIGHT_MOTOR, rightMotorPower);
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
  if (Wire.available()) {
    responeType = Wire.read();
    Serial.print("Respone recevied: ");
    Serial.println(responeType);
  }
  switch (responeType) {
    case MODULE_DATA_MODULE_NAME:
      moduleName = "";
      while (Wire.available()) {
        moduleName += (char)Wire.read();
      }
      Serial.print("Name recevied: ");
      Serial.println(moduleName);
      break;
    case MODULE_DATA_MODULE_CREATOR:
      moduleCreator = "";
      while (Wire.available()) {
        moduleCreator += (char)Wire.read();
      }
      Serial.print("Creator recevied: ");
      Serial.println(moduleCreator);
      break;
    case MODULE_DATA_MODULE_STATUS:
      moduleStatus = "";
      while (Wire.available()) {
        moduleStatus += (char)Wire.read();
      }
      Serial.print("Status recevied: ");
      Serial.println(moduleStatus);
      break;
    default:
      break;
  }
  while (Wire.available()) {
    Serial.print(Wire.read());
  }
}

void requestModuleInfo() {
  String data = "";
  Serial.println("Requesting Module Info");
  sendModuleRequest(MODULE_DATA_TYPE_REQUEST, MODULE_DATA_MODULE_NAME);
  Serial.println("Name requested");
  delay(1000);
  sendModuleRequest(MODULE_DATA_TYPE_REQUEST, MODULE_DATA_MODULE_CREATOR);
  Serial.println("Creator requested");
  delay(1000);
  data = MODULE_DATA_MODULE_INFO;
  data += MODULE_DATA_MODULE_NAME;
  data += moduleName;
  sendblueToothData(data);
  data = MODULE_DATA_MODULE_INFO;
  data += MODULE_DATA_MODULE_CREATOR;
  data += moduleCreator;
  sendblueToothData(data);
}

void requestModuleStatus() {
  sendModuleRequest(MODULE_DATA_TYPE_REQUEST, MODULE_DATA_MODULE_STATUS);
}
