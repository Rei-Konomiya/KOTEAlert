#include "config.h"
#include "finger.h"
#include "user_sd.h"
#include "functions.h"
#include "soldering_sensor.h"
#include <Arduino.h>
#include <M5Unified.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <M5GFX.h>

#define TRIG 2
#define ECHO 5
#define RIRER 23
#define THERMISTOR 35
#define TURN_OFF_TIME (1000 * 60 * 5)

enum Screens {STANDBY, SETTING, SOLDERING};
Screens currentScreens = STANDBY;

bool sensorDispFlg = false;   // センサー値を読み取るかどうか
float temperature = 0.0;      // 温度
unsigned long startTime;      // はんだごての使用開始時間
unsigned long returnStartTime;// はんだごての納刀開始時間
uint8_t userNum;              // ユーザー数
uint8_t currentUID;           // 指紋モジュールのユーザーID
String fuserID;               // FunctionsのユーザーID
String fuserName;             // Functionsのユーザー名
String logID;                 // FunctionsのログID

FingerPrint FP_M;             // 指紋モジュール通信インスタンス
UserManagement UM_S;          // SDカードユーザー管理インスタンス
SolderingSensor SS_S;         // はんだごてセンサー類インスタンス
FunctionsTransmission FT_S;   // Cloud Functions通信インスタンス

M5GFX lcd;                    // 直接表示のインスタンスを作成（M5GFXクラスを使ってlcdコマンドでいろいろできるようにする）
M5Canvas tabName(&lcd);
M5Canvas home(&lcd);
M5Canvas wifiImage(&lcd);

boolean change = false;
boolean wifiConnect = false;


#pragma once
const char* solderingID = "F8tskdHqB19OUUA8cgVI";   // はんだごて1号

const char* ssid = "C0p2Ec2-WLAN";
const char* pass = "4Emah5LdS";


const char* functionsUrl = "us-central1-smart-handagote.cloudfunctions.net";
const char* testEndpoint = "/userTest";
const char* startEndpoint = "/startUsing";
const char* endEndpoint = "/endUsing";
const char* alertEndpoint = "/alert";


/*
* Wi-Fiマークを表示するメソッド
*/
void draw_wifi(){
  wifiImage.clear(BLACK);
  if(wifiConnect){
    wifiImage.fillCircle(13, 13, 2, WHITE);
    wifiImage.drawArc(13, 13, 7, 7, 215, 325, WHITE);
    wifiImage.drawArc(13, 13, 13, 13, 215, 325, WHITE);
  }else{
    wifiImage.fillCircle(13, 13, 2, RED);
    wifiImage.drawArc(13, 13, 7, 7, 215, 325, RED);
    wifiImage.drawArc(13, 13, 13, 13, 215, 325, RED);
    wifiImage.drawLine(0, 0, wifiImage.width(), wifiImage.height(), RED);
    wifiImage.drawLine(wifiImage.width(), 0, 0, wifiImage.height(), RED);
  }
  wifiImage.pushSprite(&lcd, lcd.width() -25, 0);
}

/*
* タブ全体のスプライトを作る・表示するメソッド
*/
void draw_btn(String tabA, String tabB, String tabC){
  draw_wifi();
  tabName.clear(BLACK);
  String btnName[] = {tabA, tabB, tabC};
  for(int i=0; i<3; i++){
    if(btnName[i] != ""){
      tabName.fillRoundRect (105*i, 0, 100, 100, 20, DARKCYAN);
      tabName.fillRoundRect (105*i+2, 2, 96, 96, 18, BLACK);
      tabName.setTextColor(WHITE);
      
      tabName.setFont(&fonts::lgfxJapanGothic_36);
      tabName.setCursor(105*i+14, 13);
      tabName.print(btnName[i]);
    }
  }
  tabName.pushSprite(&lcd, 5, 60*3);
}

