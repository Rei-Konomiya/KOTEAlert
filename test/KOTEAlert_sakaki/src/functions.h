#pragma once

#include <Arduino.h>

// 可読性を上げる為Cloud Functionsとの通信処理をこちらに移す

class FunctionsTransmission {
    public:
     FunctionsTransmission(void);
     String sendRequest(String method, String url, String endpoint, String data);
     String functions_get(String url, String endpoint, String getData);
     String functions_post(String url, String endpoint, String postData);
};