#include "user_sd.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

UserManagement::UserManagement(void) {}
UserManagement UM;

bool UserManagement::SDEnable() {
  return SD.begin(GPIO_NUM_4, SPI, 25000000);
}

bool UserManagement::saveUserData(uint8_t fingerUserID, String functionsUserID, String functionsUserName) {
  File myFile;
  String filename = "/" + String(fingerUserID) + ".json";
  myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {
    myFile.println("{");
    myFile.println("\t\"fingerUserID\": \"" + String(fingerUserID) + "\", ");
    myFile.println("\t\"functionsUserID\": \"" + functionsUserID + "\", ");
    myFile.println("\t\"functionsUserName\": \"" + functionsUserName + "\"");
    myFile.println("}");
    myFile.close();
    return true;
  } else {
    return false;
  }
}

bool UserManagement::existUserData(uint8_t uid) {
  File myFile;
  myFile = SD.open("/" + String(uid) + ".json");
  if (myFile) {
    while (myFile.available()) {
      myFile.read();
    }
    myFile.close();
    return true;
  } else {
    return false;
  }
}

String UserManagement::getUserData(uint8_t uid) {
  File myFile;
  myFile = SD.open("/" + String(uid) + ".json");
  if (myFile) {
    String fileContent = "";
    while (myFile.available()) {
      fileContent += (char)myFile.read();
    }
    myFile.close();
    return fileContent;
  } else {
    return "no open";
  }
}

// 今後実装予定
void UserManagement::deleteUserData(uint8_t uid) {

}