/*
* 待機画面表示メソッド
*/
void draw_home(){
  int logo_adjustH = 50;
  int logo_adjustV = -13;

  home.clear(BLACK);
  //KOTE
  home.drawLine(logo_adjustH+55, logo_adjustV+25, logo_adjustH+55, logo_adjustV+145, WHITE);  //K
  home.drawLine(logo_adjustH+55, logo_adjustV+25, logo_adjustH+70, logo_adjustV+25, WHITE);
  home.drawLine(logo_adjustH+70, logo_adjustV+25, logo_adjustH+70, logo_adjustV+55, WHITE);
  home.drawLine(logo_adjustH+70, logo_adjustV+55, logo_adjustH+100, logo_adjustV+25, WHITE);
  home.drawLine(logo_adjustH+100, logo_adjustV+25, logo_adjustH+120, logo_adjustV+25, WHITE);
  home.drawLine(logo_adjustH+120, logo_adjustV+25, logo_adjustH+85, logo_adjustV+65, WHITE);
  home.drawLine(logo_adjustH+85, logo_adjustV+65, logo_adjustH+120, logo_adjustV+105, WHITE);
  home.drawLine(logo_adjustH+120, logo_adjustV+105, logo_adjustH+180, logo_adjustV+105, WHITE);
  home.drawEllipse (logo_adjustH+155, logo_adjustV+65, 35, 40, WHITE); //O
  home.drawEllipse (logo_adjustH+155, logo_adjustV+65, 25, 30, WHITE);
  home.drawLine(logo_adjustH+150, logo_adjustV+105, logo_adjustH+160, logo_adjustV+105, BLACK); //E
  home.drawLine(logo_adjustH+180, logo_adjustV+105, logo_adjustH+180, logo_adjustV+115, WHITE);
  home.drawLine(logo_adjustH+180, logo_adjustV+115, logo_adjustH+130, logo_adjustV+115, WHITE);
  home.drawLine(logo_adjustH+130, logo_adjustV+115, logo_adjustH+130, logo_adjustV+130, WHITE);
  home.drawLine(logo_adjustH+130, logo_adjustV+130, logo_adjustH+170, logo_adjustV+130, WHITE);
  home.drawLine(logo_adjustH+170, logo_adjustV+130, logo_adjustH+170, logo_adjustV+140, WHITE);
  home.drawLine(logo_adjustH+170, logo_adjustV+140, logo_adjustH+130, logo_adjustV+140, WHITE);
  home.drawLine(logo_adjustH+130, logo_adjustV+140, logo_adjustH+130, logo_adjustV+160, WHITE);
  home.drawLine(logo_adjustH+130, logo_adjustV+160, logo_adjustH+180, logo_adjustV+160, WHITE);
  home.drawLine(logo_adjustH+180, logo_adjustV+160, logo_adjustH+180, logo_adjustV+170, WHITE);
  home.drawLine(logo_adjustH+180, logo_adjustV+170, logo_adjustH+120, logo_adjustV+170, WHITE);
  home.drawLine(logo_adjustH+120, logo_adjustV+170, logo_adjustH+120, logo_adjustV+115, WHITE);
  home.drawLine(logo_adjustH+120, logo_adjustV+115, logo_adjustH+100, logo_adjustV+115, WHITE); //T
  home.drawLine(logo_adjustH+100, logo_adjustV+115, logo_adjustH+100, logo_adjustV+175, WHITE);
  home.drawLine(logo_adjustH+100, logo_adjustV+175, logo_adjustH+80, logo_adjustV+175, WHITE);
  home.drawLine(logo_adjustH+80, logo_adjustV+175, logo_adjustH+80, logo_adjustV+120, WHITE);
  home.drawLine(logo_adjustH+80, logo_adjustV+120, logo_adjustH+55, logo_adjustV+120, WHITE);
  home.drawLine(logo_adjustH+55, logo_adjustV+105, logo_adjustH+105, logo_adjustV+105, WHITE);  //K
  home.drawLine(logo_adjustH+105, logo_adjustV+105, logo_adjustH+80, logo_adjustV+75, WHITE);
  home.drawLine(logo_adjustH+80, logo_adjustV+75, logo_adjustH+70, logo_adjustV+80, WHITE);
  home.drawLine(logo_adjustH+70, logo_adjustV+80, logo_adjustH+70, logo_adjustV+105, WHITE);
  
  //Alert
  home.drawLine(logo_adjustH+55, logo_adjustV+145, logo_adjustH+45, logo_adjustV+150, WHITE); //A
  home.drawLine(logo_adjustH+45, logo_adjustV+150, logo_adjustH+45, logo_adjustV+160, WHITE);
  home.drawLine(logo_adjustH+45, logo_adjustV+160, logo_adjustH+55, logo_adjustV+165, WHITE);
  home.drawLine(logo_adjustH+55, logo_adjustV+165, logo_adjustH+55, logo_adjustV+170, WHITE);
  home.drawLine(logo_adjustH+55, logo_adjustV+170, logo_adjustH+15, logo_adjustV+160, WHITE);
  home.drawLine(logo_adjustH+15, logo_adjustV+160, logo_adjustH+15, logo_adjustV+150, WHITE);
  home.drawLine(logo_adjustH+15, logo_adjustV+150, logo_adjustH+55, logo_adjustV+140, WHITE);
  home.drawLine(logo_adjustH+40, logo_adjustV+152, logo_adjustH+40, logo_adjustV+158, WHITE);
  home.drawLine(logo_adjustH+40, logo_adjustV+158, logo_adjustH+25, logo_adjustV+155, WHITE);
  home.drawLine(logo_adjustH+25, logo_adjustV+155, logo_adjustH+40, logo_adjustV+152,WHITE);
  home.drawLine(logo_adjustH+55, logo_adjustV+135, logo_adjustH+15, logo_adjustV+135, WHITE); //l
  home.drawLine(logo_adjustH+15, logo_adjustV+135, logo_adjustH+15, logo_adjustV+125, WHITE);
  home.drawLine(logo_adjustH+15, logo_adjustV+125, logo_adjustH+55, logo_adjustV+125, WHITE);
  home.drawArc(logo_adjustH+40, logo_adjustV+105, 15, 8, 290, 270, WHITE);  //e
  home.fillRect(logo_adjustH+35, logo_adjustV+95, 5, 20, BLACK);
  home.drawLine(logo_adjustH+35, logo_adjustV+100, logo_adjustH+35, logo_adjustV+110, WHITE);
  home.drawLine(logo_adjustH+40, logo_adjustV+97, logo_adjustH+40, logo_adjustV+113, WHITE);
  home.drawLine(logo_adjustH+55, logo_adjustV+85, logo_adjustH+25, logo_adjustV+85, WHITE);  //r
  home.drawLine(logo_adjustH+25, logo_adjustV+85, logo_adjustH+25, logo_adjustV+75, WHITE);
  home.drawLine(logo_adjustH+25, logo_adjustV+75, logo_adjustH+55, logo_adjustV+75, WHITE);
  home.drawArc(logo_adjustH+40, logo_adjustV+67, 15, 8, 110, 180, WHITE);
  home.fillRect(logo_adjustH+28, logo_adjustV+75, 10, 7, BLACK);
  home.drawArc(logo_adjustH+47, logo_adjustV+47, 8, 3, 315, 90, WHITE); //t
  home.drawLine(logo_adjustH+47, logo_adjustV+45, logo_adjustH+47, logo_adjustV+55, BLACK);
  home.drawLine(logo_adjustH+47, logo_adjustV+55, logo_adjustH+30, logo_adjustV+55, WHITE);
  home.drawLine(logo_adjustH+30, logo_adjustV+55, logo_adjustH+30, logo_adjustV+60, WHITE);
  home.drawLine(logo_adjustH+30, logo_adjustV+60, logo_adjustH+25, logo_adjustV+60, WHITE);
  home.drawLine(logo_adjustH+25, logo_adjustV+60, logo_adjustH+25, logo_adjustV+55, WHITE);
  home.drawLine(logo_adjustH+25, logo_adjustV+55, logo_adjustH+20, logo_adjustV+55, WHITE);
  home.drawLine(logo_adjustH+20, logo_adjustV+55, logo_adjustH+20, logo_adjustV+50, WHITE);
  home.drawLine(logo_adjustH+20, logo_adjustV+50, logo_adjustH+25, logo_adjustV+50, WHITE);
  home.drawLine(logo_adjustH+25, logo_adjustV+50, logo_adjustH+25, logo_adjustV+45, WHITE);
  home.drawLine(logo_adjustH+25, logo_adjustV+45, logo_adjustH+30, logo_adjustV+45, WHITE);
  home.drawLine(logo_adjustH+30, logo_adjustV+45, logo_adjustH+30, logo_adjustV+50, WHITE);
  home.drawLine(logo_adjustH+30, logo_adjustV+50, logo_adjustH+47, logo_adjustV+50, WHITE);

  home.pushSprite(&lcd, 0, 0);
}

