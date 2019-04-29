# 1 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/robot/Data.h"
# 1 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/robot/Data.h"
# 30 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/robot/Data.h"
//Wire I2C Communication
# 1 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/robot/Robot.ino"
# 2 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/robot/Robot.ino" 2
# 3 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/robot/Robot.ino" 2
# 4 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/robot/Robot.ino" 2
# 5 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/robot/Robot.ino" 2

//H-bridge







//Bluetooth Communi cation






Timer routine;

SoftwareSerial Bluetooth(2, 3);
char BluetoothBuffer[9]; //Empty BluetoothBuffer for data communication
String BluetoothData = "";

String robotStatus = ""; //Status of robot
bool robotStatusUpdated = false;
String moduleStatus = ""; //Status of robot
bool moduleStatusUpdated = false;
String moduleName = ""; //The name of attached module
bool moduleNameUpdated = false;
String moduleCreator = ""; //The creatot of attached module
bool moduleCreatorUpdated = false;

void setup() {
  setPinMode();
  digitalWrite(5, 0x1);

  Serial.begin(115200);

  Bluetooth.begin(9600);

  Wire.begin(25);
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
  pinMode(10, 0x1);
  pinMode(11, 0x1);
  pinMode(8, 0x1);
  pinMode(9, 0x1);
  pinMode(12, 0x1);
  pinMode(13, 0x1);
  pinMode(7, 0x0);
  pinMode(4, 0x0);
  pinMode(5, 0x1);
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
      while (Bluetooth.available()) {
        delay(1);
        BluetoothBuffer[datasize] = Bluetooth.read();
        datasize++;
        if (BluetoothBuffer[datasize] > 127 || datasize > 8) {
          break; // Communication error
        }
        if (BluetoothBuffer[datasize - 1] == '}' ||
            BluetoothBuffer[datasize - 1] == ')' ||
            BluetoothBuffer[datasize - 1] == '>') {
          break; // Finish receive
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
    if ((BluetoothBuffer[1] == '?' || BluetoothBuffer[1] == '@' ) && datasize == 4) { //Module Control
      sendModuleRequest(BluetoothBuffer[1], BluetoothBuffer[2]);
    }
  }
}



void sendBluetoothData(String data) { //TODO:
  String BluetoothData = "";
  BluetoothData += '(';
  BluetoothData += data;
  BluetoothData += ')';
  Bluetooth.print(BluetoothData);
  Serial.print("Sended Bluetooth Data: ");
  Serial.println(data);
  delay(2);
}

void setRobotControl(char data[9]) {
  Serial.println("set Robot control");
  int angle = (data[2] - 48) * 100 + (data[3] - 48) * 10 + (data[4] - 48); // obtain the Int from the ASCII representation
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
    digitalWrite(8, 0x0);
    digitalWrite(9, 0x1);
    digitalWrite(12, 0x1);
    digitalWrite(13, 0x0);
    leftMix = 255;
    rightMix = 255;
  }
  if (angle >= 0 && angle < 11 || angle > 350 && angle < 360) {
    // Serial.print("Right spin");
    digitalWrite(8, 0x1);
    digitalWrite(9, 0x0);
    digitalWrite(12, 0x0);
    digitalWrite(13, 0x1);
    leftMix = 255;
    rightMix = 255;
  }
  if (angle > 10 && angle < 171) {
    // Serial.print("Forward ");
    digitalWrite(8, 0x1);
    digitalWrite(9, 0x0);
    digitalWrite(12, 0x1);
    digitalWrite(13, 0x0);
    leftMix = map(angle, 90, 170, 255, 0);
    if (angle < 90) leftMix = 255;
    rightMix = map(angle, 11, 90, 0, 255);
    if (angle > 90) rightMix = 255;
  }
  if (angle > 189 && angle < 351) {
    // Serial.print("Backward ");
    digitalWrite(8, 0x0);
    digitalWrite(9, 0x1);
    digitalWrite(12, 0x0);
    digitalWrite(13, 0x1);
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

  analogWrite(10, (float)leftMotorPower);
  analogWrite(11, (float)rightMotorPower);
}

void sendModuleRequest(char requestType, char controlChar) {
  Wire.beginTransmission(42);
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
    case '%':
      moduleName = data;
      Serial.print("Name recevied: ");
      Serial.println(moduleName);
      moduleNameUpdated = true;
      break;

    case '^':
      moduleCreator = data;
      Serial.print("Creator recevied: ");
      Serial.println(moduleCreator);
      moduleCreatorUpdated = true;
      break;

    case '$':
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
  sendModuleRequest('?', '%');
  Serial.println("Name requested");
  sendModuleRequest('?', '^');
  Serial.println("Creator requested");
}

void requestModuleStatus() {
  sendModuleRequest('?', '$');
  Serial.println("Module Status requested");
}

bool moduleEnabled() {
  return digitalRead(7);
}

void routineTask() {
  if (!moduleEnabled()) { //TODO: add timeout to this condition
    String data = "";
    data = '#';
    data += '$';
    data += 'o';
    sendBluetoothData(data);
    Serial.println(data);
  }
  if (!moduleEnabled()) {
    if (moduleName = "") {
      Serial.println("No name recevied!");
      sendModuleRequest('?', '%');
      Serial.println("Name requested");
    }
    if (moduleCreator = "") {
      Serial.println("No creator recevied!");
      sendModuleRequest('?', '^');
      Serial.println("Creator requested");
    }
  }
  if (moduleNameUpdated) {
    String data = "";
    data += '#';
    data += '%';
    data += moduleName;
    sendBluetoothData(data);
    moduleNameUpdated = false;
  }
  if (moduleCreatorUpdated) {
    String data = "";
    data += '#';
    data += '^';
    data += moduleCreator;
    sendBluetoothData(data);
    moduleCreatorUpdated = false;
  }
  if (moduleStatusUpdated) {
    String data = "";
    data = '#';
    data += '$';
    data += moduleStatus;
    sendBluetoothData(data);
    moduleStatusUpdated = false;
  }
}

void asciiToChar(char asciiInt) {

}
