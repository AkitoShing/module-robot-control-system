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
#define MODULE_NAME "Test Module 1"
#define MODULE_CREATOR "Li Kwok Shing"
/* -------------------------------------------------- */

#define ATTACK_TIMEOUT 3000
#define MODULE_RESET_DELAY 800

char requestType;
char request;
bool responsed = true;
char responseWith;

bool attackReady = true;
unsigned long time_now = 0;

char moduleStatus = MODULE_DATA_MODULE_STATUS_ATTACK_READY ;
bool moduleStatusUpdated = false;

Timer cd;
Timer waeponRestore;

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

  Serial.print("Action Received Type: ");
  Serial.print(requestType);
  Serial.print(" Data: ");
  Serial.println(request);

  switch (requestType) {
    case DATA_TYPE_REQUEST:
      Serial.println("Request recevied");
      responseWith = request;
      responsed = false;
      break;
    case MODULE_DATA_MODULE_ACTION:
      Serial.println("Control recevied");
      getModuleControl(request);
      break;
    default:
      break;
  }
}

void response(char request) {
  switch (request) {
    case MODULE_DATA_MODULE_INFO_NAME:
      Serial.println("Name Requested");
      Wire.beginTransmission(ROBOT_I2C_ADDRESS);
      Wire.write(MODULE_DATA_MODULE_INFO_NAME);
      Wire.write(MODULE_NAME);
      Wire.endTransmission();
      Serial.print("Name responsed: ");
      Serial.println(MODULE_NAME);
      break;
    case MODULE_DATA_MODULE_INFO_CREATOR:
      Serial.println("Creator Requested");
      Wire.beginTransmission(ROBOT_I2C_ADDRESS);
      Wire.write(MODULE_DATA_MODULE_INFO_CREATOR);
      Wire.write(MODULE_CREATOR);
      Wire.endTransmission();
      Serial.print("Creaotr responsed: ");
      Serial.println(MODULE_CREATOR);
      break;
    case MODULE_DATA_MODULE_STATUS:
      Serial.println("Status Requested");
      Wire.beginTransmission(ROBOT_I2C_ADDRESS);
      Wire.write(MODULE_DATA_MODULE_STATUS);
      Wire.write(moduleStatus);
      Wire.endTransmission();
      Serial.print("Status responsed: ");
      Serial.println(moduleStatus);
      break;
    default:
      break;
  }
}
void sendModuleStatus (char moduleStatus) {
  Wire.beginTransmission(ROBOT_I2C_ADDRESS);
  Wire.write(MODULE_DATA_MODULE_STATUS);
  Wire.write(moduleStatus);
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

void cdEnd () {
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
  digitalWrite(L9110S_A_1A, HIGH);
  digitalWrite(L9110S_A_1B, LOW);
  digitalWrite(L9110S_B_1A, HIGH);
  digitalWrite(L9110S_B_1B, LOW);
  stopMotor.start();
}

void moduleRestore() {
  //Insert code here to restore the module after it action is profrommed
  //This method will be called after the module profromed it action and some delay
  //The delay can be configured in the define section at the top of this file
  //The constant is named 'MODULE_RESET_DELAY' default => 800ms
  digitalWrite(L9110S_A_1A, LOW);
  digitalWrite(L9110S_A_1B, HIGH);
  digitalWrite(L9110S_B_1A, LOW);
  digitalWrite(L9110S_B_1B, HIGH);
  stopMotor.start();
}

void stop_L9110S() {
  Serial.println("stop motor");
  digitalWrite(L9110S_A_1B, 0);
  digitalWrite(L9110S_A_1A, 0);
  digitalWrite(L9110S_B_1B, 0);
  digitalWrite(L9110S_B_1A, 0);
}