// はんだごてのセンサーを監視するタスク
void solderingSensorTask(void *parameter) {
  while (true) {
    if (sensorDispFlg) {
      temperature = SS_S.readTemperature();
      if (SS_S.readDistance() < 5.0 /*&& temperature > 50*/) {
        // はんだ放置中
      } else {
        // はんだ使用中




        returnStartTime = millis();
      }
    } else {
      
    }
    delay(300);
  }
}

// WiFi接続を監視するタスク
void WiFiConnectionTask(void *parameter) {
  while (true) {
    static int delayTime = 10000;
    static int count = 0;
    if (WiFi.status() != WL_CONNECTED) {
      wifiConnect = false;
      delayTime = 1000;
      if(count == 0) {
        WiFi.begin(ssid, pass);
      } else if (count == 5) {
        WiFi.begin(ssid, pass);
        count = 1;
      }
      count++;
    } else {
      count = 0;
      delayTime = 10000;
      wifiConnect = true;
    }
    delay(delayTime);
  }
}

void WiFiConnect() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    M5.Lcd.fillRect(0, 0, 350, 300, BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextFont(2);
    M5.Lcd.println("Wifi-connecting...");
    delay(1000);
  }
  M5.Lcd.fillRect(0, 0, 350, 300, BLACK);
}

