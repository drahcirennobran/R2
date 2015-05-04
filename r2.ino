#include <Arduino.h>

const int AVANT = HIGH;
const int ARRIERE = LOW;
const int brocheVitesseMoteurL = 3;
const int brocheDirMoteurL = 12;
const int brocheFreinMoteurL = 9;
const int brocheVitesseMoteurR = 11;
const int brocheDirMoteurR = 13;
const int brocheFreinMoteurR = 8;

const int brocheThro = 4;
const int brocheAile = 5;
const int brocheINT4 = 18;
const int brocheINT5 = 19;

const int kTics = 150;

volatile int encoderR = 0;
volatile int encoderL = 0;

void setup()  {
  Serial.begin(9600);
  pinMode(brocheINT4, INPUT);
  pinMode(brocheINT5, INPUT);
  pinMode(brocheThro, INPUT);
  pinMode(brocheAile, INPUT);

  pinMode(brocheVitesseMoteurL, OUTPUT);
  pinMode(brocheDirMoteurL, OUTPUT);
  pinMode(brocheFreinMoteurL, OUTPUT);
  pinMode(brocheVitesseMoteurR, OUTPUT);
  pinMode(brocheDirMoteurR, OUTPUT);
  pinMode(brocheFreinMoteurR, OUTPUT);

  digitalWrite(brocheDirMoteurL, AVANT);
  digitalWrite(brocheDirMoteurR, AVANT);
  digitalWrite(brocheFreinMoteurL, LOW);
  digitalWrite(brocheFreinMoteurR, LOW);

  attachInterrupt(4, intEncoderL, CHANGE); //PIN 19
  attachInterrupt(5, intEncoderR, CHANGE); //PIN 18
}

void repeat(void) {
  static float kP = 1;
  static float kI = 0;
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

  dxL = encoderL - previousXL;
  vL = (unsigned long) kTics * dxL / dt;

  dxR = encoderR - previousXR;
  vR = (unsigned long) kTics * dxR / dt;

  //throVal = getAverage(pulseIn(brocheThro, HIGH, 25000));
  throVal = pulseIn(brocheThro, HIGH, 25000);
  aileVal = pulseIn(brocheAile, HIGH, 25000);

  puissanceMoteurs = map(throVal, 1087, 1880, -10, 10);
  direction = map(aileVal, 1110, 1865, -2, 2);
  //traceVal(throVal);
  //traceVal(direction);

  puissanceMoteurs = 4;
  traceVal(puissanceMoteurs);

  direction = 0;

  puissanceVirtuelleMoteurL = puissanceMoteurs - direction;
  puissanceVirtuelleMoteurR = puissanceMoteurs + direction;

  sensMoteurL = computeSensMoteur(puissanceVirtuelleMoteurL);
  puissanceReelleMoteurL = abs(puissanceVirtuelleMoteurL);
  sensMoteurR = computeSensMoteur(puissanceVirtuelleMoteurR);
  puissanceReelleMoteurR = abs(puissanceVirtuelleMoteurR);

  deltaErrL = errL;
  errL = puissanceReelleMoteurL - vL;
  deltaErrL -= errL;
  errSumL += errL;
  traceVal(vL);
    traceVal(errL);
  puissanceReelleCompenseeMoteurL += kP * errL ;// + kI * errSumL + kD * deltaErrL;
  puissanceReelleCompenseeMoteurL = max(0, min(255, puissanceReelleCompenseeMoteurL));

  deltaErrR = errR;
  errR = puissanceReelleMoteurR - vR;
  deltaErrR -= errR;
  errSumR += errR;
  puissanceReelleCompenseeMoteurR += kP * errR ;// + kI * errSumR + kD * deltaErrR;
  puissanceReelleCompenseeMoteurR = max(0, min(255, puissanceReelleCompenseeMoteurR));
/*
  if (iScenar > 20 || iScenar > 50) {
    puissanceReelleCompenseeMoteurL = 0;
    puissanceReelleCompenseeMoteurR = 0;
  }
*/
  digitalWrite(brocheDirMoteurL, sensMoteurL);
  digitalWrite(brocheDirMoteurR, sensMoteurR);
  traceVal(puissanceReelleCompenseeMoteurL);
  analogWrite(brocheVitesseMoteurL, puissanceReelleCompenseeMoteurL);
  analogWrite(brocheVitesseMoteurR, puissanceReelleCompenseeMoteurR);

  previousXL = encoderL;
  previousXR = encoderR;
  previousTimeMillis = timeMillis;
  /*
    trace(iScenar,timeMillis,kP,kI,kD,puissanceVirtuelleMoteurL,puissanceVirtuelleMoteurR,puissanceReelleMoteurL,puissanceReelleMoteurR,puissanceReelleCompenseeMoteurL,sensMoteurL,
  		  puissanceReelleCompenseeMoteurR,sensMoteurR,vL,vR,errL,errR,errSumL,errSumR,deltaErrL,deltaErrR,
  		  encoderL,encoderR);
  */
  Serial.println("");
  iScenar++;
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




