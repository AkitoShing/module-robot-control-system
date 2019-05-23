#include <SoftwareSerial.h>
#include <Wire.h>
#include <Data.h>
#include <timer.h>

/* ---------- Custom Code Here ---------- */

#define L9110S_A_1B 8
#define L9110S_A_1A 9
#define L9110S_B_1B 10
#define L9110S_B_1A 11

Timer stopMotor;

/* ---------- Custom Code End ---------- */

/* -------------------------------------------------- */
/* You can change the module infomation here          */
/* The information will be sent back to the robot     */
/* and show in the application                        */
#define MODULE_NAME "Flip Flipper" //Modifiable
#define MODULE_CREATOR "MRCS Team" //Modifiable
/* -------------------------------------------------- */

#define ATTACK_TIMEOUT 3000
#define MODULE_RESET_DELAY 800

char requestType;
char request;
bool responsed = true;
char responseWith;

bool attackReady = true;
unsigned long time_now = 0;

char moduleStatus;
bool moduleStatusUpdated = false;

Timer cd;
Timer waeponRestore;

RobotModule module;

void setup() {
  coustomSetup();
  setPinMode();

  cd.setTimeout(ATTACK_TIMEOUT);
  cd.setCallback(cdEnd);

  waeponRestore.setTimeout(MODULE_RESET_DELAY);
  waeponRestore.setCallback(restore);

  Wire.begin(MODULE_I2C_ADDRESS);
  Wire.onReceive(requestReceived);
  Serial.begin(SOFTWARE_SERIAL_BUADRATE);

  Serial.print("Module Name: ");
  Serial.println(MODULE_NAME);
  Serial.print("Module Creator: ");
  Serial.println(MODULE_CREATOR);
  delay(10);

  restore();
  cdEnd();
  delay(500);

  Serial.println("Setup Finish......");
  digitalWrite(MODULE_ENABLE_PIN, HIGH);
}

void loop() {
  stopMotor.update();
  cd.update();
  waeponRestore.update();

  if (!responsed) {
    response(responseWith);
    responsed = !responsed;
  }
  if (moduleStatusUpdated) {
    sendModuleStatus(moduleStatus);
    moduleStatusUpdated = false;
  }
  if (moduleStatus = MODULE_DATA_MODULE_STATUS_ATTACK_READY) {
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
  } else if (moduleStatus = MODULE_DATA_MODULE_STATUS_ATTACK_CD) {
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
  }
}

void coustomSetup(){
  //Insert code that only run single time like you normal do in setup
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  stopMotor.setTimeout(300);
  stopMotor.setCallback(stop_L9110S);
}

void setPinMode() {
  pinMode(L9110S_A_1B, OUTPUT);
  pinMode(L9110S_A_1A, OUTPUT);
  pinMode(L9110S_B_1B, OUTPUT);
  pinMode(L9110S_B_1A, OUTPUT);
  pinMode(MODULE_ENABLE_PIN, OUTPUT);
}

void requestReceived(int count) { //onReceive
  requestType = Wire.read();
  request = Wire.read();
  while (Wire.available()) Wire.read();

  #ifdef DEBUG
    Serial.print("Action Received Type: ");
    Serial.print(requestType);
    Serial.print(" Data: ");
    Serial.println(request);
  #endif

  switch (requestType) {
    case DATA_TYPE_REQUEST:
      responseWith = request;
      responsed = false;
      break;
    case MODULE_DATA_MODULE_ACTION:
      getModuleControl(request);
      break;
  }
}

void response(char request) {
  switch (request) {
    case MODULE_DATA_MODULE_INFO_NAME:
      sendModuleName();
      break;
    case MODULE_DATA_MODULE_INFO_CREATOR:
      sendModuleCreator();
      break;
    case MODULE_DATA_MODULE_STATUS:
      sendModuleStatus();
      break;
  }
}

void sendModuleName () {
  sendData(MODULE_DATA_MODULE_INFO_NAME, MODULE_NAME);
}

void sendModuleCreator() {
  sendData(MODULE_DATA_MODULE_INFO_CREATOR, MODULE_CREATOR);
}

void sendModuleStatus () {
  sendData(MODULE_DATA_MODULE_STATUS, moduleStatus);
}

void sendData (char dataType, char data) {
  Wire.beginTransmission(ROBOT_I2C_ADDRESS);
  Wire.write(dataType);
  Wire.write(data);
  Wire.endTransmission();
}

void getModuleControl(char control) { //
  switch (control) {
    case MODULE_DATA_MODULE_ACTION_UP:
      Serial.println("Module Up");
      moduleUp();
      break;
    case MODULE_DATA_MODULE_ACTION_DOWN:
      Serial.println("Module Down");
      moduleDown();
      break;
    case MODULE_DATA_MODULE_ACTION_LEFT:
      Serial.println("Module Left");
      moduleLeft();
      break;
    case MODULE_DATA_MODULE_ACTION_RIGHT:
      Serial.println("Module Right");
      moduleRight();
      break;
    case MODULE_DATA_MODULE_ACTION_ATTACK:
      if (attackReady) {
        Serial.println("Module Attack");
        moduleAttack();
        attackReady = false;
        waeponRestore.start();
        cd.start();
      }
      Serial.println("Attack CD");
      moduleStatus = MODULE_DATA_MODULE_STATUS_ATTACK_CD;
      moduleStatusUpdated = true;
      break;
    default:
      break;
  }
}

void cdEnd() {
  attackReady = true;
  Serial.println("Attack Ready");
  sendModuleStatus(MODULE_DATA_MODULE_STATUS_ATTACK_READY);
}

void restore(){
  Serial.println("restore");
  moduleRestore();
}

void moduleUp() {
  //Insert code here to control the module like pointing the weapon upward
  //This method be called when the module recevied a 'MODULE_DATA_MODULE_ACTION_UP' command from the robot
}

void moduleDown() {
  //Insert code here to control the module like pointing the weapon downward
  //This method be called when the module recevied a 'MODULE_DATA_MODULE_ACTION_DOWN' command from the robot
}

void moduleLeft() {
  //Insert code here to control the module like trun toward Left
  //This method be called when the module recevied a 'MODULE_DATA_MODULE_ACTION_LEFT' command from the robot
}

void moduleRight() {
  //Insert code here to control the module like trun toward right
  //This method be called when the module recevied a 'MODULE_DATA_MODULE_ACTION_RIGHT' command from the robot
}

void moduleAttack() {
  //Insert code here to control the weapon
  //This method will be called when the module recevied a 'MODULE_DATA_MODULE_ACTION_ATTACK' command from the robot
}

void moduleRestore() {
  //Insert code here to restore the module after it action is profrommed
  //This method will be called after the module profromed it action and some delay
  //The delay can be configured in the define section at the top of this file
  //The constant is named 'MODULE_RESET_DELAY' default => 800ms
}