void setup() {
  Serial.begin(115200);
  delay(10);

  auto cfg = M5.config();
  cfg.clear_display = true;
  M5.begin(cfg);


  WiFiConnect();
  

  delay(100);

  // while (FP_M.fpm_getUserNum() == 255) {
  //   M5.Lcd.fillRect(0, 0, 350, 300, BLACK);
  //   M5.Lcd.setCursor(0, 20);
  //   M5.Lcd.setTextFont(2);
  //   M5.Lcd.println("disable finger module");
  //   delay(500);
  // }

  FP_M.fpm_setAddMode(0x00);                  // 指紋データの重複を許す

  SS_S.attach(TRIG, ECHO, RIRER, THERMISTOR);

  while (!UM_S.SDEnable()) {
    M5.Lcd.fillRect(0, 0, 350, 300, BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextFont(2);
    M5.Lcd.println("SD disable");
    delay(500);
  }

  lcd.begin();
  lcd.setBrightness(64);

  delay(100);

  tabName.setColorDepth(8);
  tabName.createSprite(310, 60);
  home.setColorDepth(8);
  home.createSprite(lcd.width()-25, 180);
  wifiImage.setColorDepth(8);
  wifiImage.createSprite(25, 15);

  delay(100);
  Serial2.begin(19200, SERIAL_8N1, 3, 1);     // 3ピンをRX(受信), 1ピンをTX(送信)にする
  delay(1000);

  // センサータスクの作成
  xTaskCreate(
      solderingSensorTask,        /* タスク関数 */
      "solderingSensorTask",     /* タスク名 */
      1000,        /* スタックサイズ */
      NULL,         /* タスクのパラメータ */
      2,            /* このタスクの優先度 */
      NULL          /* タスクハンドル */
  );

  // WiFi接続確認のタスク作成
  xTaskCreate(WiFiConnectionTask, "WiFiConnectionTask", 1000, NULL, 1, NULL);
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
    currentScreens = STANDBY;
  }
}

/*
* 待機メソッド（delayではボタン判定ができないため代わりにこのメソッドを使う）
* time : 待機時間
* now : 現在選択中のモード（ボタン判定によってモード切替がループしないようにするため）
*/
void wait(int time){
  for(int i=0; i<time; i++){
    M5.update();
    if(M5.BtnC.wasPressed()) change = true;
    
    if(change) break;
    delay(1);
  }
}

