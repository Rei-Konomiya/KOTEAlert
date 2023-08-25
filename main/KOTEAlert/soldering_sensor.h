#pragma once

#include <stdint.h>
#include <Arduino.h>

class SolderingSensor {
  public:
   SolderingSensor(void);
   void attach(int TRIG, int ECHO, int THERMISTOR);
   double readDistance();
   float readTemperature();
  private:
   int trig;
   int echo;
   int thermistor;
   float calcTempratureByResistor(float R, int B, int R0, float T0);
   float calcResistorByAnalogValue(uint16_t analog, uint16_t analogMax, float resistorPullDown);
};