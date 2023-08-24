#pragma once

#include <Arduino.h>

class UserManagement {
  public:
   UserManagement(void);
   bool saveUserData(uint8_t fingerUserID, String functionsUserID, String functionsUserName);
   bool existUserData(uint8_t uid);
   String getUserData(uint8_t uid);
   void deleteUserData(uint8_t uid);
};