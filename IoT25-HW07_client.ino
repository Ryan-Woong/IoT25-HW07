#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

BLEAdvertisedDevice* myDevice = nullptr;
BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharacteristic;

bool doConnect = false;
bool connected = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    Serial.print("🔍 스캔 결과: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      Serial.println("✅ 서버 발견!");
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      BLEDevice::getScan()->stop();
    }
  }
};

float estimateDistance(int rssi) {
  int txPower = -69; // 기본값 (RSSI @1m)
  if (rssi == 0) return -1.0;
  float ratio = (txPower - rssi) / (10.0 * 2.0); // n=2 환경
  return pow(10.0, ratio);
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  Serial.println("🔎 서버 스캔 시작...");
  pBLEScan->start(10, false);
}

void loop() {
  if (doConnect && !connected) {
    Serial.println("🔗 서버 연결 시도 중...");
    pClient = BLEDevice::createClient();
    if (pClient->connect(myDevice)) {
      Serial.println("✅ 서버 연결 성공!");
      pRemoteCharacteristic = pClient->getService(SERVICE_UUID)->getCharacteristic(CHARACTERISTIC_UUID);
      connected = true;
    } else {
      Serial.println("❌ 서버 연결 실패");
    }
    doConnect = false;
  }

  if (connected) {
    int rssi = pClient->getRssi();
    float distance = estimateDistance(rssi);
    Serial.print("📶 RSSI: "); Serial.print(rssi);
    Serial.print(" → 📏 거리: "); Serial.print(distance); Serial.println(" m");

    String message;
    if (distance <= 1.0) message = "RED";
    else if (distance <= 2.0) message = "GREEN";
    else if (distance <= 3.0) message = "BLUE";
    else message = "OFF";

    if (pRemoteCharacteristic->canWrite()) {
      pRemoteCharacteristic->writeValue(message.c_str());
      Serial.print("📤 전송 메시지: ");
      Serial.println(message);
    }

    delay(2000);
  }
}
