#include <SoftwareSerial.h>
#include <Wire.h>
#include <Data.h>
#include <timer.h>

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
#define BLUETOOTH_BUFFER_SIZE 9
#define BLUETOOTH_UPDATE_INTERVAL 5 //in mill-second

#define ROBOT_STATUS_UPDATE_INTERVAL 1000 //in mill-second

#define MAXIMUM_AMPLITUDE 255


Timer routine; //Createing a timer object called routine 
Timer recevier; //Create a timer object called recevier

SoftwareSerial Bluetooth(BLUETOOTH_TX, BLUETOOTH_RX);
char bluetoothBuffer[BLUETOOTH_BUFFER_SIZE];        //Empty bluetoothBuffer for data communication
String BluetoothData = "";

String robotStatus = "";                                    //Status of robot
bool robotStatusUpdated = false;
String moduleStatus = "";                                   //Status of robot
bool moduleStatusUpdated = false;
String moduleName = "";                                     //The name of attached module
bool moduleNameUpdated = false;
String moduleCreator = "";                                  //The creatot of attached module
bool moduleCreatorUpdated = false;

bool joystickModeXY = false;

void setup() {
  setPinMode();
  digitalWrite(BLUETOOTH_POWER, HIGH);

  Serial.begin(115200);

  Bluetooth.begin(BLUETOOTH_BAUDRATE);

  Wire.begin(ROBOT_I2C_ADDRESS);
  Wire.onReceive(ResponeReceived);

  routine.setInterval(ROBOT_STATUS_UPDATE_INTERVAL);
  routine.setCallback(routineTask);
  routine.start();

  recevier.setInterval(BLUETOOTH_UPDATE_INTERVAL);
  recevier.setCallback(getBlueToothData);
  recevier.start();

  if (moduleEnabled()) {
    Serial.println("Module Detacted ");
    Serial.println("Requesting Module Info");
    requestModuleInfo();
    delay(2000);
  }
}

