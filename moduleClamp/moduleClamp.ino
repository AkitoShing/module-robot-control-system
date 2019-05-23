#include <SoftwareSerial.h>
#include <Wire.h>
#include <Data.h>
#include <timer.h>
#include <Servo.h>

#define __DEBUG__

/* ---------- Custom Code Here ---------- */

#define SERVO_SIGNAL_PIN 8
#define SERVO_CLOSE_ANGLE 100
#define SERVO_OPEN_ANGLE 15
#define SERVO_CLOSE_DELAY 500

Timer stopMotor;

Servo servo;

/* ---------- Custom Code End ---------- */

/* -------------------------------------------------- */
/* You can change the module infomation here          */
/* The information will be sent back to the robot     */
/* and show in the application                        */
#define MODULE_NAME "Clamp Clamper"
#define MODULE_CREATOR "MRCS Team"
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
Timer weaponRestore;

void setup() {
    customPinSetup();
    setServoPinMode();

    cd.setTimeout(ATTACK_TIMEOUT);
    cd.setCallback(cdEnd); // TODO: do the cdEnd

    weaponRestore.setTimeout(MODULE_RESET_DELAY);
    weaponRestore.setCallback(restore); // TODO: do the restore

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

    servo.attach(SERVO_SIGNAL_PIN);
    servo.write(SERVO_CLOSE_ANGLE);

    Serial.println("Setup Finish...");
    digitalWrite(MODULE_ENABLE_PIN, HIGH);
}

void loop() {
    stopMotor.update();
    cd.update();
    weaponRestore.update();

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
      digitalWrite(3,HIGH);
    }
}

void setServoPinMode() {
    pinMode(SERVO_SIGNAL_PIN, OUTPUT);
    pinMode(MODULE_ENABLE_PIN, OUTPUT);
}

void customPinSetup() {
    pinMode(2,OUTPUT);
    pinMode(3,OUTPUT);

    stopMotor.setTimeout(300);
    stopMotor.setCallback(stopServo);
}

void requestReceived(int count) {
    requestType = Wire.read();
    request = Wire.read();
    while (Wire.available()) Wire.read();

    #ifdef __DEBUG__
    Serial.print("Action Received Type: ");
    Serial.print(requestType);
    Serial.print(" Data: ");
    Serial.println(request);
    #endif

    switch (requestType) {
        case DATA_TYPE_REQUEST:
            Serial.println("Request received");
            responseWith = request;
            responsed = false;
            break;
        case MODULE_DATA_MODULE_ACTION:
            Serial.println("Control received");
            getModuleControl(request);
            break;
        default:
            break;
    }
}

void response(char request) {
    switch(request) {
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
            Serial.print("Creator responsed: ");
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

void getModuleControl(char Control) {
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
                weaponRestore.start();
                cd.start()
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
    attackReady = ture;
    Serial.println("Attack Ready");
    sendModuleStatus(MODULE_DATA_MODULE_STATUS_ATTACK_READY);
}

void restore() {
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
    // TODO: write the code from austin
    servo.write(SERVO_CLOSE_ANGLE);
    Serial.println("Close Servo");
    stopMotor.start();
}

void moduleRestore() {
    servo.write(SERVO_OPEN_ANGLE);
    Serial.println("Open Servo");
    stopMotor.start();
}

void stopServo() {
    Serial.println("stop motor");
    servo.write(SERVO_CLOSE_ANGLE);
}