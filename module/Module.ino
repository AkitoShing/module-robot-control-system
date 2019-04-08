#include <Data.h>
#include "Wire.h"
#include "SoftwareSerial.h"

#define MODULE_NAME "Test Module 1"
#define MODULE_CREATOR "Li Kwok Shing"

char requestType;
char request;
bool responsed = true;
char responseWith;

bool attackReady = true;
int attackTimeout = 3000;
unsigned long time_now = 0;

char moduleStatus = MODULE_ATTACK_READY ;

void setup() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(actionReceived);
  Serial.begin(9600);

  Serial.print("Module Name: ");
  Serial.println(MODULE_NAME);
  Serial.print("Module Creator: ");  
  Serial.println(MODULE_CREATOR);
  delay(10);
  Serial.println("Setup Finish......");

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop() {
  if (!responsed) {
    response(responseWith);
    responsed = !responsed;
  }
  if(moduleStatus = MODULE_ATTACK_READY){
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
  }else if(moduleStatus = MODULE_ATTACK_CD){
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
  }
}

void actionReceived(int count) { //onReceive
  requestType = Wire.read();
  request = Wire.read();
  while (Wire.available()) Wire.read();

  Serial.print("Action Received Type: ");
  Serial.print(requestType);
  Serial.print(" Data: ");
  Serial.println(request);

  switch (requestType) {
    case MODULE_DATA_TYPE_REQUEST:
      Serial.println("Request recevied");
      responseWith = request;
      responsed = false;
      break;
    case MODULE_DATA_TYPE_CONTROL:
      Serial.println("Control recevied");
      getModuleControl(request);
      break;
    default:
      break;
  }
}

void response(char request) {
  switch (request) {
    case MODULE_DATA_MODULE_NAME:
      Serial.println("Name Requested");
      Wire.beginTransmission(MASTER_ADDRESS);
      Wire.write(MODULE_DATA_MODULE_NAME);
      Wire.write(MODULE_NAME);
      Wire.endTransmission();
      Serial.print("Name responsed: ");
      Serial.println(MODULE_NAME);
      break;
    case MODULE_DATA_MODULE_CREATOR:
      Serial.println("Creator Requested");
      Wire.beginTransmission(MASTER_ADDRESS);
      Wire.write(MODULE_DATA_MODULE_CREATOR);
      Wire.write(MODULE_CREATOR);
      Wire.endTransmission();
      Serial.print("Creaotr responsed: ");
      Serial.println(MODULE_CREATOR);
      break;
    case MODULE_DATA_MODULE_STATUS:
      Serial.println("Status Requested");
      Serial.print("Status responsed: ");
      Serial.println(moduleStatus);
      sendResponse(MODULE_DATA_MODULE_STATUS, moduleStatus);
      break;
    default:
      break;
  }
}

void sendResponse(char responseType, char responseData) {
  Wire.beginTransmission(MASTER_ADDRESS);
  Wire.write(responseType);
  Wire.write(responseData);
  Wire.endTransmission();
  Serial.print("Sending Response Type: ");
  Serial.print(responseType);
  Serial.print(" Data: ");
  Serial.println(responseData);
}


void getModuleControl(char control) { //
  switch (control) {
    case MODULE_ACTION_UP:
      moduleUp();
      break;
    case MODULE_ACTION_DOWN:
      moduleDown();
      break;
    case MODULE_ACTION_LEFT:
      moduleLeft();
      break;
    case MODULE_ACTION_RIGHT:
      moduleRight();
      break;
    case 'A':
      if (attackReady) {
        moduleAttack();
        attackReady = false;
        time_now = millis(); //Start timeout
      } else {
        if (millis() - time_now > attackTimeout) {
          attackReady = true;
          moduleStatus = MODULE_ATTACK_READY;
          Serial.println("Attack ready");
        } else {
          moduleStatus = MODULE_ATTACK_CD;
          Serial.println("Attack CD");
        }
      }
      break;
    default:
      break;
  }
}

void moduleUp() {
  Serial.println("Module Up");
}

void moduleDown() {
  Serial.println("Module Down");
}

void moduleLeft() {
  Serial.println("Module left");
}

void moduleRight() {
  Serial.println("Module Right");
}

void moduleAttack() {
  Serial.println("Module Attack");
}