void loop() {
  recevier.update();
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
    bluetoothBuffer[datasize] = Bluetooth.read();
    datasize++;
    if (bluetoothBuffer[0] == MOBILE_START_TRANSMIT) {
      while (Bluetooth.available())  {
        delay(1);
        bluetoothBuffer[datasize] = Bluetooth.read();
        datasize++;
        if (bluetoothBuffer[datasize] > 127 || datasize > BLUETOOTH_BUFFER_SIZE) {
          Serial.println("Communication error");
          break;     // Communication error
        }
        if (bluetoothBuffer[datasize - 1] == MOBILE_END_TRANSMIT) {
          break;     // Finish receive
        }
      }
    }
    for (int pointer = 0; pointer < datasize; pointer++) {
      Serial.print((char)bluetoothBuffer[pointer]);
    }
    Serial.print("\n");
    if (bluetoothBuffer[1] == ROBOT_DATA_JOYSTICK_CONTROL && datasize == 9) {
      setRobotControl(bluetoothBuffer);
    }
    if (bluetoothBuffer[1] == MODULE_DATA_MODULE_ACTION && datasize == 4) { //Module Control
      sendModuleRequest(bluetoothBuffer[1], bluetoothBuffer[2]);
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
  byte leftMix;
  byte rightMix;

  float leftMotorPower;
  float rightMotorPower;

  int XorAngle     = (data[2] - 48) * 100 + (data[3] - 48) * 10 + (data[4] - 48);
  int YorAmplitude = (data[5] - 48) * 100 + (data[6] - 48) * 10 + (data[7] - 48);
  
  Serial.print("Joystick Control received: ");
  Serial.print("X / Angle: ");
  Serial.print(XorAngle);
  Serial.print(", Y / YorAmplitude: ");
  Serial.println(YorAmplitude);

  if (joystickModeXY) { //andY mode
    int x = XorAngle;
    int y = YorAmplitude;

    if (x < 0 || x > MAXIMUM_AMPLITUDE || y < 0 || y > MAXIMUM_AMPLITUDE) {
      return; //Data Error
    }

    x = map(x, 0, MAXIMUM_AMPLITUDE, -255, 255);
    y = map(y, MAXIMUM_AMPLITUDE, 0, -255, 255);

    leftMotorPower = y + x;
    rightMotorPower = y - x;

    float d = abs( abs(y) - abs(x) );
    leftMotorPower = leftMotorPower < 0 ? leftMotorPower - d : leftMotorPower + d;
    rightMotorPower = rightMotorPower < 0 ? rightMotorPower - d : rightMotorPower + d;

    leftMotorPower = map( leftMotorPower, -512, 512, -255, 255);
    rightMotorPower = map( rightMotorPower, -512, 512, -255, 255);

    if (leftMotorPower > 0) {
      digitalWrite(HBRIDGE_IN1, HIGH);
      digitalWrite(HBRIDGE_IN2, LOW);
    } else {
      digitalWrite(HBRIDGE_IN1, LOW);
      digitalWrite(HBRIDGE_IN2, HIGH);
    }
    if (rightMotorPower > 0) {
      digitalWrite(HBRIDGE_IN3, HIGH);
      digitalWrite(HBRIDGE_IN4, LOW);
    } else {
      digitalWrite(HBRIDGE_IN3, LOW);
      digitalWrite(HBRIDGE_IN4, HIGH);
    }

    leftMotorPower = abs(leftMotorPower);
    rightMotorPower = abs(rightMotorPower);

} else {//Angle and Amplitude mode
    int angle = XorAngle;
    int amplitude = YorAmplitude;

    if (angle < 0 || angle > 360 || amplitude < 0 || amplitude > MAXIMUM_AMPLITUDE) {
      return; //Data Error
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

    Serial.print("Left Motor Mix: ");
    Serial.print(leftMix);
    Serial.print(" Right Motor Mix: ");
    Serial.println(rightMix);

    leftMotorPower = ((float)leftMix * (float)amplitude) / (float)255;
    rightMotorPower = ((float)rightMix * (float)amplitude) / (float)255;
  }

  Serial.print("Left Motor Power: ");
  Serial.print((float)leftMotorPower);
  Serial.print(" Right Motor Power: ");
  Serial.println((float)rightMotorPower);

  // Prevent buzzing at low speeds (Adjust according to your motors.
  if (leftMotorPower < 10) {
    leftMotorPower = 0;
  }
  if (rightMotorPower < 10) {
    rightMotorPower = 0;
  }

  analogWrite(LEFT_MOTOR, (float)leftMotorPower);
  analogWrite(RIGHT_MOTOR, (float)rightMotorPower);
}

void sendModuleRequest(char requestType, char controlChar) {
  Wire.beginTransmission(MODULE_I2C_ADDRESS);
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
  Serial.println("Module Name requesting");
  sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_CREATOR);
  Serial.println("Module Creator requesting...");
}

void requestModuleStatus() {
  sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_STATUS);
  Serial.println("Module Status requesting...");
}

bool moduleEnabled() {
  return digitalRead(MODULE_ENABLE_PIN);
}

void routineTask() {
  if (!moduleEnabled()) { //TODO: add timeout to this condition
    Serial.println("Module is not attached or get disabled!");

    String data = "";
    data = MODULE_DATA_MODULE_INFO;
    data += MODULE_DATA_MODULE_STATUS;
    data += MODULE_DATA_MODULE_STATUS_MODULE_DISABLE;
    sendBluetoothData(data);
  }
  if (moduleEnabled()) {
    if (moduleName == "") {
      Serial.println("No name recevied!");
      sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_NAME);
      Serial.println("Module Name requesting...");
    }
    if (moduleCreator == "") {
      Serial.println("No creator recevied!");
      sendModuleRequest(DATA_TYPE_REQUEST, MODULE_DATA_MODULE_INFO_CREATOR);
      Serial.println("Module Creator requesting...");
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
