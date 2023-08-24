#include "config.h"       // WiFi情報、Functionsのurlとendpointが入っている
#include "finger.h"
#include "user_sd.h"
#include "functions.h"
#include <Arduino.h>
#include <M5Unified.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define TRIG 2
#define ECHO 5
#define PIN_THERMISTOR 35  // analog read pin

#define THERMISTOR_B 3431
#define THERMISTOR_R0 10000
#define THERMISTOR_T0 25
#define RESISTOR_PULL_DOWN 10000
#define ANALOG_MAX 4095
#define TURN_OFF_TIME (1000 * 60 * 5)

bool sensorFlg = false;
double duration = 0;
double distance = 0;                      // 距離
double speed_of_sound = 331.5 + 0.6 * 25; // 25℃の気温の想定

double temperature = 0.0;                 // 温度

enum Screens {
  STANDBY,
  SETTING,
  SOLDERING
};

Screens currentScreens = STANDBY;

uint8_t userNum;              // ユーザー数
uint8_t currentUID;           // 指紋モジュールのユーザーID
String fuserID;               // FunctionsのユーザーID
String fuserName;             // Functionsのユーザー名
String logID;                 // FunctionsのログID
uint8_t fingerResult;         // 指紋モジュール結果
FingerPrint FP_M;             // 指紋モジュール通信インスタンス
UserManagement UM_S;          // SDカードユーザー管理インスタンス
FunctionsTransmission FT_S;   // Cloud Functions通信インスタンス


float calcTempratureByResistor(float R, int B, int R0, float T0) {
  return 1 / (log(R / R0) / B + (1.0 / (T0 + 273))) - 273;
}

float calcResistorByAnalogValue(uint16_t analog, uint16_t analogMax,
                                float resistorPullDown) {
  return (double)(analogMax - analog) / analog * resistorPullDown;
}

// はんだごてのセンサーを監視するタスク
void solderingSensorTask(void *parameter) {
  while (true) {
    digitalWrite(TRIG, LOW); 
    delayMicroseconds(2); 
    digitalWrite( TRIG, HIGH );
    delayMicroseconds( 10 ); 
    digitalWrite( TRIG, LOW );
    duration = pulseIn( ECHO, HIGH ); // 往復にかかった時間が返却される[マイクロ秒]

    if (duration > 0) {
      duration = duration / 2; // 往路にかかった時間
      distance = duration * speed_of_sound * 100 / 1000000;
    }

    uint16_t valAnalog = analogRead(PIN_THERMISTOR);
    float resistor =
        calcResistorByAnalogValue(valAnalog, ANALOG_MAX, RESISTOR_PULL_DOWN);
    temperature = calcTempratureByResistor(resistor, THERMISTOR_B,
                                                 THERMISTOR_R0, THERMISTOR_T0);

    if (sensorFlg) {
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.fillRect(0, 20, 350, 300, BLACK);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.setTextFont(2);
      M5.Lcd.print("temp:");
      M5.Lcd.println(temperature);
    }
    delay(1000);
  }
}

// 画面の初期化
void cleanScreen() {
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.fillRect(0, 20, 350, 300, BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextFont(2);
  userNum = FP_M.fpm_getUserNum();
  M5.Lcd.print("userNum:");
  M5.Lcd.println(userNum);
}

// WiFi接続
void setup() {
  auto cfg = M5.config();
  cfg.clear_display = true;
  M5.begin(cfg);

  Serial2.begin(19200, SERIAL_8N1, 3, 1);     // 3ピンをRX(受信), 1ピンをTX(送信)にする
  delay(1000);                                // シリアル通信が有効になるまで待つ
  FP_M.fpm_setAddMode(0x00);                  // 指紋データの重複を許す

  pinMode(ECHO, INPUT);
  pinMode(TRIG, OUTPUT);

  M5.Lcd.begin();
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  while (!UM_S.SDEnable()) {
    delay(500);
    M5.Lcd.println("SD disable");
  }
  M5.Lcd.fillRect(0, 0, 350, 300, BLACK);

  connectWiFi();

  // センサータスクの作成
  xTaskCreate(
      solderingSensorTask,        /* タスク関数 */
      "solderingSensorTask",     /* タスク名 */
      1000,        /* スタックサイズ */
      NULL,         /* タスクのパラメータ */
      1,            /* このタスクの優先度 */
      NULL          /* タスクハンドル */
  );

  M5.Lcd.println("Standby Screen");

}

void loop() {
  switch (currentScreens) {
    case STANDBY:
     standbyScreen();
     break;
    case SETTING:
     settingScreen();
     break;
    case SOLDERING:
     solderingScreen();
     break;
    default:
     break;
  }
  M5.update();
}

// 待機画面
void standbyScreen() {
  if (M5.BtnA.wasPressed()) {
    registerUser();
  }
  if (M5.BtnB.wasPressed()) {
    authenticateUser();
  }
  if (M5.BtnC.wasPressed()) {
    cleanScreen();
    M5.Lcd.println("Setting Screen");
    currentScreens = SETTING;
  }
}

// 設定画面
void settingScreen() {
  if (M5.BtnA.wasPressed()) {
    //FP_M.fpm_showAllUser();
  }
  if (M5.BtnB.wasPressed()) {
    // 空き
  }
  if (M5.BtnC.wasPressed()) {
    cleanScreen();
    M5.Lcd.println("Standby Screen");
    currentScreens = STANDBY;
  }
}

// はんだごて使用中画面
void solderingScreen() {
  if (M5.BtnA.wasPressed()) {
    solderingFinish();
  }
  if (M5.BtnB.wasPressed()) {
    // 空き
  }
  if (M5.BtnC.wasPressed()) {
    // 空き
  }
}

// WiFiに接続する
void connectWiFi() {
  M5.Lcd.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    M5.Lcd.print(".");
  }
  M5.Lcd.println("\nConnected!");
  delay(500);

  cleanScreen();
}

