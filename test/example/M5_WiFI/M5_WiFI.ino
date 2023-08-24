#include "config.h"
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>


void setup() {
  /** M5セットアップ **/
  M5.begin();
  M5.Lcd.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  
  M5.Lcd.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    M5.Lcd.print(".");
  }

  M5.Lcd.println("\nConnected!");
  M5.Lcd.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());


  // HTTPクライアントを使ってWebサイトにアクセス
  HTTPClient http;
  http.begin("http://example.com");
  int httpCode = http.GET(); // リクエストを開始

  if (httpCode > 0) {
    String payload = http.getString();  // レスポンスを取得
    M5.Lcd.println("Response:");
    M5.Lcd.println(payload);            // レスポンスを表示
  } else {
    M5.Lcd.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();  // HTTP接続をクローズ
}

void loop() {
  // ここにメインループ内のコードを記述
}
