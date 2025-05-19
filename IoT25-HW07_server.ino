#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define RED_PIN 4
#define GREEN_PIN 15
#define BLUE_PIN 2
#define LED_BUILTIN 2

BLECharacteristic* pCharacteristic;

void setColor(const String& color) {
  if (color == "RED") {
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 255);
    analogWrite(BLUE_PIN, 255);
  } else if (color == "GREEN") {
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 255);
  } else if (color == "BLUE") {
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 255);
    analogWrite(BLUE_PIN, 0);
  } else {
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
  }
}

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
    String value = String((char*)pCharacteristic->getValue().c_str());
    Serial.print("📩 받은 데이터: ");
    Serial.println(value);

    setColor(value);
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("✅ 클라이언트 연결됨!");
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("❌ 클라이언트 연결 해제됨.");
    setColor("OFF");
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  setColor("OFF");

  BLEDevice::init("ESP32_LED_SERVER");
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService("12345678-1234-1234-1234-1234567890ab");
  pCharacteristic = pService->createCharacteristic(
    "abcd1234-5678-1234-5678-abcdef123456",
    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("12345678-1234-1234-1234-1234567890ab");
  pAdvertising->start();

  Serial.println("📡 BLE 서버 광고 시작됨!");
}

void loop() {
  // 이벤트 기반, 사용 안 함
}