// BLE接続
void bleConnecting(){
  boolean BLEconnect = false;   //デバッグ用、実装時は置換する
  int periodCount = 0;
  int timeCount = 0;
  const int timeout = 10;

  while(!BLEconnect){
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("スマホとBluetoothを");
    home.print("接続してください");
    for(int i=0; i<periodCount; i++) home.print(".");
    ++periodCount %= 4;
    home.pushSprite(&lcd, 0, 0);
    wait(1000);
    timeCount++;
    if(timeCount >= timeout){
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("タイムアウトしました");
      home.println("ホーム画面に戻ります");
      home.pushSprite(&lcd, 0, 0);
      change = true;
      delay(2000);
    }
    if(change) return;
    
    //以下2行はデバッグ用、実装時は削除する
    M5.update();
    if(M5.BtnA.isPressed()) BLEconnect = true;
  }
  
  home.clear(BLACK);
  home.setCursor(0, 0);
  home.println("Bluetooth接続完了");
  home.pushSprite(&lcd, 0, 0);
  wait(2000);
}

// BLEデータ受け取り
void receive(){
  fuserID = "0ATMiNgfn71RkoSu5zeL";       //ここでuid受け取り
  fuserName = "黒田 直樹";    //ここでユーザー名受け取り
}

// BLEデータ受け取り待機
void bleReceive(){
  boolean received = false;  //デバッグ用、実装時は置換する
  int periodCount = 0;
  int timeCount = 0;
  const int timeout = 10;

  while(!received){
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("データを");
    home.print("受け取っています");
    for(int i=0; i<periodCount; i++) home.print(".");
    ++periodCount %= 4;
    home.pushSprite(&lcd, 0, 0);
    wait(1000);
    timeCount++;
    if(timeCount >= timeout){
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("タイムアウトしました");
      home.println("ホーム画面に戻ります");
      home.pushSprite(&lcd, 0, 0);
      change = true;
      delay(2000);
    }
    if(change) return;
    
    //以下2行はデバッグ用、実装時は削除する
    M5.update();
    if(M5.BtnA.isPressed()) received = true;
  }
  receive();
  home.clear(BLACK);
  home.setCursor(0, 0);
  home.println("ユーザー情報　受取完了");
  home.pushSprite(&lcd, 0, 0);
  wait(2000);
}


// 指紋登録
void fingerPrint(){
  boolean inited = false;  //デバッグ用、実装時は削除する
  boolean initFail = false;  //デバッグ用、実装時は削除する
  int periodCount = 0;
  int timeCount = 0;
  const int timeout = 10;

  userNum = FP_M.fpm_getUserNum();
  if (userNum == 255) return;

  while(!inited){
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("指紋を登録します");
    home.println("センサーに指を");
    home.print("あてつづけて");
    home.println("ください");
    home.pushSprite(&lcd, 0, 0);
    uint8_t fingeruid = userNum+1;
    uint8_t res     = FP_M.fpm_addUser(fingeruid, 1);
    if (res == ACK_SUCCESS) {
      if (!UM_S.saveUserData(fingeruid, fuserID, fuserName)) {
        home.clear(BLACK);
        home.setCursor(0, 0);
        home.println("SDカードに書き込めませんでした");
        home.pushSprite(&lcd, 0, 0);
        FP_M.fpm_deleteUser(fingeruid);
        delay(2000);
        change = true;
      } else {
        home.clear(BLACK);
        home.setCursor(0, 0);
        home.println("指紋情報　登録完了");
        home.pushSprite(&lcd, 0, 0);
        delay(1500);
        inited = true;
      }
    } else if (res == ACK_FAIL) {
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("登録に失敗しました");
      home.println("再度登録する場合は");
      home.println("「再開」ボタンを");
      home.println("押してください");
      home.pushSprite(&lcd, 0, 0);
      draw_btn("再開", "", "中止");
      while(true){
        M5.update();
        if(M5.BtnA.wasPressed()){
          timeCount = 0;
          periodCount = 0;
          initFail = false;
          while(M5.BtnA.isPressed()) M5.update();
          draw_btn("", "", "中止");
          break;
        }
        if(M5.BtnC.wasPressed()){
          change = true;
          break;
        }
      }
    } else {
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("タイムアウトしました");
      home.println("再度登録する場合は");
      home.println("「再開」ボタンを");
      home.println("押してください");
      home.pushSprite(&lcd, 0, 0);
      draw_btn("再開", "", "中止");
      while(true){
        M5.update();
        if(M5.BtnA.wasPressed()){
          timeCount = 0;
          periodCount = 0;
          while(M5.BtnA.isPressed()) M5.update();
          draw_btn("", "", "中止");
          break;
        }
        if(M5.BtnC.wasPressed()){
          change = true;
          break;
        }
      }
    }
    if(initFail){
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("登録に失敗しました");
      home.println("再度登録する場合は");
      home.println("「再開」ボタンを");
      home.println("押してください");
      home.pushSprite(&lcd, 0, 0);
      draw_btn("再開", "", "中止");
      while(true){
        M5.update();
        if(M5.BtnA.wasPressed()){
          timeCount = 0;
          periodCount = 0;
          initFail = false;
          while(M5.BtnA.isPressed()) M5.update();
          draw_btn("", "", "中止");
          break;
        }
        if(M5.BtnC.wasPressed()){
          change = true;
          break;
        }
      }
    }
    if(change) return;

    //以下5行はデバッグ用、実装時は削除する
    M5.update();
    if(M5.BtnA.isPressed()){
      if(timeCount > 5) initFail = true;
      else inited = true;
    }
  }
}

