#include "config.h"
#include "functions.h"
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

FunctionsTransmission FT_S;

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  M5.Lcd.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    M5.Lcd.print(".");
  }
  M5.Lcd.println("\nConnected!");
  delay(500);

  // String postData = "{\"user_id\": \"1111\"}";
  // String result = FT_S.functions_post(String(functionsUrl), String(testEndpoint), postData);

  // M5.Lcd.println(result);
  // Serial.println(result);


  String postData = "{\"user_id\": \"" + user_id + "\", \"device_id\": \"" + device_id + "\"}";
  String result = FT_S.functions_post(String(functionsUrl), String(startEndpoint), postData);

  M5.Lcd.println(result);
  Serial.println(result);

  String log_id = result.substring(1, result.length() - 1);

  postData = "{\"user_id\": \"" + user_id + "\", \"device_id\": \"" + device_id + "\", " + log_id + "}";
  result = FT_S.functions_post(String(functionsUrl), String(alertEndpoint), postData);

  M5.Lcd.println(result);
  Serial.println(result);

  //String log_id = result;
  postData = "{" + log_id + "}";
  result = FT_S.functions_post(String(functionsUrl), String(endEndpoint), postData);

  M5.Lcd.println(result);
  Serial.println(result);
}

void loop() {
  // put your main code here, to run repeatedly:

}
