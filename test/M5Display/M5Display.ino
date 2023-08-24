#include <M5Stack.h>    // Core2のヘッダーを準備
#include <M5GFX.h>      // M5GFXライブラリのインクルード
M5GFX lcd;              // 直接表示のインスタンスを作成（M5GFXクラスを使ってlcdコマンドでいろいろできるようにする）
M5Canvas tab(&lcd);
M5Canvas selecting(&lcd);
M5Canvas tabName(&lcd);
M5Canvas home(&lcd);
M5Canvas wifiImage(&lcd);

//320 * 240
int selectBtn = -1;
boolean change = false;
boolean wifiConnect = true;

String names[200];
String uids[200];
int datas[200];
int registered = 0;

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

void draw_checkBtn(){
  tabName.clear(BLACK);
  draw_tab();
  String btnName[] = {"完了", "戻る", "中止"};

  for(int i=0; i<3; i++){
    tab.pushSprite(&tabName, 105*i, 0);
    tabName.setTextColor(WHITE);
    
    tabName.setFont(&fonts::lgfxJapanGothic_36);
    tabName.setCursor(105*i+14, 13);
    tabName.print(btnName[i]);
  }
  tabName.pushSprite(&lcd, 5, 60*3);
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


/*
* BLE接続待機メソッド
*/
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

/*
* BLE受け取り
*/
void receive(){
  String uid = "ecc2023";       //ここでuid受け取り
  String name = "えしし太郎";    //ここでユーザー名受け取り

  uids[registered] = uid;
  names[registered] = name;
}

/*
* BLE受け取り待機
*/
void bleReceive(){
  boolean received = false;  //デバッグ用、実装時は置換する
  int periodCount = 0;
  int timeCount = 0;
  const int timeout = 10;

  while(!received){
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("ユーザー情報を");
    home.print("登録してください");
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


/*
* 指紋登録メソッド
*/
void fingerPrint(){
  boolean inited = false;  //デバッグ用、実装時は削除する
  boolean initFail = false;  //デバッグ用、実装時は削除する
  int periodCount = 0;
  int timeCount = 0;
  const int timeout = 10;

  int data = 1234567890;
  while(!inited){
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("指紋を登録します");
    home.println("センサーに指を");
    home.print("あててください");
    for(int i=0; i<periodCount; i++) home.print(".");
    ++periodCount %= 4;
    home.pushSprite(&lcd, 0, 0);
    wait(1000);
    timeCount++;
    if(timeCount >= timeout){
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
  datas[registered] = data;

  home.clear(BLACK);
  home.setCursor(0, 0);
  home.println("指紋情報　登録完了");
  home.pushSprite(&lcd, 0, 0);
  registered++;
  delay(2000);
}

/*
* 指紋登録全体メソッド
*/
void entry(){
  home.setFont(&fonts::lgfxJapanGothic_24);
  
  bleConnecting(); //BLE接続
  if(!change)bleReceive();    //BLE情報受け取り
  if(!change)fingerPrint();   //指紋登録

}



/*
* 指紋認証メソッド
*/
int fingerCheck(){
  home.setFont(&fonts::lgfxJapanGothic_24);
  boolean checked = false;  //デバッグ用、実装時は置換する
  boolean failed = false;    //デバッグ用、実装時は置換する
  int periodCount = 0;
  int timeCount = 0;
  const int timeout = 10;

  while(!checked){
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("指紋認証を行います");
    home.println("センサーに指を");
    home.print("あててください");
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
    if(failed){
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("認証に失敗しました");
      home.println("ホーム画面に戻ります");
      home.pushSprite(&lcd, 0, 0);
      change = true;
      delay(2000);
    }
    if(change) return 0;
    
    //以下5行はデバッグ用、実装時は削除する
    M5.update();
    if(M5.BtnA.isPressed()){
      if(timeCount > 5 || registered == 0) failed = true;
      else checked = true;
    }
  }
  home.clear(BLACK);
  home.setCursor(0, 0);
  home.println("ユーザー情報　受取完了");
  home.pushSprite(&lcd, 0, 0);
  wait(2000);
  if(change) return 0;
}

/*
* はんだごて使用時画面メソッド
*/
boolean useSolder(int userNum){
  home.setFont(&fonts::lgfxJapanGothic_24);
  String username = names[userNum];
  double KOTEdepth = 0;
  double KOTEtemp = 0;
  int leaveTime = 0;
  int useTime = 0;
  const int timeOut = 60 * 10;
  boolean finish = false;
  draw_btn("終了", "", "");

  while(true){
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.print("使用者　：");
    home.println(username);
    home.print("温度　　：");
    home.println(KOTEtemp);
    home.print("納刀時間：");
    home.println(leaveTime);
    home.print("使用時間：");
    home.println(useTime);
    home.pushSprite(&lcd, 0, 0);

    useTime++;
    if(KOTEdepth < 5 && KOTEtemp > 300) leaveTime++;
    else leaveTime = 0;

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
            home.println("終了しました");
            home.pushSprite(&lcd, 0, 0);
            delay(1000);
            return false;
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

    if(leaveTime >= timeOut){
      if(wifiConnect){
        //firebase通知
        home.clear(BLACK);
        home.setCursor(0, 0);
        home.println("放置時間が一定時間");
        home.println("を超えたため");
        home.println("強制終了します");
        home.pushSprite(&lcd, 0, 0);
        return true;
      }
    }
  }

}

/*
* はんだごて使用メソッド
*/
void use(){
  home.setFont(&fonts::lgfxJapanGothic_24);
  int userNum = fingerCheck();
  boolean lock = false;
  int reservation = 1;  //デバッグ用
  
  if(!change){
    home.clear(BLACK);
    home.setCursor(0, 0);
    home.println("KOTEのロックが");
    home.println("解除されました");
    home.pushSprite(&lcd, 0, 0);
    wait(1000);
  }
  while(!lock && !change){
    if(!change) lock = useSolder(userNum);
    if(!lock && reservation > 0){
      home.clear(BLACK);
      home.setCursor(0, 0);
      home.println("次の予約者に");
      home.println("移りますか？");
      home.pushSprite(&lcd, 0, 0);
      draw_btn("譲渡", "終了", "");
      while(true){
        M5.update();
        if(M5.BtnA.wasPressed()){
          home.clear(BLACK);
          home.setCursor(0, 0);
          home.println("次の予約者に");
          home.println("譲渡しました");
          home.pushSprite(&lcd, 0, 0);
          reservation -= 1;
          delay(1000);
          break;
        }
        if(M5.BtnB.wasPressed()){
          home.clear(BLACK);
          home.setCursor(0, 0);
          home.println("KOTEをロックしました");
          home.pushSprite(&lcd, 0, 0);
          lock = true;
          delay(1000);
          break;
        }
      }
    }else if(reservation == 0) lock = true;
  }
}

/*
* 各種設定メソッド
*/
void config(){
  home.clear(BLACK);
  home.setFont(&fonts::lgfxJapanGothic_24);
  home.setCursor(0, 0);
  home.println("設定");
  home.println(registered);
  home.pushSprite(&lcd, 0, 0);

  while(true){
    wait(1000);
    if(change) break;
  }
}



void setup(){
  M5.begin();
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

  draw_btn("登録", "使用", "設定");
  draw_wifi();
}

void loop(){

  M5.update();
  if(M5.BtnA.isPressed() && !change){
    selectBtn = 0;
    draw_btn("", "", "中止");
    entry();
  }else if(M5.BtnB.isPressed() && !change){
    selectBtn = 1;
    draw_btn("", "", "中止");
    use();
  }else if(M5.BtnC.isPressed() && !change){
    selectBtn = 2;
    wifiConnect = (!wifiConnect);
    draw_btn("", "", "中止");
    draw_wifi();
    config();
  }else{
    selectBtn = -1;
    draw_btn("登録", "使用", "設定");
    draw_home();
    while(M5.BtnA.isPressed() || M5.BtnB.isPressed() || M5.BtnC.isPressed()) M5.update();
    change = false;
  }
}