// ユーザー登録
void registerUser(){
  home.setFont(&fonts::lgfxJapanGothic_24);
  
  bleConnecting(); //BLE接続
  if(!change)bleReceive();    //BLE情報受け取り
  if(!change)fingerPrint();   //指紋登録

}

// はんだごて使用開始
void solderingStart() {
  String userData = UM_S.getUserData(currentUID);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, userData);
  fuserID = doc["functionsUserID"].as<String>();
  fuserName = doc["functionsUserName"].as<String>();

  if (WiFi.status() != WL_CONNECTED) {
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("WiFi接続されていません");
    home.pushSprite(&lcd, 0, 0);
    delay(30000);
  }

  delay(1000);

  String postData = "{\"user_id\": \"" + fuserID + "\", \"device_id\": \"" + String(solderingID) + "\"}";
  String response = FT_S.functions_post(String(functionsUrl), String(startEndpoint), postData);

  if (response == "User not found") {
    // Functions側で存在しないユーザー
  } else if (response == "User not authorized") {
    // 承認されていないユーザー
  } else if (response == "reservation exists") {
    // ほかに予約している人がいる
  } else if (response == "Internal Server Error") {
    // 接続エラー
  } else {
    logID = response.substring(2, response.length() - 2);
    sensorDispFlg = true;
    startTime = millis();
    returnStartTime = millis();
    SS_S.rirerRun(true);
    currentScreens = SOLDERING;
  }
}

// はんだごて使用終了
void solderingFinish() {
  SS_S.rirerRun(false);
  sensorDispFlg = false;
  String postData = "{\"" + logID + "\"}";
  String response = FT_S.functions_post(String(functionsUrl), String(endEndpoint), postData);

  if (response != "Internal Server Error") {
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("KOTEの使用を");
    home.println("終了しました");
    home.pushSprite(&lcd, 0, 0);
    delay(3000);
    currentScreens = STANDBY;
  } else {
    M5.Lcd.println("Server Error.");
    //solderingFinish();
  }
}