// ユーザー登録処理
void registerUser() {
  if (waitForConnectionBLE()) {
    String  funcuid = getUserIDBLE();
    String  uname   = getUserNameBLE();
    uint8_t fingeruid = userNum+1;
    uint8_t res     = FP_M.fpm_addUser(fingeruid, 1);     // 今後小さい順の空いているidにする
    if (res == ACK_SUCCESS) {
      //res = FP_M.fpm_compareFinger();
      if (res == ACK_SUCCESS) {
        M5.Lcd.println("Add User");
        if (!UM_S.saveUserData(fingeruid, funcuid, uname)) {
          M5.Lcd.println("Fail SD write");
          FP_M.fpm_deleteUser(fingeruid);
        }
      } else {
        // 登録はできたが認証ができなかった
        // もう一回認証する
      }
    } else if (res == ACK_FAIL) {
      // 登録失敗
    } else if (res == ACK_FULL) {
      // 登録数最大
    } else {
      // タイムアウト
    }
  } else {
    // 待機画面に戻る
  }
  userNum = FP_M.fpm_getUserNum();
}

// BLE接続待ち
bool waitForConnectionBLE() {
  return true;
}

// uidを受け取る
String getUserIDBLE() {
  return "0ATMiNgfn71RkoSu5zeL";
}

// unameを受け取る
String getUserNameBLE() {
  return "kuroda";
}

// ユーザー認証
void authenticateUser() {
  M5.Lcd.println("Matching");
  fingerResult = FP_M.fpm_compareFinger();
  if (fingerResult == ACK_SUCCESS) {
      M5.Lcd.println("Success");
      currentUID = FP_M.getUID();
      M5.Lcd.println(currentUID);
      if (UM_S.existUserData(currentUID)) {
        M5.Lcd.println("soldering start");
        solderingStart();
      } else {
        // モジュールには登録されているが、SDに保存されていない

      }
  }
  if (fingerResult == ACK_NOUSER) {
      M5.Lcd.println("No Such User");
  }
  if (fingerResult == ACK_TIMEOUT) {
      M5.Lcd.println("Timeout");
  }
}

// はんだごて使用開始
void solderingStart() {
  String userData = UM_S.getUserData(currentUID);
  // JSONドキュメントのバッファを作成
  StaticJsonDocument<256> doc;
  // JSON文字列をパース
  DeserializationError error = deserializeJson(doc, userData);
  if (error) {
    Serial.println(F("deserializeJson() failed"));
    return;
  }
  fuserID = doc["functionsUserID"].as<String>();
  fuserName = doc["functionsUserName"].as<String>();
  String postData = "{\"user_id\": \"" + fuserID + "\", \"device_id\": \"" + String(solderingID) + "\"}";
  String response = FT_S.functions_post(String(functionsUrl), String(startEndpoint), postData);
  if (response == "User not found") {
    // Functions側で存在しないユーザー
  } else if (response == "User not authorized") {
    // 承認されていないユーザー
  } else if (response == "reservation exists") {
    // ほかに予約している人がいる
  } else {
    logID = response.substring(2, response.length() - 2);
    cleanScreen();
    M5.Lcd.println("Using Name: " + fuserName);
    M5.Lcd.println("Soldering Screen");
    delay(2000);
    sensorFlg = true;
    currentScreens = SOLDERING;
  }
}

// はんだごて使用終了
void solderingFinish() {
  sensorFlg = false;
  forgetTurnOffAlert();
  String postData = "{\"" + logID + "\"}";
  String response = FT_S.functions_post(String(functionsUrl), String(endEndpoint), postData);
  if (response != "Internal Server Error") {
    M5.Lcd.println("handa finish.");
    delay(3000);
    cleanScreen();
    M5.Lcd.println("Standby Screen");
    currentScreens = STANDBY;
  } else {
    M5.Lcd.println("Server Error.");
    //solderingFinish();
  }
}

// はんだごて切り忘れ通知
void forgetTurnOffAlert() {
  String postData = "{\"user_id\": \"" + fuserID + "\", \"device_id\": \"" + String(solderingID) + "\", \"" + logID + "\"}";
  String response = FT_S.functions_post(String(functionsUrl), String(alertEndpoint), postData);
  if (response != "Internal Server Error") {
    M5.Lcd.println("handa alert.");
  } else {
    M5.Lcd.println("Server Error.");
    //forgetTurnOffAlert();
  }
}

