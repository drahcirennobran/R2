#include <SimpleTimer.h>
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
const int brocheINT4 = 19;
const int brocheINT5 = 18;

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

	attachInterrupt(4, intEncoderL, CHANGE);
	attachInterrupt(5, intEncoderR, CHANGE);

	timer.setInterval(100, repeat2);
}

void repeat2() {

	static float kP = 0;
	static float kI = 0;
	static float kD = 0;

	static int iScenar = 0;
	static int throVal = 0;
	static int aileVal = 0;
	static int previousXR = 0;
	static int previousXL = 0;
	static int dxR = 0;
	static int dxL = 0;
	static unsigned long timeMillis;
	static unsigned long previousTimeMillis;
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

	int scenar1;

	timeMillis = millis();
	int dt = timeMillis - previousTimeMillis;

	dxL = encoderL - previousXL;
	vL = (unsigned long) kTics * dxL / dt;

	dxR = encoderR - previousXR;
	vR = (unsigned long) kTics * dxR / dt;


	throVal = getAverage(pulseIn(brocheThro, HIGH, 25000));
	aileVal = pulseIn(brocheAile, HIGH, 25000);

	puissanceMoteurs = map(throVal, 1087, 1880, -255, 255);

	scenar1 = (iScenar%20)-10;
	scenar1 = scenar1==0?1:scenar1;
	puissanceMoteurs = 20 * scenar1/abs(scenar1);
	kP = (iScenar/20) - (iScenar/20)%20;

	direction = map(aileVal, 1086, 1865, -1000, 1000);
	direction = 0;

	puissanceVirtuelleMoteurL = puissanceMoteurs - direction;
	puissanceVirtuelleMoteurR = puissanceMoteurs + direction;
	puissanceVirtuelleMoteurR = 0;

	sensMoteurL = computeSensMoteur(puissanceVirtuelleMoteurL);
	puissanceReelleMoteurL = abs(puissanceVirtuelleMoteurL);
	sensMoteurR = computeSensMoteur(puissanceVirtuelleMoteurR);
	puissanceReelleMoteurR = abs(puissanceVirtuelleMoteurR);

	//if(puissanceReelleMoteurL<20) puissanceReelleMoteurL=0;
	//if(puissanceReelleMoteurB<20) puissanceReelleMoteurB=0;

	//Serial.print(puissanceReelleMoteurL);
	//Serial.print(':');
	deltaErrL = errL;
	errL = puissanceReelleMoteurL - vL;
	deltaErrL -= errL;
	errSumL += errL;
	puissanceReelleMoteurL += kP * errL + kI * errSumL + kD * deltaErrL;
	puissanceReelleMoteurL = max(0, min(255, puissanceReelleMoteurL));

	deltaErrR = errR;
	errR = puissanceReelleMoteurR - vR;
	deltaErrR -= errR;
	errSumR += errR;
	puissanceReelleCompenseeMoteurR += kP * errR + kI * errSumR + kD * deltaErrR;
	puissanceReelleCompenseeMoteurR = max(0, min(255, puissanceReelleMoteurR));

	//Serial.print(errL + 100);
	//Serial.print(':');

	//Serial.print(errSumL + 150);
//  Serial.print(':');

//  Serial.print(puissanceReelleMoteurL);
//  Serial.print(':');

	digitalWrite(brocheDirMoteurL, sensMoteurL);
	digitalWrite(brocheDirMoteurR, sensMoteurR);
	analogWrite(brocheVitesseMoteurL, puissanceReelleCompenseeMoteurL);
	analogWrite(brocheVitesseMoteurR, puissanceReelleCompenseeMoteurR);

	previousXL = encoderL;
	previousXR = encoderR;
	previousTimeMillis = timeMillis;

	Serial.print(iScenar);
	Serial.print(':');
	Serial.print(kP);
	Serial.print(':');
	Serial.print(kI);
	Serial.print(':');
	Serial.print(kD);
	Serial.print(':');
	Serial.print(puissanceVirtuelleMoteurL);
	Serial.print(':');
	Serial.print(puissanceVirtuelleMoteurR);
	Serial.print(':');
	Serial.print(puissanceReelleMoteurL);
	Serial.print(':');
	Serial.print(puissanceReelleMoteurR);
	Serial.print(':');
	Serial.print(puissanceReelleCompenseeMoteurL);
	Serial.print(':');
	Serial.print(sensMoteurL);
	Serial.print(':');
	Serial.print(puissanceReelleCompenseeMoteurR);
	Serial.print(':');
	Serial.print(sensMoteurR);
	Serial.print(':');
	Serial.print(vL);
	Serial.print(':');
	Serial.print(vR);
	Serial.print(':');
	Serial.print(errL);
	Serial.print(':');
	Serial.print(errR);
	Serial.print(':');
	Serial.print(errSumL);
	Serial.print(':');
	Serial.print(errSumR);
	Serial.print(':');
	Serial.print(deltaErrL);
	Serial.print(':');
	Serial.print(deltaErrR);

	Serial.println("");

	iScenar++;
}

int getAverage(int val) {
	static int vals[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
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
	timer.run();
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

