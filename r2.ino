#include <PID_v1.h>
#include <Arduino.h>

#define MOTOR_LEFT 0
#define MOTOR_RIGHT 1

const int AVANT = HIGH;
const int ARRIERE = LOW;
const int motorConf [3][3] = {{7, 5, 6}, {2, 4, 3}};

const int brocheThro = 8;
const int brocheAile = 9;
const int brocheINT4 = 18;
const int brocheINT5 = 19;

const int kTics = 100;

volatile int encoderR = 0;
volatile int encoderL = 0;

double leftSpeedTarget, leftInput, leftOutput;
double rightSpeedTarget, rightInput, rightOutput;

PID pidLeft(&leftInput, &leftOutput, &leftSpeedTarget, 2, 5, 0, DIRECT);
PID pidRight(&rightInput, &rightOutput, &rightSpeedTarget, 2, 5, 0, DIRECT);

void setup()  {
  Serial.begin(115000);

  pinMode(brocheINT4, INPUT);
  pinMode(brocheINT5, INPUT);
  pinMode(brocheThro, INPUT);
  pinMode(brocheAile, INPUT);

  pinMode(motorConf[0][0], OUTPUT);
  pinMode(motorConf[0][1], OUTPUT);
  pinMode(motorConf[0][2], OUTPUT);
  pinMode(motorConf[1][0], OUTPUT);
  pinMode(motorConf[1][1], OUTPUT);
  pinMode(motorConf[1][2], OUTPUT);

  attachInterrupt(4, intEncoderL, CHANGE); //PIN 19
  attachInterrupt(5, intEncoderR, CHANGE); //PIN 18

  leftInput = 0;
  rightInput = 0;
  leftSpeedTarget = 0;
  rightSpeedTarget = 0;
  pidLeft.SetMode(AUTOMATIC);
  pidLeft.SetSampleTime(100);
  pidRight.SetMode(AUTOMATIC);
  pidRight.SetSampleTime(100);

}

void repeat(void) {

  static int iScenar = 0;
  static int throVal = 0;
  static int aileVal = 0;
  static int puissanceMoteurs = 0;
  static int sensMoteurL = AVANT;
  static int sensMoteurR = AVANT;
  static int direction = 0;

  static unsigned long timeMillis;
  static unsigned long previousTimeMillis;

  static int dxR = 0;
  static int dxL = 0;
  static int previousXR = 0;
  static int previousXL = 0;
  static int vL;
  static int vR;
  static int dt;
  
  //throVal = getAverage(pulseIn(brocheThro, HIGH, 25000));
  throVal = pulseIn(brocheThro, HIGH, 25000);
  aileVal = pulseIn(brocheAile, HIGH, 25000);
  /*
    puissanceMoteurs = map(throVal, 1087, 1880, -50, 50);
    direction = map(aileVal, 1110, 1865, -20, 20);
  */
  timeMillis = millis();
  dt = timeMillis - previousTimeMillis;
  dxL = encoderL - previousXL;
  dxR = encoderR - previousXR;

  previousXL = encoderL;
  previousXR = encoderR;

  previousTimeMillis = timeMillis;
  vL = (unsigned long) kTics * dxL / dt;
  vR = (unsigned long) kTics * dxR / dt;

  leftInput = vL;
  rightInput = vR;
  //leftSpeedTarget = map(throVal, 1087, 1880, -50, 50);
  leftSpeedTarget = 40; //TODO : gérer le sens de rotation
  rightSpeedTarget = 40; //TODO : gérer le sens de rotation
  pidLeft.Compute();
  pidRight.Compute();
/*
  traceVal(leftSpeedTarget);
  traceVal(leftInput);
  traceVal(leftOutput);
  //traceVal(vL);
  Serial.println("");
  */
  if (iScenar > 20) {
    leftOutput = 0;
  }
  setMotorPower(MOTOR_LEFT, sensMoteurL, leftOutput, false);
  setMotorPower(MOTOR_RIGHT, sensMoteurR, leftOutput, false);

  iScenar++;
}

void setMotorPower(int motor, int direction, int power, boolean brake) {

  if (brake) {
    analogWrite(motorConf[motor][0], 0);
    digitalWrite(motorConf[motor][1],  LOW);
    digitalWrite(motorConf[motor][2], LOW);
  } else {
    if (direction == AVANT) {
      digitalWrite(motorConf[motor][1],  LOW);
      digitalWrite(motorConf[motor][2], HIGH);
    } else {
      digitalWrite(motorConf[motor][1],  HIGH);
      digitalWrite(motorConf[motor][2], LOW);
    }

    analogWrite(motorConf[motor][0], power);
  }
}

void traceVal(float val) {
  Serial.print(val);
  Serial.print('|');
}

void traceVal(double val) {
  Serial.print(val);
  Serial.print('|');
}

void traceVal(int val) {
  Serial.print(val);
  Serial.print('|');
}

void traceVal(unsigned long val) {
  Serial.print(val);
  Serial.print('|');
}

int getAverage(int val) {
  static int vals[10] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  static int index = 0;
  static int average;
  vals[index] = val;

  index = (index + 1) % 10;
  average = 0;
  for (int i = 0; i < 10; i++) {
    average += vals[i];
  }
  return average / 10;
}

void loop() {
  repeat();
}

void intEncoderL() {
  encoderL++;
}

void intEncoderR() {
  encoderR++;
}

int computeSensMoteur(int p) {
  if (p > 0) {
    return AVANT;
  }
  return ARRIERE;
}




