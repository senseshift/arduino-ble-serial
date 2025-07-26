#include <BLESerial.h>

// FOR ETL: Uncomment the following lines
// #include <Embedded_Template_Library.h>
// #include <etl/queue.h>
// #include <etl/circular_buffer.h>

BLESerial SerialBLE;
// If you are using older version of Arduino IDE, you may need to use
// empty template argument (<>), due to the old C++ compiler version (<=std=c++17).
// https://www.cppreference.com/w/cpp/language/ctad.html
//
// Uncomment the line below if you are using an older version of Arduino IDE/C++ compiler
// BLESerial<> SerialBLE;

// FOR ETL: Uncomment one of the following lines
// BLESerial<etl::queue<uint8_t, 255, etl::memory_model::MEMORY_MODEL_SMALL>> SerialBLE;
// OR
// BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

void setup() {
    BLEDevice::init("ESP32-BLE-Slave");

    BLEServer* pServer = BLEDevice::createServer();

    // Transparent UART Service
    // https://developerhelp.microchip.com/xwiki/bin/view/applications/ble/android-development-for-bm70rn4870/transparent-uart-service-for-bm70rn4870/
    auto pService = pServer->createService("49535343-FE7D-4AE5-8FA9-9FAFD205E455");

    auto pRxCharacteristic = pService->createCharacteristic("49535343-1E4D-4BD9-BA61-23C647249616", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
    auto pTxCharacteristic = pService->createCharacteristic("49535343-8841-43F4-A8D4-ECBE34729BB3", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    SerialBLE.begin(pRxCharacteristic, pTxCharacteristic);

    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

void loop() {
    if (Serial.available()) {
        SerialBLE.write(Serial.read());
        SerialBLE.flush();
    }
    if (SerialBLE.available()) {
        Serial.write(SerialBLE.read());
    }
}