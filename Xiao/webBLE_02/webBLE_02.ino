/*
 * ESP32C3 BLE Server using NimBLE
 * Provides BLE access for web browser connection
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

// BLE UART Service UUIDs 
#define SERVICE_UUID        "13176d7e-f725-4071-a054-18e67347d6f6"
#define CHARACTERISTIC_UUID_RX "13176d7e-f725-4071-a054-18e67347d6f7"
#define CHARACTERISTIC_UUID_TX "13176d7e-f725-4071-a054-18e67347d6f8"

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pTxCharacteristic = nullptr;
NimBLECharacteristic* pRxCharacteristic = nullptr;
bool deviceConnected = false;

// Callback for when device connects/disconnects
class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Device connected");
    }

    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Device disconnected");
        // Restart advertising when device disconnects
        delay(500);
        NimBLEDevice::startAdvertising();
        Serial.println("Advertising restarted");
    }
};

// Callback for when data is received via BLE
class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        Serial.println("=== onWrite callback triggered ===");
        std::string rxValue = pCharacteristic->getValue();
        
        Serial.print("Value length: ");
        Serial.println(rxValue.length());
        Serial.print("Characteristic UUID: ");
        Serial.println(pCharacteristic->getUUID().toString().c_str());

        if (rxValue.length() > 0) {
            Serial.print("Received via BLE: ");
            for (size_t i = 0; i < rxValue.length(); i++) {
                Serial.print((char)rxValue[i]);
            }
            Serial.println();
        } else {
            Serial.println("Received empty message");
        }
        Serial.println("=== End callback ===");
    }
    
    void onRead(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Characteristic read");
    }
};

void setup() {
    // USB Serial for debugging
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Initializing BLE...");
    
    // Initialize NimBLE
    NimBLEDevice::init("unsortedEvent");
    
    // Set power level for better range/stability
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    
    // Set MTU size for larger data transfers
    NimBLEDevice::setMTU(512);
    
    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create BLE Service
    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    // Create TX Characteristic (for sending data to client)
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::NOTIFY
    );

    // Create RX Characteristic (for receiving data from client)
    // Try with just WRITE first to test
    pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::READ
    );

    Serial.println("Setting up RX characteristic callbacks...");
    pRxCharacteristic->setCallbacks(new CharacteristicCallbacks());
    Serial.println("RX characteristic callbacks set");
    Serial.print("RX Characteristic UUID: ");
    Serial.println(pRxCharacteristic->getUUID().toString().c_str());

    // Start the service
    pService->start();

    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    
    // Clear any previous advertising data
    pAdvertising->reset();
    
    // Add service UUID to advertising (required for Web Bluetooth filter)
    pAdvertising->addServiceUUID(SERVICE_UUID);
    
    // Set advertising parameters - slower intervals for better compatibility
    pAdvertising->setMinInterval(100);   // 62.5ms (in units of 0.625ms)
    pAdvertising->setMaxInterval(200);   // 125ms (in units of 0.625ms)
    
    // Start advertising
    pAdvertising->start();
    
    Serial.println("BLE device is ready!");
    Serial.println("Device name: unsortedEvent");
    Serial.println("Advertising started - device should be discoverable now");
    Serial.println("Note: BLE devices don't show in regular Bluetooth settings");
    Serial.println("Use Web Bluetooth in Chrome/Edge or a BLE scanner app");
}

void loop() {
    // Check if callback missed any data (backup check)
    if (pRxCharacteristic != nullptr && deviceConnected) {
        std::string value = pRxCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.print("Loop check - Found data: ");
            for (size_t i = 0; i < value.length(); i++) {
                Serial.print((char)value[i]);
            }
            Serial.println();
            // Clear after reading
            pRxCharacteristic->setValue("");
        }
    }
    delay(100);
}
