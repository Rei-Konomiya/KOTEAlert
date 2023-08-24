#include <Arduino.h>

#define TRIG 4
#define ECHO 15
#define PIN_THERMISTOR 13  // analog read pin

#define THERMISTOR_B 3431
#define THERMISTOR_R0 10000
#define THERMISTOR_T0 25

#define RESISTOR_PULL_DOWN 10000

#ifdef ESP32
#define ANALOG_MAX 4095
#else
#define ANALOG_MAX 1023
#endif

double duration = 0;
double distance = 0;
double speed_of_sound = 331.5 + 0.6 * 25; // 25℃の気温の想定

float calcTempratureByResistor(float R, int B, int R0, float T0) {
  return 1 / (log(R / R0) / B + (1.0 / (T0 + 273))) - 273;
}

float calcResistorByAnalogValue(uint16_t analog, uint16_t analogMax,
                                float resistorPullDown) {
  return (double)(analogMax - analog) / analog * resistorPullDown;
}

void setup() {
  Serial.begin(115200);

  pinMode(ECHO, INPUT);
  pinMode(TRIG, OUTPUT);
}

void loop() {
  digitalWrite(TRIG, LOW); 
  delayMicroseconds(2); 
  digitalWrite( TRIG, HIGH );
  delayMicroseconds( 10 ); 
  digitalWrite( TRIG, LOW );
  duration = pulseIn( ECHO, HIGH ); // 往復にかかった時間が返却される[マイクロ秒]

  if (duration > 0) {
    duration = duration / 2; // 往路にかかった時間
    distance = duration * speed_of_sound * 100 / 1000000;
    Serial.print("Distance:");
    Serial.print(distance);
    Serial.println(" cm");
  }

  uint16_t valAnalog = analogRead(PIN_THERMISTOR);
  float resistor =
      calcResistorByAnalogValue(valAnalog, ANALOG_MAX, RESISTOR_PULL_DOWN);
  float temperature = calcTempratureByResistor(resistor, THERMISTOR_B,
                                               THERMISTOR_R0, THERMISTOR_T0);
  Serial.println("Temperature: " + String(temperature) + "C");
  Serial.println("at " + String(millis()));
  delay(200);
}