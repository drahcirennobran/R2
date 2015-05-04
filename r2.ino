#include <SimpleTimer.h>

const int AVANT = HIGH;
const int ARRIERE = LOW;
const int brocheVitesseMoteurA = 3;
const int brocheDirMoteurA = 12;
const int brocheFreinMoteurA = 9;
const int brocheVitesseMoteurB = 11;
const int brocheDirMoteurB = 13;
const int brocheFreinMoteurB = 8;

const int brocheThro = 4;
const int brocheAile = 5;
const int brocheINT4 = 19;
const int brocheINT5 = 18;

const float kP = 0;
const float kI = 0;
const float kD = 1;
const int kTics = 150;

int encoderR = 0;
int encoderL = 0;

SimpleTimer timer;

void setup() {
  Serial.begin(9600);
  pinMode(brocheINT4, INPUT);
  pinMode(brocheINT5, INPUT);
  pinMode(brocheThro, INPUT);
  pinMode(brocheAile, INPUT);

  pinMode(brocheVitesseMoteurA, OUTPUT);
  pinMode(brocheDirMoteurA, OUTPUT);  
  pinMode(brocheFreinMoteurA, OUTPUT);  
  pinMode(brocheVitesseMoteurB, OUTPUT);
  pinMode(brocheDirMoteurB, OUTPUT);  
  pinMode(brocheFreinMoteurB, OUTPUT);  

  digitalWrite(brocheDirMoteurA, AVANT);
  digitalWrite(brocheDirMoteurB, AVANT);
  digitalWrite(brocheFreinMoteurA, LOW);
  digitalWrite(brocheFreinMoteurB, LOW);

  attachInterrupt(4, intEncoderL, CHANGE);
  attachInterrupt(5, intEncoderR, CHANGE); 

  timer.setInterval(100, repeat2);
}

void repeat2() {

  static int coincoin = 0;
  static int throVal = 0;
  static int aileVal = 0;
  static int previousXR = 0;
  static int previousXL = 0;
  static int dxL = 0;
  static int dxR = 0;
  static unsigned long timeMillis;
  static unsigned long previousTimeMillis;
  static int puissanceMoteurs = 0;
  static int puissanceVirtuelleMoteurA = 0;
  static int puissanceVirtuelleMoteurB = 0;
  static int puissanceReelleMoteurA = 0;
  static int sensMoteurA = AVANT;
  static int puissanceReelleMoteurB = 0;
  static int sensMoteurB = AVANT;
  static int direction = 0;
  static int v;
  static int err;
  static int deltaErr = 0;
  static int errSum = 0;

  timeMillis = millis();  
  dxL = encoderL - previousXL;
  dxR = encoderR - previousXR;
  int dt = timeMillis - previousTimeMillis;
  v = (unsigned long)kTics * dxL / dt;
  Serial.print(v);
  Serial.print(':');
  //Serial.print((unsigned long)1000 * dxR / dt);
  //Serial.print(':');

  throVal = getAverage(pulseIn(brocheThro, HIGH, 25000));
  aileVal = pulseIn(brocheAile, HIGH, 25000);

  puissanceMoteurs = map(throVal, 1087, 1880, -255, 255);

  if(coincoin>30 && coincoin<60) {
    puissanceMoteurs = 20;
  } 
  else {
    puissanceMoteurs = 0;
  }

  direction = map(aileVal, 1086, 1865, -1000, 1000); 
  direction = 0;

  puissanceVirtuelleMoteurA = puissanceMoteurs - direction;
  //puissanceVirtuelleMoteurB = max(-255, min(255, puissanceMoteurs + direction));
  puissanceVirtuelleMoteurB = 0;

  sensMoteurA=computeSensMoteur(puissanceVirtuelleMoteurA);
  puissanceReelleMoteurA = abs(puissanceVirtuelleMoteurA);
  sensMoteurB=computeSensMoteur(puissanceVirtuelleMoteurB);
  puissanceReelleMoteurB = abs(puissanceVirtuelleMoteurB);

  //if(puissanceReelleMoteurA<20) puissanceReelleMoteurA=0;
  //if(puissanceReelleMoteurB<20) puissanceReelleMoteurB=0;

  //Serial.print(puissanceReelleMoteurA);
  //Serial.print(':');
  deltaErr = err;
  err = puissanceReelleMoteurA - v;
  deltaErr -= err;
  errSum += err;
  puissanceReelleMoteurA += kP * err + kI * errSum + kD*deltaErr;

  puissanceReelleMoteurA = max(0, min(255, puissanceReelleMoteurA));

  Serial.print(err + 100);
  Serial.print(':');

  Serial.print(errSum + 150);
  Serial.print(':');

  Serial.print(puissanceReelleMoteurA);
  Serial.print(':');

  digitalWrite(brocheDirMoteurA, sensMoteurA);
  digitalWrite(brocheDirMoteurB, sensMoteurB);
  analogWrite(brocheVitesseMoteurA, puissanceReelleMoteurA);
  analogWrite(brocheVitesseMoteurB, puissanceReelleMoteurB);

  previousXL = encoderL;
  previousXR = encoderR;
  previousTimeMillis = timeMillis;

  Serial.println("");
  coincoin++;
}

int getAverage(int val) {
  static int vals[10] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0      };
  static int index=0;
  static int average;
  vals[index] = val;

  index = (index+1)%10;
  average = 0;
  for(int i=0 ; i<10 ; i++) {
    average+= vals[i];
  }
  return average / 10;
}

void loop() {
  timer.run();
}

void intEncoderL() {
  encoderL++;
}

void intEncoderR() {
  encoderR++;
}

int computeSensMoteur(int p) {
  if(p > 0) {
    return AVANT;
  } 
  return ARRIERE;
}








