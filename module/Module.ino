#include <SoftwareSerial.h>
#include <Wire.h>
#include <Data.h>
#include <timer.h>

#define MODULE_NAME "Test Module 1"
#define MODULE_CREATOR "Li Kwok Shing"

#define L9110S_A_1B 8
#define L9110S_A_1A 9
#define L9110S_B_1B 10
#define L9110S_B_1A 11

char requestType;
char request;
bool responsed = true;
char responseWith;

bool attackReady = true;
int attackTimeout = 3000;
unsigned long time_now = 0;

Timer cd;
Timer waeponRestore;
Timer stopMotor;

char moduleStatus = MODULE_DATA_MODULE_STATUS_ATTACK_READY ;
bool moduleStatusUpdated = false;

void setup() {
  setPinMode();

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(MODULE_ENABLE_PIN, OUTPUT);

  stopMotor.setTimeout(1000);
  stopMotor.setCallback(stop_L9110S);

  cd.setTimeout(3000);
  cd.setCallback(cdEnd);

  waeponRestore.setTimeout(500);
  waeponRestore.setCallback(moduleRestore);

  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(requestReceived);
  Serial.begin(115200);

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

void setPinMode() {
  pinMode(L9110S_A_1B, OUTPUT);
  pinMode(L9110S_A_1A, OUTPUT);
  pinMode(L9110S_B_1B, OUTPUT);
  pinMode(L9110S_B_1A, OUTPUT);
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
      Wire.beginTransmission(MASTER_ADDRESS);
      Wire.write(MODULE_DATA_MODULE_INFO_NAME);
      Wire.write(MODULE_NAME);
      Wire.endTransmission();
      Serial.print("Name responsed: ");
      Serial.println(MODULE_NAME);
      break;
    case MODULE_DATA_MODULE_INFO_CREATOR:
      Serial.println("Creator Requested");
      Wire.beginTransmission(MASTER_ADDRESS);
      Wire.write(MODULE_DATA_MODULE_INFO_CREATOR);
      Wire.write(MODULE_CREATOR);
      Wire.endTransmission();
      Serial.print("Creaotr responsed: ");
      Serial.println(MODULE_CREATOR);
      break;
    case MODULE_DATA_MODULE_STATUS:
      Serial.println("Status Requested");
      Wire.beginTransmission(MASTER_ADDRESS);
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
  Wire.beginTransmission(MASTER_ADDRESS);
  Wire.write(MODULE_DATA_MODULE_STATUS);
  Wire.write(moduleStatus);
  Wire.endTransmission();
}
void getModuleControl(char control) { //
  switch (control) {
    case MODULE_DATA_MODULE_ACTION_UP:
      moduleUp();
      break;
    case MODULE_DATA_MODULE_ACTION_DOWN:
      moduleDown();
      break;
    case MODULE_DATA_MODULE_ACTION_LEFT:
      moduleLeft();
      break;
    case MODULE_DATA_MODULE_ACTION_RIGHT:
      moduleRight();
      break;
    case MODULE_DATA_MODULE_ACTION_ATTACK:
      if (attackReady) {
        Serial.println("Module Attack");
        moduleAttack();
        attackReady = false;
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

void moduleUp() {
  Serial.println("Module Up");
}

void moduleDown() {
  Serial.println("Module Down");
}

void moduleLeft() {
  Serial.println("Module Left");
}

void moduleRight() {
  Serial.println("Module Right");
}

void moduleAttack() {
  analogWrite(L9110S_A_1A, HIGH);
  analogWrite(L9110S_A_1B, LOW);
  analogWrite(L9110S_B_1A, HIGH);
  analogWrite(L9110S_B_1B, LOW);
  stopMotor.start();
  waeponRestore.start();
}

void moduleRestore() {
  analogWrite(L9110S_A_1A, LOW);
  analogWrite(L9110S_A_1B, HIGH);
  analogWrite(L9110S_B_1A, LOW);
  analogWrite(L9110S_B_1B, HIGH);
  stopMotor.start();
}

void stop_L9110S() {
  analogWrite(L9110S_A_1B, 0);
  analogWrite(L9110S_A_1A, 0);
  analogWrite(L9110S_B_1B, 0);
  analogWrite(L9110S_B_1A, 0);
}
