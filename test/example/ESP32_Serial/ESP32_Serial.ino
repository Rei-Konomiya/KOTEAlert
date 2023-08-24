#define RX_PIN 4
#define TX_PIN 14

void setup() {
  Serial.begin(19200);
  //Serial2.begin(19200);
  Serial2.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
}

void loop() {
  if(Serial2.available()){
    Serial.println("get");
    char c = Serial2.read();    // データを読み取る
    Serial.print(c);
  }
  for (int i = 0 ; i < 100 ; i++){
    Serial.print(i);
    Serial2.print(i);
    Serial2.write(i);
    delay(1000);
  }
  Serial.println();
}
