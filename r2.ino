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

const int kTics = 56;

volatile int encoderR = 0;
volatile int encoderL = 0;

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
}

void repeat(void) {
  static float kP = 0.5;//0.6
  static float kI = 0.2;
  static float kD = 0;
  static unsigned long timeMillis;
  static unsigned long previousTimeMillis;

  static int iScenar = 0;
  static int throVal = 0;
  static int aileVal = 0;
  static int previousXR = 0;
  static int previousXL = 0;
  static int dxR = 0;
  static int dxL = 0;
  static int puissanceMoteurs = 0;
  static int puissanceVirtuelleMoteurL = 0;
  static int puissanceVirtuelleMoteurR = 0;
  static int puissanceReelleMoteurL = 0;
  static int puissanceReelleCompenseeMoteurL = 0;
  static int sensMoteurL = AVANT;
  static int puissanceReelleMoteurR = 0;
  static int puissanceReelleCompenseeMoteurR = 0;
  static int sensMoteurR = AVANT;
  static int direction = 0;
  static int vL;
  static int vR;
  static int errL;
  static int errR;
  static int deltaErrL = 0;
  static int deltaErrR = 0;
  static int errSumL = 0;
  static int errSumR = 0;

  timeMillis = millis();
  int dt = timeMillis - previousTimeMillis;

  //traceVal(encoderL);
  //traceVal(previousXL);
  dxL = encoderL - previousXL;
  previousXL = encoderL;
  //traceVal(dxL);

  vL = (unsigned long) kTics * dxL / dt;
  //traceVal(vL);

  dxR = encoderR - previousXR;
  previousXR = encoderR;
  vR = (unsigned long) kTics * dxR / dt;

  //throVal = getAverage(pulseIn(brocheThro, HIGH, 25000));
  throVal = pulseIn(brocheThro, HIGH, 25000);
  aileVal = pulseIn(brocheAile, HIGH, 25000);

  puissanceMoteurs = map(throVal, 1087, 1880, -50, 50);
  direction = map(aileVal, 1110, 1865, -20, 20);
  //traceVal(throVal);
  //traceVal(direction);


  previousTimeMillis = timeMillis;

  //puissanceMoteurs = 15;
  //traceVal(puissanceMoteurs);

  //direction = 0;

  puissanceVirtuelleMoteurL = puissanceMoteurs - direction;
  puissanceVirtuelleMoteurR = puissanceMoteurs + direction;
/*
if (abs(puissanceVirtuelleMoteurL) < 10) {
       puissanceVirtuelleMoteurL = 0;
       vL = 0;
    }
    if (abs(puissanceVirtuelleMoteurR) < 10) {
       puissanceVirtuelleMoteurR = 0;
       vR = 0;
    }
*/
  sensMoteurL = computeSensMoteur(puissanceVirtuelleMoteurL);
  puissanceReelleMoteurL = abs(puissanceVirtuelleMoteurL);
  sensMoteurR = computeSensMoteur(puissanceVirtuelleMoteurR);
  puissanceReelleMoteurR = abs(puissanceVirtuelleMoteurR);

  deltaErrL = errL;
  errL = puissanceReelleMoteurL - vL;
  deltaErrL -= errL;
  errSumL += errL;
  //traceVal(vL);
  //traceVal(errL);

  // le truc que je crois que ça marchait
  //puissanceReelleCompenseeMoteurL += kP * errL ;// + kI * errSumL + kD * deltaErrL;
  // régul PID
  //puissanceReelleCompenseeMoteurL = kP * errL ;// + kI * errSumL + kD * deltaErrL;
  // Sauce Ric

  puissanceReelleCompenseeMoteurL = puissanceReelleMoteurL + kP * errL + kI * errSumL + kD * deltaErrL;
  puissanceReelleCompenseeMoteurL = max(0, min(255, puissanceReelleCompenseeMoteurL));

  deltaErrR = errR;
  errR = puissanceReelleMoteurR - vR;
  deltaErrR -= errR;
  errSumR += errR;
  puissanceReelleCompenseeMoteurR = puissanceReelleMoteurR + kP * errR + kI * errSumR + kD * deltaErrR;
  puissanceReelleCompenseeMoteurR = max(0, min(255, puissanceReelleCompenseeMoteurR));
  /*
    if (iScenar > 200) {
      puissanceReelleCompenseeMoteurL = 0;
      puissanceReelleCompenseeMoteurR = 0;
    }
    */
  //traceVal(puissanceReelleCompenseeMoteurL);

  setMotorPower(MOTOR_LEFT, sensMoteurL, puissanceReelleCompenseeMoteurL, false);
  setMotorPower(MOTOR_RIGHT, sensMoteurR, puissanceReelleCompenseeMoteurR, false);

  /*
    trace(iScenar,timeMillis,kP,kI,kD,puissanceVirtuelleMoteurL,puissanceVirtuelleMoteurR,puissanceReelleMoteurL,puissanceReelleMoteurR,puissanceReelleCompenseeMoteurL,sensMoteurL,
  		  puissanceReelleCompenseeMoteurR,sensMoteurR,vL,vR,errL,errR,errSumL,errSumR,deltaErrL,deltaErrR,
  		  encoderL,encoderR);
  */
  //Serial.println("");
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
void trace(int v1, unsigned long t, float v2, float v3, float v4, int v5, int v6, int v7, int v8, int v9, int v10, int v11, int v12, int v13, int v14, int v15, int v16, int v17, int v18, int v19, int v20, int
           v21, int v22) {
  traceVal(v1);
  traceVal(t);
  traceVal(v2);
  traceVal(v3);
  traceVal(v4);
  traceVal(v5);
  traceVal(v6);
  traceVal(v7);
  traceVal(v8);
  traceVal(v9);
  traceVal(v10);
  traceVal(v11);
  traceVal(v12);
  traceVal(v13);
  traceVal(v14);
  traceVal(v15);
  traceVal(v16);
  traceVal(v17);
  traceVal(v18);
  traceVal(v19);
  traceVal(v20);
  traceVal(v21);
  traceVal(v22);
  Serial.println("");
}

void traceVal(float val) {
  Serial.print(val);
  Serial.print(':');
}

void traceVal(int val) {
  Serial.print(val);
  Serial.print(':');
}

void traceVal(unsigned long val) {
  Serial.print(val);
  Serial.print(':');
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




