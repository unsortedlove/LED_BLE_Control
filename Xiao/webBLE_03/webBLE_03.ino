#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// UART link from XIAO -> Teensy
// Update pins if your XIAO wiring uses different GPIOs.
static const uint32_t UART_BAUD = 115200;
static const int UART_TX_PIN = 21;
static const int UART_RX_PIN = 20;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
//
#define SERVICE_UUID        "54a145a2-5493-483a-8b10-be70e00e3c93"
#define CHARACTERISTIC_UUID "54a145a3-5493-483a-8b10-be70e00e3c93"

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println(">>> Client connecting...");
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    Serial.println(">>> Client disconnected");
    deviceConnected = false;
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.print("Characteristic event, written: ");
      Serial.println(value); // Print the full string value

      // Forward one line per BLE message to Teensy.
      Serial1.println(value);
      Serial.print("Forwarded to Teensy: ");
      Serial.println(value);
    }
  }
};

void setup() {
  Serial.begin(UART_BAUD);
  Serial1.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  Serial.println("XIAO BLE -> UART bridge started");

  // Create the BLE Device
  BLEDevice::init("unsortedLED");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic for receiving data
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ   |
                                         BLECharacteristic::PROPERTY_WRITE  |
                                         BLECharacteristic::PROPERTY_WRITE_NR
                                       );

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  // notify changed value
  if (deviceConnected) {
    Serial.print("New value notified: ");
    Serial.println(value);
    delay(3000); // keep existing behavior
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); 
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");
  }
}