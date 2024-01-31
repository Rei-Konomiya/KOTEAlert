#include "soldering_sensor.h"
#include <Arduino.h>
#include <stdint.h>

#define THERMISTOR_B 3431
#define THERMISTOR_R0 10000
#define THERMISTOR_T0 25
#define RESISTOR_PULL_DOWN 10000
#define ANALOG_MAX 4095

SolderingSensor::SolderingSensor(void) {}
SolderingSensor SS_;

void SolderingSensor::attach(int TRIG, int ECHO, int RIRER, int THERMISTOR) {
  this->trig = TRIG;
  this->echo = ECHO;
  this->rirer = RIRER;
  this->thermistor = THERMISTOR;
  pinMode(this->echo, INPUT);
  pinMode(this->trig, OUTPUT);
}

double SolderingSensor::readDistance() {
  double speed_of_sound = 331.5 + 0.6 * 25; // 25℃の気温の想定
  double distance = 0;
  double duration;

  digitalWrite(this->trig, LOW); 
  delayMicroseconds(2); 
  digitalWrite(this->trig, HIGH);
  delayMicroseconds(10); 
  digitalWrite(this->trig, LOW);
  duration = pulseIn(this->echo, HIGH); // 往復にかかった時間が返却される[マイクロ秒]

  if (duration > 0) {
    duration = duration / 2; // 往路にかかった時間
    distance = duration * speed_of_sound * 100 / 1000000;
  }
  
  return distance;
}

float SolderingSensor::readTemperature() {
  uint16_t valAnalog = analogRead(this->thermistor);
  float resistor = calcResistorByAnalogValue(valAnalog, ANALOG_MAX, RESISTOR_PULL_DOWN);
  float temperature = calcTempratureByResistor(resistor, THERMISTOR_B, THERMISTOR_R0, THERMISTOR_T0);
  return temperature;
}

void SolderingSensor::rirerRun(bool flg){
  if (flg) {
    digitalWrite(this->trig, HIGH);
  } else {
    digitalWrite(this->trig, LOW); 
  }
}

float SolderingSensor::calcTempratureByResistor(float R, int B, int R0, float T0) {
  return 1 / (log(R / R0) / B + (1.0 / (T0 + 273))) - 273;
}

float SolderingSensor::calcResistorByAnalogValue(uint16_t analog, uint16_t analogMax, float resistorPullDown) {
  return (double)(analogMax - analog) / analog * resistorPullDown;
}