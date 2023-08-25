#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <M5Stack.h>

String inVolume = "100";
String inTiming = "mix";


//以下BLE接続
//#define DEVICE_NAME "BLE-ESP32"
#define DEVICE_NAME "travel_mate"
#define SERVICE_UUID "7c68efdd-a727-4b51-ba18-a3519164875c"  // サービスのUUID

// Bluetoothのパケットサイズ
#define MTU_SIZE 200

BLECharacteristic *timeInfoCharacteristic;

bool deviceConnected = false;
bool connectingWifi = false;



// Bluetoothの接続状態の結果がこのクラスのメソッドが呼び出されることによって返ってくる(Observerパターン)
class ConnectionCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    pServer->updatePeerMTU(pServer->getConnId(), 200);
    Serial.println("接続された");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("接続解除された");
  }
};
void startBluetooth() {
  BLEDevice::init(DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new ConnectionCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  doPrepare(pService);
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
}

class timeInfoBleCallback : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pC) {
  }

  // スマホから通知情報が書き込まれた場合ここが呼び出される
  void onWrite(BLECharacteristic *pC) {
    std::string value = pC->getValue();
    inTiming = value.c_str();
    Serial.println(inTiming);
  }
};

// スマホから通知の情報を送信するためのキャラスタリスティックを識別するためのUUID
#define TIME_INFO_CHARACTERISTIC_UUID "83b35c6d-56ce-4e0b-ad0e-3ced0d1cee75"


void doPrepare(BLEService *pService) {
  // 通知情報をスマホから書き込むためのキャラクタリスティックを作成する
  timeInfoCharacteristic = pService->createCharacteristic(
    TIME_INFO_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);


  // 部屋
  timeInfoCharacteristic->setCallbacks(new timeInfoBleCallback());
}


void setup() {
  // put your setup code here, to run once:
  M5.begin();
  Serial.begin(115200);
  startBluetooth();
}

void loop() {
}