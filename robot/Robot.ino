#include <SoftwareSerial.h>
#include <Wire.h>
#include <Data.h>

//H-bridge
#define HBRIDGE_IN1 8
#define HBRIDGE_IN2 9
#define LEFT_MOTOR 10
#define HBRIDGE_IN3 12
#define HBRIDGE_IN4 13
#define RIGHT_MOTOR 11

//blueTooth Communication
#define BLUE_TOOTH_BAUDRATE 9600
#define BLUE_TOOTH_TX 2
#define BLUE_TOOTH_RX 3

SoftwareSerial blueTooth(BLUE_TOOTH_TX, BLUE_TOOTH_RX);
byte blueToothBuffer[8] ;         //Empty blueToothBuffer for data communication
String blueToothData = "";

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

  Serial.begin(115200);

  blueTooth.begin(BLUE_TOOTH_BAUDRATE);

  Wire.begin(MASTER_ADDRESS);
  Wire.onReceive(ResponeReceived);

  if (moduleEnabled()){
    Serial.println("Module Detacted ");
    Serial.println("Requesting Module Info");
    requestModuleInfo();
    delay(2000);
  }
}

void loop(){
  getBlueToothData();
  if (!moduleEnabled()){
    String data = "";
    data = MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_STATUS;
    data += MODULE_DATA_MODULE_STATUS_MODULE_DISABLE;
//    sendblueToothData(data);
    Serial.println(data);
  }
  if (moduleNameUpdated) {
    String data = "";
    data += MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_INFO_NAME;
    data += moduleName;
    sendblueToothData(data);
    moduleNameUpdated = false;
  }
  if (moduleCreatorUpdated) {
    String data = "";
    data += MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_INFO_CREATOR;
    data += moduleCreator;
    sendblueToothData(data);
    moduleCreatorUpdated = false;
  }
  if (moduleStatusUpdated) {
    String data = "";
    data = MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_STATUS;
    data += moduleStatus;
    sendblueToothData(data);
    moduleStatusUpdated = false;
  }
  
//  if (moduleName = "") {
//    Serial.println("No name recevied!");
//    sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_NAME);
//    Serial.println("Name requested");
//  }
//  if (moduleCreator = "") {
//    Serial.println("No creator recevied!");
//    sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_CREATOR);
//    Serial.println("Creator requested");
//  }
  
}

void setPinMode() {
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);
  pinMode(HBRIDGE_IN1, OUTPUT);
  pinMode(HBRIDGE_IN2, OUTPUT);
  pinMode(HBRIDGE_IN3, OUTPUT);
  pinMode(HBRIDGE_IN4, OUTPUT);
  pinMode(MODULE_ENABLE_PIN, INPUT);
}

void getBlueToothData() {
  if (blueTooth.available()) { // data received from smartphone
    int size = 0;
    blueToothBuffer[size] = blueTooth.read();
    if (blueToothBuffer[0] = MOBILE_START_TRANSMIT){
      while (blueTooth.available()) {
        blueToothBuffer[size] = blueTooth.read();
        if (blueToothBuffer[size] = MOBILE_END_TRANSMIT) {
          break;
        }
        size++;
      }
      if (blueToothBuffer[1] == ROBOT_DATA_JOYSTICK_CONTROL && size == 8) { //joystick Control
        Serial.print("Joystick Control received: ");
        setRobotControl(blueToothBuffer);
      }
      if ((blueToothBuffer[1] == DATA_TYPE_REQUEST || blueToothBuffer[1] == MODULE_DATA_MODULE_ACTION ) && size == 4) { //Module Control
        sendModuleRequest(blueToothBuffer[1], blueToothBuffer[2]);
      }
    }
  }
  while(blueTooth.available()){
    blueTooth.read();
  }
}

void sendblueToothData(String data) { //TODO:
  String blueToothData = "";
  blueToothData += ROBOT_START_TRANSMIT;
  blueToothData += data;
  blueToothData += ROBOT_END_TRANSMIT;
  blueTooth.print(blueToothData);
  Serial.print("Sended Bluetooth Data: ");
  Serial.println(data);
  delay(2);
}

void setRobotControl(byte data[8]) {
  int angle     = (data[1] - 48) * 100 + (data[2] - 48) * 10 + (data[3] - 48); // obtain the Int from the ASCII representation
  int amplitube = (data[4] - 48) * 100 + (data[5] - 48) * 10 + (data[6] - 48);
  
  Serial.print("Angle: ");
  Serial.print(angle);
  Serial.print(" Amplitube: ");
  Serial.println(amplitube);

  if (angle < 0 || angle > 360 || amplitube < 0 || amplitube > 100) {
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
    if (angle < 90) leftMix = 255;
    leftMix = map(angle, 90, 170, 255, 0);
    if (angle > 90) rightMix = 255;
    rightMix = map(angle, 11, 90, 0, 255);
  }
  if (angle > 189 && angle < 351) {
    // Serial.print("Backward ");
    digitalWrite(HBRIDGE_IN1, LOW);
    digitalWrite(HBRIDGE_IN2, HIGH);
    digitalWrite(HBRIDGE_IN3, LOW);
    digitalWrite(HBRIDGE_IN4, HIGH);
    if (angle < 270) leftMix = 255;
    leftMix = map(angle, 270, 351, 255, 0);
    if (angle > 270) rightMix = 255;
    rightMix = map(angle, 189, 270, 0, 255);
  }

  Serial.print("Left Motor Mix: ");
  Serial.print(leftMix);
  Serial.print(" Right Motor Mix: ");
  Serial.println(rightMix);

  //TODO: Test the new calculation
  int leftMotorPower = (leftMix * amplitube) / 100;
  int rightMotorPower = (rightMix * amplitube) / 100;

  Serial.print("Left Motor Power: ");
  Serial.print(leftMotorPower);
  Serial.print(" Right Motor Power: ");
  Serial.println(rightMotorPower);

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
