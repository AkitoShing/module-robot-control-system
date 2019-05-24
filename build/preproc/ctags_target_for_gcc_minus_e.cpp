# 1 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/Robot/Robot.ino"
# 1 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/Robot/Robot.ino"
# 2 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/Robot/Robot.ino" 2
# 3 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/Robot/Robot.ino" 2
# 4 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/Robot/Robot.ino" 2
# 5 "/Users/akitoshing/Documents/GitHub/module-robot-control-system/Robot/Robot.ino" 2

//H-bridge







//Bluetooth Communi cation






Timer routine;
Timer recevier;

SoftwareSerial Bluetooth(2, 3);
char bluetoothBuffer[9]; //Empty bluetoothBuffer for data communication
String BluetoothData = "";

String robotStatus = ""; //Status of robot
bool robotStatusUpdated = false;
String moduleStatus = ""; //Status of robot
bool moduleStatusUpdated = false;
String moduleName = ""; //The name of attached module
bool moduleNameUpdated = false;
String moduleCreator = ""; //The creatot of attached module
bool moduleCreatorUpdated = false;

bool joystickModeXY = false;

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

  recevier.setInterval(5);
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
    bluetoothBuffer[datasize] = Bluetooth.read();
    datasize++;
    if (bluetoothBuffer[0] == '{') {
      while (Bluetooth.available()) {
        delay(1);
        bluetoothBuffer[datasize] = Bluetooth.read();
        datasize++;
        if (bluetoothBuffer[datasize] > 127 || datasize > 9) {
          Serial.println("Communication error");
          break; // Communication error
        }
        if (bluetoothBuffer[datasize - 1] == '}') {
          break; // Finish receive
        }
      }
    }
    for (int pointer = 0; pointer < datasize; pointer++) {
      Serial.print((char)bluetoothBuffer[pointer]);
    }
    Serial.print("\n");
    if (bluetoothBuffer[1] == '&' && datasize == 9) {
      setRobotControl(bluetoothBuffer);
    }
    if (bluetoothBuffer[1] == '@' && datasize == 4) { //Module Control
      sendModuleRequest('@', bluetoothBuffer[2]);
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
  byte leftMix;
  byte rightMix;

  float leftMotorPower;
  float rightMotorPower;

  int XorAngle = (data[2] - 48) * 100 + (data[3] - 48) * 10 + (data[4] - 48);
  int YorAmplitude = (data[5] - 48) * 100 + (data[6] - 48) * 10 + (data[7] - 48);

  Serial.print("Joystick Control received: ");
  Serial.print("X / Angle: ");
  Serial.print(XorAngle);
  Serial.print(" Y / YorAmplitude: ");
  Serial.println(YorAmplitude);

  if (joystickModeXY) { //andY mode
    int x = XorAngle;
    int y = YorAmplitude;

    if (x < 0 || x > 255 || y < 0 || y > 255) {
      return; //Data Error
    }

    x = map(x, 0, 255, -255.0, 255.0);
    y = map(y, 255, 0, -255.0, 255.0);

    leftMotorPower = y + x;
    rightMotorPower = y - x;

    float d = abs( abs(y) - abs(x) );
    leftMotorPower = leftMotorPower < 0 ? leftMotorPower - d : leftMotorPower + d;
    rightMotorPower = rightMotorPower < 0 ? rightMotorPower - d : rightMotorPower + d;

    leftMotorPower = map( leftMotorPower, -512, 512, -255, 255);
    rightMotorPower = map( rightMotorPower, -512, 512, -255, 255);

    if (leftMotorPower > 0) {
      digitalWrite(8, 0x1);
      digitalWrite(9, 0x0);
    } else {
      digitalWrite(8, 0x0);
      digitalWrite(9, 0x1);
    }
    if (rightMotorPower > 0) {
      digitalWrite(12, 0x1);
      digitalWrite(13, 0x0);
    } else {
      digitalWrite(12, 0x0);
      digitalWrite(13, 0x1);
    }

    leftMotorPower = abs(leftMotorPower);
    rightMotorPower = abs(rightMotorPower);
} else {//Angle and Amplitude mode
    int angle = XorAngle;
    int amplitude = YorAmplitude;

    if (angle < 0 || angle > 360 || amplitude < 0 || amplitude > 255) {
      return; //Data Error
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
  if (moduleEnabled()) {
    if (moduleName == "") {
      Serial.println("No name recevied!");
      sendModuleRequest('?', '%');
      Serial.println("Name requested");
    }
    if (moduleCreator == "") {
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
