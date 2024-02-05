#include "finger.h"

#include <Arduino.h>

FingerPrint::FingerPrint(void) {}

FingerPrint FP;

uint8_t FingerPrint::fpm_sendAndReceive(uint16_t timeout) {
    uint8_t i, j;
    uint8_t checkSum = 0;

    FP.RxCnt    = 0;
    FP.TxBuf[5] = 0;

    Serial2.write(FINGER_CMD_HEAD);
    for (i = 1; i < 6; i++) {
        Serial2.write(FP.TxBuf[i]);
        checkSum ^= FP.TxBuf[i];
    }
    Serial2.write(checkSum);
    Serial2.write(FINGER_CMD_TAIL);

    while (FP.RxCnt < 8 && timeout > 0) {
        delay(1);
        timeout--;
    }

    uint8_t ch;
    for (i = 0; i < 8; i++) {
        if (Serial2.available()) {
            ch = Serial2.read();
            FP.RxCnt++;
            FP.RxBuf[i] = ch;
        }
    }

    if (FP.RxCnt != 8) {
        FP.RxCnt = 0;
        return FINGER_ACK_TIMEOUT;
    }
    if (FP.RxBuf[FINGER_HEAD] != FINGER_CMD_HEAD) return FINGER_ACK_FAIL;
    if (FP.RxBuf[FINGER_TAIL] != FINGER_CMD_TAIL) return FINGER_ACK_FAIL;
    if (FP.RxBuf[FINGER_CMD] != (FP.TxBuf[FINGER_CMD])) return FINGER_ACK_FAIL;

    checkSum = 0;
    for (j = 1; j < FINGER_CHK; j++) {
        checkSum ^= FP.RxBuf[j];
    }
    if (checkSum != FP.RxBuf[FINGER_CHK]) {
        return FINGER_ACK_FAIL;
    }
    return FINGER_ACK_SUCCESS;
}

