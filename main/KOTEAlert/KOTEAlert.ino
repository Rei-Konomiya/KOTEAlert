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
#define THERMISTOR 35
#define TURN_OFF_TIME (1000 * 60 * 5)

enum Screens {STANDBY, SETTING, SOLDERING};
Screens currentScreens = STANDBY;

bool sensorDispFlg = false;   // センサー値を読み取るかどうか
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
M5Canvas tab(&lcd);
M5Canvas selecting(&lcd);
M5Canvas tabName(&lcd);
M5Canvas home(&lcd);
M5Canvas wifiImage(&lcd);

boolean change = false;
boolean wifiConnect = false;


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
* 選択されていないタブのスプライトを作るメソッド
*/
void draw_tab(){
  tab.clear(BLACK);
  tab.fillRoundRect (0, 0, 100, 100, 20, DARKCYAN);
  tab.fillRoundRect (2, 2, 96, 96, 18, BLACK);
}

/*
* タブ全体のスプライトを作る・表示するメソッド
*/
void draw_btn(String tabA, String tabB, String tabC){
  tabName.clear(BLACK);
  draw_tab();
  String btnName[] = {tabA, tabB, tabC};
  for(int i=0; i<3; i++){
    if(btnName[i] != ""){
      tab.pushSprite(&tabName, 105*i, 0);
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
  home.clear(BLACK);
  home.pushSprite(&lcd, 0, 0);
}

// はんだごてのセンサーを監視するタスク
void solderingSensorTask(void *parameter) {
  while (true) {
    if (sensorDispFlg) {
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.fillRect(0, 20, 350, 300, BLACK);
      M5.Lcd.setCursor(0, 20);
      M5.Lcd.setTextFont(2);
      M5.Lcd.print("temp:");
      M5.Lcd.println(SS_S.readTemperature());
    }
    delay(1000);
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

}

void setup() {
  auto cfg = M5.config();
  cfg.clear_display = true;
  M5.begin(cfg);

  lcd.begin();
  lcd.setBrightness(64);

  tab.setColorDepth(8);
  tab.createSprite(100, 60);
  selecting.setColorDepth(8);
  selecting.createSprite(100, 60);
  tabName.setColorDepth(8);
  tabName.createSprite(310, 60);
  home.setColorDepth(8);
  home.createSprite(lcd.width()-25, 180);
  wifiImage.setColorDepth(8);
  wifiImage.createSprite(25, 15);

  delay(100);

  Serial2.begin(19200, SERIAL_8N1, 3, 1);     // 3ピンをRX(受信), 1ピンをTX(送信)にする
  delay(1000);
  FP_M.fpm_setAddMode(0x00);                  // 指紋データの重複を許す

  SS_S.attach(TRIG, ECHO, THERMISTOR);

  while (!UM_S.SDEnable()) {
    delay(500);
    M5.Lcd.println("SD disable");
  }

  // センサータスクの作成
  xTaskCreate(
      solderingSensorTask,        /* タスク関数 */
      "solderingSensorTask",     /* タスク名 */
      10000,        /* スタックサイズ */
      NULL,         /* タスクのパラメータ */
      1,            /* このタスクの優先度 */
      NULL          /* タスクハンドル */
  );

  WiFiConnect();

  // WiFi接続確認のタスク作成
  xTaskCreate(WiFiConnectionTask, "WiFiConnectionTask", 10000, NULL, 2, NULL);
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

// 待機画面
void standbyScreen() {
  if (M5.BtnA.wasPressed()) {
    draw_btn("", "", "中止");
    registerUser();
  }
  if (M5.BtnB.wasPressed()) {
    //authenticateUser();
  }
  if (M5.BtnC.wasPressed()) {
    //currentScreens = SETTING;
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
    currentScreens = STANDBY;
  }
}

// はんだごて使用中画面
void solderingScreen() {
  if (M5.BtnA.wasPressed()) {
    //solderingFinish();
  }
  if (M5.BtnB.wasPressed()) {
    // 空き
  }
  if (M5.BtnC.wasPressed()) {
    // 空き
  }
}

// ユーザー登録
void registerUser(){
  home.setFont(&fonts::lgfxJapanGothic_24);
  
  bleConnecting(); //BLE接続
  if(!change)bleReceive();    //BLE情報受け取り
  if(!change)fingerPrint();   //指紋登録

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

// BLEデータ受け取り
void receive(){
  fuserID = "0ATMiNgfn71RkoSu5zeL";       //ここでuid受け取り
  fuserName = "黒田 直樹";    //ここでユーザー名受け取り
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
    home.println("動かさないようにしてください");
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
        delay(2000);
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