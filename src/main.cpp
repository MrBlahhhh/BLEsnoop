#include <Arduino.h>
#include <NimBLEDevice.h>

#define SCAN_TIME_SECONDS 0 // 0 means scan continuously

void printHex(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        Serial.printf("%02X ", data[i]);
    }
}

class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        std::string name = advertisedDevice->getName();
        std::string address = advertisedDevice->getAddress().toString();
        
        bool isTpmsCandidate = false;
        
        // Filter by common TPMS names or look for manufacturer data payloads
        if (name.find("TPMS") != std::string::npos || name.find("SPI") != std::string::npos) {
            isTpmsCandidate = true;
        }
        
        std::string mfgData = advertisedDevice->getManufacturerData();
        if (mfgData.length() >= 8) {
            isTpmsCandidate = true;
        }

        if (isTpmsCandidate) {
            Serial.printf("\n--- Found Potential TPMS Device ---\n");
            Serial.printf("MAC Address: %s\n", address.c_str());
            Serial.printf("RSSI:        %d dBm\n", advertisedDevice->getRSSI());
            
            if (!name.empty()) {
                Serial.printf("Device Name: %s\n", name.c_str());
            }
            
            if (!mfgData.empty()) {
                Serial.print("Mfg Data (Hex): ");
                printHex((const uint8_t*)mfgData.data(), mfgData.length());
                Serial.println();
                Serial.printf("Mfg Data Length: %d bytes\n", mfgData.length());
            }
            
            const uint8_t* rawPayload = advertisedDevice->getPayload();
            size_t payloadLength = advertisedDevice->getPayloadLength();
            Serial.print("Raw Payload (Hex): ");
            printHex(rawPayload, payloadLength);
            Serial.println();
            Serial.printf("-----------------------------------\n");
        }
    }
};

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Starting ESP32-S3 BLE TPMS Sniffer...");

    NimBLEDevice::init("");
    
    NimBLEScan* pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
    
    pBLEScan->setActiveScan(false); 
    pBLEScan->setInterval(100); 
    pBLEScan->setWindow(99);    
    
    pBLEScan->start(SCAN_TIME_SECONDS, nullptr, false);
}

void loop() {
    delay(1000);
}