uint8_t FingerPrint::fpm_sleep(void) {
    uint8_t res;

    FP.TxBuf[FINGER_CMD] = FINGER_CMD_SLEEP_MODE;
    FP.TxBuf[FINGER_P1]  = 0;
    FP.TxBuf[FINGER_P2]  = 0;
    FP.TxBuf[FINGER_P3]  = 0;

    res = fpm_sendAndReceive(500);

    if (res == FINGER_ACK_SUCCESS) {
        return FINGER_ACK_SUCCESS;
    } else {
        return FINGER_ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_setAddMode(uint8_t fpm_mode) {
    uint8_t res;

    FP.TxBuf[FINGER_CMD] = FINGER_CMD_ADD_MODE;
    FP.TxBuf[FINGER_P1]  = 0;
    FP.TxBuf[FINGER_P2]  = fpm_mode;
    FP.TxBuf[FINGER_P3]  = 0;

    res = fpm_sendAndReceive(200);

    if (res == FINGER_ACK_SUCCESS && RxBuf[FINGER_Q3] == FINGER_ACK_SUCCESS) {
        return FINGER_ACK_SUCCESS;
    } else {
        return FINGER_ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_readAddMode(void) {
    FP.TxBuf[FINGER_CMD] = FINGER_CMD_ADD_MODE;
    FP.TxBuf[FINGER_P1]  = 0;
    FP.TxBuf[FINGER_P2]  = 0;
    FP.TxBuf[FINGER_P3]  = 0X01;

    fpm_sendAndReceive(200);

    return FP.RxBuf[FINGER_Q2];
}

uint16_t FingerPrint::fpm_getUserNum(void) {
    uint8_t res;

    FP.TxBuf[FINGER_CMD] = FINGER_CMD_USER_CNT;
    FP.TxBuf[FINGER_P1]  = 0;
    FP.TxBuf[FINGER_P2]  = 0;
    FP.TxBuf[FINGER_P3]  = 0;

    res = fpm_sendAndReceive(200);

    if (res == FINGER_ACK_SUCCESS && RxBuf[FINGER_Q3] == FINGER_ACK_SUCCESS) {
        return FP.RxBuf[FINGER_Q2];
    } else {
        return 0XFF;
    }
}

uint8_t FingerPrint::fpm_deleteAllUser(void) {
    uint8_t res;

    FP.TxBuf[FINGER_CMD] = FINGER_CMD_DEL_ALL;
    FP.TxBuf[FINGER_P1]  = 0;
    FP.TxBuf[FINGER_P2]  = 0;
    FP.TxBuf[FINGER_P3]  = 0;

    res = fpm_sendAndReceive(200);

    if (res == FINGER_ACK_SUCCESS && RxBuf[FINGER_Q3] == FINGER_ACK_SUCCESS) {
        return FINGER_ACK_SUCCESS;
    } else {
        return FINGER_ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_deleteUser(uint8_t userNum) {
    uint8_t res;

    FP.TxBuf[FINGER_CMD] = FINGER_CMD_DEL;
    FP.TxBuf[FINGER_P1]  = 0;
    FP.TxBuf[FINGER_P2]  = userNum;
    FP.TxBuf[FINGER_P3]  = 0;

    res = fpm_sendAndReceive(200);

    if (res == FINGER_ACK_SUCCESS && RxBuf[FINGER_Q3] == FINGER_ACK_SUCCESS) {
        return FINGER_ACK_SUCCESS;
    } else {
        return FINGER_ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_addUser(uint8_t userNum, uint8_t userPermission) {
    uint8_t res;

    FP.TxBuf[FINGER_CMD] = FINGER_CMD_ADD_1;
    FP.TxBuf[FINGER_P1]  = 0;
    FP.TxBuf[FINGER_P2]  = userNum;
    FP.TxBuf[FINGER_P3]  = userPermission;

    res = fpm_sendAndReceive(3000);

    if (res == FINGER_ACK_SUCCESS) {
        if (FP.RxBuf[FINGER_Q3] == FINGER_ACK_SUCCESS) {
            FP.TxBuf[FINGER_CMD] = FINGER_CMD_ADD_2;

            res = fpm_sendAndReceive(3000);

            if (res == FINGER_ACK_SUCCESS) {
                if (FP.RxBuf[FINGER_Q3] == FINGER_ACK_SUCCESS) {
                    FP.TxBuf[FINGER_CMD] = FINGER_CMD_ADD_3;
                    res           = fpm_sendAndReceive(3000);
                    if (res == FINGER_ACK_SUCCESS) {
                        return FP.RxBuf[FINGER_Q3];
                    }
                }
            }
        }
    }
    return res;
}

uint8_t FingerPrint::fpm_compareFinger(void) {
    uint8_t res;

    FP.TxBuf[FINGER_CMD] = FINGER_CMD_MATCH;
    FP.TxBuf[FINGER_P1]  = 0;
    FP.TxBuf[FINGER_P2]  = 0;
    FP.TxBuf[FINGER_P3]  = 0;

    res = fpm_sendAndReceive(3000);

    if (res == FINGER_ACK_SUCCESS) {
        if (FP.RxBuf[FINGER_Q3] == FINGER_ACK_NOUSER) {
            return FINGER_ACK_NOUSER;
        }
        if (FP.RxBuf[FINGER_Q3] == FINGER_ACK_TIMEOUT) {
            return FINGER_ACK_TIMEOUT;
        }
        if ((FP.RxBuf[FINGER_Q2] != 0) &&
            (FP.RxBuf[FINGER_Q3] == 1 || FP.RxBuf[FINGER_Q3] == 2 || FP.RxBuf[FINGER_Q3] == 3)) {
            return FINGER_ACK_SUCCESS;
        }
    }
    return res;
}

String FingerPrint::fpm_showAllUser() {
  uint8_t res;

  FP.TxBuf[FINGER_CMD] = FINGER_CMD_ALL_USER;
  FP.TxBuf[FINGER_P1]  = 0;
  FP.TxBuf[FINGER_P2]  = 0;
  FP.TxBuf[FINGER_P3]  = 0;

  res = fpm_sendAndReceive(200);
  uint16_t dataLength = (FP.RxBuf[FINGER_Q1] * 16 * 16) + FP.RxBuf[FINGER_Q2];

  if (res == FINGER_ACK_SUCCESS) {
    if (FP.RxBuf[FINGER_Q3] == FINGER_ACK_FAIL) {
      return "User read failed.";
    }
    if (FP.RxBuf[FINGER_Q3] == FINGER_ACK_SUCCESS) {
      delay(100);
      return fpm_ReceiveLongData(dataLength);
      // String result = "";
      // result += "dataLength: " + String(dataLength);
      // while(Serial2.available()){
      //   result += String(Serial2.read()) + ",";
      // }
      // return result;
    }
  }
  return String(FP.RxBuf[FINGER_Q3]);
}

String FingerPrint::fpm_ReceiveLongData(uint16_t dataLength) {
  uint8_t i;
  String result = "";
  uint8_t longRXBuf[dataLength];
  uint8_t ch;

  if(Serial2.available()) {
    if (Serial2.read() != FINGER_CMD_HEAD) return "miss data";
  }

  for (i = 0 ; i < dataLength ; i++) {
    if (Serial2.available()) {
      ch = Serial2.read();
      longRXBuf[i] = ch;
    }
  }
  //if (longRXBuf[0] != 0xF5) return "miss data";
  uint16_t userNum;
  for (i = 2 ; i < dataLength ; i++) {
    if (i%3 == 2) userNum = longRXBuf[i]*16*16;
    if (i%3 == 0) {
      userNum += longRXBuf[i];
      result += "{uNo: " + String(userNum);
      userNum = 0;
    }
    if (i%3 == 1) result += ", auth: " +  String(longRXBuf[i]) + "}, ";
  }
   if(Serial2.available()) {
    Serial2.read();
    if (Serial2.read() != FINGER_CMD_HEAD) return "miss data";
  }
  return result;
}

uint8_t FingerPrint::getUID() {
  return FP.RxBuf[3];
}
