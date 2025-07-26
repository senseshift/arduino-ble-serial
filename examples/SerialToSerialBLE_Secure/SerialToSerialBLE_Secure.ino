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
// Uncomment the line below if you are using an older version of Arduino IDE
// BLESerial<> SerialBLE;

// FOR ETL: Uncomment one of the following lines
// BLESerial<etl::queue<uint8_t, 255, etl::memory_model::MEMORY_MODEL_SMALL>> SerialBLE;
// OR
// BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;

uint32_t passKey;

class AppSecurityCallbacks : public BLESecurityCallbacks {
  public:
    AppSecurityCallbacks() {
        this->passKey = random(111111, 999999);
    }

    uint32_t onPassKeyRequest(){
        ESP_LOGI(LOG_TAG, "PassKeyRequest");

        // Generate a random passkey
        this->passKey = random(111111, 999999);

        return this->passKey;
    }

    void onPassKeyNotify(uint32_t pass_key){
        ESP_LOGI(LOG_TAG, "The passkey Notify number: %d", pass_key);
    }

    bool onConfirmPIN(uint32_t pass_key){
        ESP_LOGI(LOG_TAG, "The passkey YES/NO number: %d", pass_key);
        vTaskDelay(5000);
        return true;
    }

    bool onSecurityRequest(){
        ESP_LOGI(LOG_TAG, "SecurityRequest");
        return true;
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl){
        ESP_LOGI(LOG_TAG, "Starting BLE work!");
    }

  private:
    uint32_t passKey;
};

void setup() {
    BLEDevice::init("ESP32-BLE-Slave");
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(new AppSecurityCallbacks());

    BLEServer* pServer = BLEDevice::createServer();

    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setKeySize();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_ONLY);
    pSecurity->setCapability(ESP_IO_CAP_IO);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

    Serial.begin(9600);
    SerialBLE.begin(pServer);
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