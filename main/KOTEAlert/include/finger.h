#pragma once

#include <stdint.h>
#include <Arduino.h>

#define FINGER_TRUE  1
#define FINGER_FALSE 0

#define FINGER_ACK_SUCCESS    0x00
#define FINGER_ACK_FAIL       0x01
#define FINGER_ACK_FULL       0x04
#define FINGER_ACK_NOUSER     0x05
#define FINGER_ACK_USER_EXIST 0x07
#define FINGER_ACK_TIMEOUT    0x08

#define FINGER_ACK_GO_OUT 0x0F

#define FINGER_ACK_ALL_USER    0x00
#define FINGER_ACK_GUEST_USER  0x01
#define FINGER_ACK_NORMAL_USER 0x02
#define FINGER_ACK_MASTER_USER 0x03

#define FINGER_USER_MAX_CNT 50

#define FINGER_HEAD 0
#define FINGER_CMD  1
#define FINGER_CHK  6
#define FINGER_TAIL 7

#define FINGER_P1 2
#define FINGER_P2 3
#define FINGER_P3 4
#define FINGER_Q1 2
#define FINGER_Q2 3
#define FINGER_Q3 4

#define FINGER_CMD_HEAD       0xF5
#define FINGER_CMD_TAIL       0xF5
#define FINGER_CMD_ADD_1      0x01
#define FINGER_CMD_ADD_2      0x02
#define FINGER_CMD_ADD_3      0x03
#define FINGER_CMD_MATCH      0x0C
#define FINGER_CMD_DEL        0x04
#define FINGER_CMD_DEL_ALL    0x05
#define FINGER_CMD_USER_CNT   0x09
#define FINGER_CMD_ALL_USER   0x2B
#define FINGER_CMD_SLEEP_MODE 0x2C
#define FINGER_CMD_ADD_MODE   0x2D

#define FINGER_CMD_DETECTED 0x14

class FingerPrint {
   public:
    FingerPrint(void);
    uint8_t fpm_sendAndReceive(uint16_t delayMs);
    uint8_t fpm_sleep(void);
    uint8_t fpm_setAddMode(uint8_t fpm_mode);
    uint8_t fpm_readAddMode(void);
    uint16_t fpm_getUserNum(void);
    uint8_t fpm_deleteAllUser(void);
    uint8_t fpm_deleteUser(uint8_t userNum);
    uint8_t fpm_addUser(uint8_t userNum, uint8_t userPermission);
    uint8_t fpm_compareFinger(void);
    String fpm_showAllUser();
    String fpm_ReceiveLongData(uint16_t dataLength);

   public:
    uint8_t TxBuf[9];
    uint8_t RxBuf[9];
    uint8_t RxCnt;
    uint8_t getUID();

   private:
   private:
};
