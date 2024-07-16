#include <BLESerial.h>

String device_name = "ESP32-BLE-Slave";

BLESerial<> SerialBLE;

void setup() {
    Serial.begin(9600);
    SerialBLE.begin(device_name);
}

void loop() {
    if (Serial.available()) {
        SerialBLE.write(Serial.read());
    }
    if (SerialBLE.available()) {
        Serial.write(SerialBLE.read());
    }
}