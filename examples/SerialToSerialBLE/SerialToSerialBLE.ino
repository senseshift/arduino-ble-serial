#include <BLESerial.h>

// FOR ETL: Uncomment the following lines
// #include <Embedded_Template_Library.h>
// #include <etl/queue.h>
// #include <etl/circular_buffer.h>

String device_name = "ESP32-BLE-Slave";

BLESerial SerialBLE;
// If you are using an older version of Arduino IDE or C++ compiler, you may need to use
// an empty template argument (<>), as Class Template Argument Deduction (CTAD) is not
// supported in C++ versions older than C++17. For more details, see:
// https://www.cppreference.com/w/cpp/language/ctad.html
//
// Uncomment the line below if you are using an older version of Arduino IDE/C++ compiler
// BLESerial<> SerialBLE;

// FOR ETL: Uncomment one of the following lines
// BLESerial<etl::queue<uint8_t, 255, etl::memory_model::MEMORY_MODEL_SMALL>> SerialBLE;
// OR
// BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

void setup() {
    Serial.begin(9600);
    SerialBLE.begin(device_name);
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