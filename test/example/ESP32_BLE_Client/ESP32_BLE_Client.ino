#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

static BLEUUID demo_serviceUUID("7c68efdd-a727-4b51-ba18-a3519164875c");
static BLEUUID demo1_charUUID("83b35c6d-56ce-4e0b-ad0e-3ced0d1cee75");

static BLEClient* pClient = nullptr;                      //Server or Client のクライアント側
static BLEAdvertisedDevice* connDevice;                   //接続するデバイスサーバー
static BLERemoteService* pRemoteService;                  //サービスを操作するインスタンス
static BLERemoteCharacteristic* pRemoteCharacteristic;    //キャラリスを操作するインスタンス

//自身の通信状態を監視するコールバック
class MyClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {        //デバイスと接続したら呼び出す
    Serial.println("Client Connected!");
    std::string val = "";
    val = "0ATMiNgfn71RkoSu5zeL:黒田 直樹";
    pRemoteService = pClient->getService(demo_serviceUUID);
    pRemoteCharacteristic = pRemoteService->getCharacteristic(demo1_charUUID);
    pRemoteCharacteristic->writeValue(val);
  }

  void onDisconnect(BLEClient* pclient) {     //デバイスと切断したら呼び出す
    Serial.println("Client Disconnected!");
    delete pClient;
    pClient = nullptr;
  }
};

//アドバタイズデバイスを監視するコールバック
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {   //アドバタイズデバイスを見つけたら呼び出す
    if (advertisedDevice.getName() == "travel_mate") {    //デバイス名でソートする
      Serial.println("travel_mate founded!");
      BLEDevice::getScan()->stop();
      delay(5000);  // 1秒遅延
      Serial.println("through1");
      //BLE接続の初期設定
      pClient  = BLEDevice::createClient();
      pClient->setClientCallbacks(new MyClientCallbacks());
      delay(5000);  // 1秒遅延
      Serial.println("through2");
      //BLEサーバーに接続する
      pClient->connect(&advertisedDevice);
    }
  }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("travel_band");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);

  pBLEScan->start(30, false);
}

void loop() {
  // put your main code here, to run repeatedly:

}