// ユーザー認証
void authenticateUser() {
  home.setFont(&fonts::lgfxJapanGothic_24);
  home.clear(BLACK);
  home.setCursor(0, 0);
  home.println("指紋認証を行います");
  home.println("センサーに指を");
  home.print("あててください");
  home.pushSprite(&lcd, 0, 0);
  uint8_t res = FP_M.fpm_compareFinger();
  if (res == ACK_SUCCESS) {
    currentUID = FP_M.getUID();
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("認証成功");
    //home.println(String(currentUID));
    home.pushSprite(&lcd, 0, 0);
    wait(2000);
    if (UM_S.existUserData(currentUID)) {
      solderingStart();
      delay(1000);
    } else {
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("モジュールには登録されていますが、");
      home.println("SDカードに保存されていません");
      home.println("ホーム画面に戻ります");
      home.pushSprite(&lcd, 0, 0);
      delay(2000);
      return;
    }
  }
  if (res == ACK_NOUSER) {
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("認証に失敗しました");
      home.println("ホーム画面に戻ります");
      home.pushSprite(&lcd, 0, 0);
      delay(2000);
      return;
  }
  if (res == ACK_TIMEOUT) {
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("タイムアウトしました");
      home.println("ホーム画面に戻ります");
      home.pushSprite(&lcd, 0, 0);
      delay(2000);
      return;
  }
}

// 待機画面
void standbyScreen() {
  if (M5.BtnA.wasPressed()) {
    draw_btn("", "", "中止");
    FP_M.fpm_setAddMode(0x00);                  // 指紋データの重複を許す
    registerUser();
  }
  if (M5.BtnB.wasPressed()) {
    authenticateUser();
  }
  if (M5.BtnC.wasPressed()) {
    //currentScreens = SETTING;
  }
}

// はんだごて切り忘れ通知
void forgetTurnOffAlert() {
  draw_btn("", "", "");
  home.clear(BLACK);
  home.setCursor(0, 0);
  home.println("放置時間が一定時間");
  home.println("を超えたため");
  home.println("強制終了します");
  home.pushSprite(&lcd, 0, 0);
  String postData = "{\"user_id\": \"" + fuserID + "\", \"device_id\": \"" + String(solderingID) + "\", \"" + logID + "\"}";
  String response = FT_S.functions_post(String(functionsUrl), String(alertEndpoint), postData);

  if (response != "Internal Server Error") {
    //M5.Lcd.println("handa alert.");
  } else {
    M5.Lcd.println("Server Error.");
    //forgetTurnOffAlert();
  }
}

// はんだごて使用中画面
void solderingScreen() {
  home.setFont(&fonts::lgfxJapanGothic_24);
  double KOTEdepth = 0;
  double KOTEtemp = 0;
  int useTime = 0;
  int returnTime = 0;
  const int timeOut = 60;
  boolean finish = false;
  draw_btn("終了", "", "");

  while(true){
    returnTime = (int)((millis() - returnStartTime) / 1000 + 0.5);
    useTime = (int)((millis() - startTime) / 1000 + 0.5);
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.print("使用者　：");
    home.println(fuserName);
    home.print("温度　　：");
    home.println(temperature);
    home.print("納刀時間：");
    home.println(returnTime);
    home.print("使用時間：");
    home.println(useTime);
    home.pushSprite(&lcd, 0, 0);

    for(int i=0; i<1000; i++){
      M5.update();
      if(M5.BtnA.wasPressed()){
        home.clear(BLACK);
        home.setCursor(0, 0);
        home.println("KOTEの使用を");
        home.println("終了しますか？");
        home.pushSprite(&lcd, 0, 0);
        draw_btn("終了", "続行", "");

        while(true){
          M5.update();
          if(M5.BtnA.wasPressed()){
            home.clear(BLACK);
            home.setCursor(0, 0);
            home.println("KOTEの使用を");
            home.println("終了します");
            home.pushSprite(&lcd, 0, 0);
            draw_btn("", "", "");
            solderingFinish();
            return;
          }
          if(M5.BtnB.wasPressed()){
            home.clear(BLACK);
            home.setCursor(0, 0);
            home.println("KOTEを使用続行します");
            home.pushSprite(&lcd, 0, 0);
            draw_btn("終了", "", "");
            break;
          }
        }
      }
      delay(1);
    }

    if(returnTime >= timeOut){
      if(wifiConnect){
        forgetTurnOffAlert();
        solderingFinish();
        currentScreens = STANDBY;
        return;
      }
    }
  }
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
  draw_btn("登録", "使用", "設定");
  draw_home();
  while(M5.BtnA.isPressed() || M5.BtnB.isPressed() || M5.BtnC.isPressed()) M5.update();
  change = false;
  M5.update();
}