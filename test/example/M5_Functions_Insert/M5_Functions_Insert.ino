#include "config.h"
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>

WiFiClientSecure client;
HTTPClient https;

void setup() {
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

  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.println("\nConnected!");
  M5.Lcd.setTextColor(WHITE);

  client.setInsecure();

  if (!client.connect(FireFunctionUrl, 443)) {
    M5.Lcd.println("Connection failed");
    return;
  }

  // データをJSON形式で送信
  String postData = "{\"user_id\": \"1111\"}";

  client.println("POST " + String(testEndpoint) + " HTTP/1.1");
  client.println("Host: " + String(FireFunctionUrl));
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(postData.length());
  client.println();
  client.print(postData);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  String payload = client.readString();
  M5.Lcd.println(payload);
}


void loop() {
  // M5.Lcd.println("Connecting to Firebase...");

  // client.setInsecure();

  // if (!client.connect(FireFunctionUrl, 443)) {
  //   M5.Lcd.println("Connection failed");
  //   return;
  // }

  // String url = String(getFirestoreDocumentEndPoint) + "?path=test/User";  // Replace with your Firestore document path

  // client.println("GET " + url + " HTTP/1.1");
  // client.println("Host: " + String(FireFunctionUrl));
  // client.println("Connection: close");
  // client.println();

  // while (client.connected()) {
  //   String line = client.readStringUntil('\n');
  //   if (line == "\r") {
  //     break;
  //   }
  // }

  // String payload = client.readString();
  // M5.Lcd.println(payload);

  delay(100000);
}

