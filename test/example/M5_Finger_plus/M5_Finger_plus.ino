#include "finger.h"
#include <M5Stack.h>

uint8_t userNum;
FingerPrint FP_M;

void CleanScreen() {
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.fillRect(0, 20, 350, 300, BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextFont(2);
    userNum = FP_M.fpm_getUserNum();
    M5.Lcd.print("userNum:");
    M5.Lcd.println(userNum);
}

void setup() {
  M5.begin();
  // Serial.begin(115200);
  Serial2.begin(19200, SERIAL_8N1, 3, 1);

  delay(1000);

  if (FP_M.fpm_deleteAllUser() == ACK_SUCCESS) {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Init success.");
  }else{
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("Init fail.");
  }

  FP_M.fpm_setAddMode(0x00);
  
  CleanScreen();

  M5.Lcd.println(FP_M.fpm_readAddMode());

  // Serial.println("test");
}

void loop() {
    uint8_t res1;
    if (M5.BtnA.wasPressed()) {
        CleanScreen();
        M5.Lcd.println("Fingerprint Typing");

        uint8_t uid = userNum+1;

        res1 = FP_M.fpm_addUser(uid, 1);
        if (res1 == ACK_SUCCESS) {
            M5.Lcd.println("Success");
            M5.Lcd.println(String(uid));
        } else if (res1 == ACK_FAIL) {
            M5.Lcd.println("Fail");
        } else if (res1 == ACK_FULL) {
            M5.Lcd.println("Full");
        } else {
            M5.Lcd.setTextColor(RED);
            M5.Lcd.println("Timeout");
        }
        userNum++;
    }

    if (M5.BtnB.wasPressed()) {
        CleanScreen();
        M5.Lcd.println("Matching");

        res1 = FP_M.fpm_compareFinger();
        if (res1 == ACK_SUCCESS) {
            M5.Lcd.println("Success");
            M5.Lcd.println(String(FP_M.getUID()));
        }
        if (res1 == ACK_NOUSER) {
            M5.Lcd.println("No Such User");
        }
        if (res1 == ACK_TIMEOUT) {
            M5.Lcd.println("Timeout");
        }
    }

    if (M5.BtnC.wasPressed()) {
      CleanScreen();
      M5.Lcd.println(FP_M.fpm_showAllUser());
    }

    M5.update();
}
