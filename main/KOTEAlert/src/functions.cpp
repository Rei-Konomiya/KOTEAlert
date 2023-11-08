#include "functions.h"
#include <Arduino.h>
#include <stdint.h>
#include <WiFi.h>
#include <HTTPClient.h>

WiFiClientSecure client;
HTTPClient https;

FunctionsTransmission::FunctionsTransmission(void) {}
FunctionsTransmission FT;

String FunctionsTransmission::sendRequest(String method, String url, String endpoint, String data) {
    client.setInsecure();

    if (!client.connect(url.c_str(), 443)) {
        return "Connection failed";
    }

    client.println(method + " " + endpoint + " HTTP/1.1");
    client.println("Host: " + url);
    client.println("Connection: close");
    if(method == "POST") {
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.print(data);
    } else {
        client.println();
    }

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
    }

    return client.readString();
}

String FunctionsTransmission::functions_get(String url, String endpoint, String getData) {
  return sendRequest("GET", url, endpoint, getData);
}

String FunctionsTransmission::functions_post(String url, String endpoint, String postData) {
  return sendRequest("POST", url, endpoint, postData);
}