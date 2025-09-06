#ifndef BLESERIAL_H
#define BLESERIAL_H

#include <Arduino.h>

#ifndef BLESERIAL_USE_NIMBLE
// Global flag to use NimBLE in all SenseShift libraries
#  ifdef SS_USE_NIMBLE
#    define BLESERIAL_USE_NIMBLE SS_USE_NIMBLE
#  else // SS_USE_NIMBLE
#    define BLESERIAL_USE_NIMBLE false
#  endif // SS_USE_NIMBLE
#endif // BLESERIAL_USE_NIMBLE

#if defined(BLESERIAL_USE_NIMBLE) && BLESERIAL_USE_NIMBLE
#  include <NimBLEDevice.h>
#else // BLESERIAL_USE_NIMBLE
#  include <BLE2902.h>
#  include <BLEDevice.h>
#endif // BLESERIAL_USE_NIMBLE

#if !defined(BLESERIAL_NIMBLE_VERSION_MAJOR) && defined(BLESERIAL_USE_NIMBLE) && BLESERIAL_USE_NIMBLE
#  if __has_include(<NimBLEAdvertisementData.h>)
#    define BLESERIAL_NIMBLE_VERSION_MAJOR 2
// #    warning "Using NimBLE version 2"
#  else // __has_include(<NimBLEAdvertisementData.h>)
#    define BLESERIAL_NIMBLE_VERSION_MAJOR 1
// #    warning "Using NimBLE version 1"
#  endif // __has_include(<NimBLEAdvertisementData.h>)
#endif // BLESERIAL_USE_NIMBLE

#ifndef BLESERIAL_USE_STL
#  define BLESERIAL_USE_STL true
#endif // BLESERIAL_USE_STL

#if defined(BLESERIAL_USE_STL) && BLESERIAL_USE_STL
#  include <queue>
#endif // BLESERIAL_USE_STL

template<typename T>
class BLESerialCharacteristicCallbacks;

template<typename T>
class BLESerialServerCallbacks;

/**
 * @tparam T Type of the receive buffer
 */
#if defined(BLESERIAL_USE_STL) && BLESERIAL_USE_STL
template<typename T = std::queue<uint8_t>>
#else  // BLESERIAL_USE_STL
template<typename T>
#endif // BLESERIAL_USE_STL
class BLESerial : public Stream {
    friend class BLESerialCharacteristicCallbacks<T>;
    friend class BLESerialServerCallbacks<T>;

  private:
    T m_receiveBuffer;

    /**
     * BLE server instance
     * @note This is only used if the BLESerial instance is managing the BLE server
     */
    BLEServer* m_pServer = nullptr;

    /**
     * BLE service instance
     * @note This is only used if the BLESerial instance is managing the BLE service
     */
    BLEService* m_pService = nullptr;

    /**
     * BLE characteristic instance for receiving data
     */
    BLECharacteristic* m_pRxCharacteristic = nullptr;

    /**
     * BLE characteristic instance for transmitting data
     */
    BLECharacteristic* m_pTxCharacteristic = nullptr;

  public:
    static const char* SERVICE_UUID;
    static const char* RX_UUID;
    static const char* TX_UUID;

    BLESerial() : m_receiveBuffer() {}

    BLESerial(BLESerial const& other) = delete;      // disable copy constructor
    void operator=(BLESerial const& other) = delete; // disable assign constructor

    inline int available() override { return m_receiveBuffer.size(); }

    inline int peek() override { return m_receiveBuffer.front(); }

    int read() override
    {
        auto front = m_receiveBuffer.front();
        m_receiveBuffer.pop();
        return front;
    }

    size_t write(const uint8_t* buffer, size_t bufferSize) override
    {
        if (this->m_pTxCharacteristic == nullptr || !this->connected()) {
            return 0;
        }

        this->m_pTxCharacteristic->setValue(const_cast<uint8_t*>(buffer), bufferSize);
        // this->flush();

        return bufferSize;
    }

    size_t write(uint8_t byte) override
    {
        if (this->m_pTxCharacteristic == nullptr || !this->connected()) {
            return 0;
        }

        this->m_pTxCharacteristic->setValue(&byte, 1);
        // this->flush();

        return 1;
    }

    void flush(void) override { this->m_pTxCharacteristic->notify(true); }

    void begin(
      const String& deviceName = String(),
      const char* serviceUuid = SERVICE_UUID,
      const char* rxUuid = RX_UUID,
      const char* txUuid = TX_UUID
    )
    {
        this->begin(deviceName.c_str(), serviceUuid, rxUuid, txUuid);
    }

    /**
     * Begin BLE serial. This will create and start BLE server, service and characteristics.
     *
     * @note This will manage the BLE server, service and characteristics. If you want to manage them yourself, use the
     * other begin().
     *
     * @param deviceName Name of the BLE device
     * @param serviceUuid UUID of the BLE service
     * @param rxUuid UUID of the BLE characteristic for receiving data
     * @param txUuid UUID of the BLE characteristic for transmitting data
     */
    void begin(
      const char* deviceName,
      const char* serviceUuid = SERVICE_UUID,
      const char* rxUuid = RX_UUID,
      const char* txUuid = TX_UUID
    );

    /**
     * Begin BLE serial. This will create and start BLE service and characteristics.
     *
     * @note This will manage the BLE service and characteristics. If you want to manage them yourself, use the other
     * begin().
     *
     * @param pServer BLE server instance
     * @param serviceUuid UUID of the BLE service
     * @param rxUuid UUID of the BLE characteristic for receiving data
     * @param txUuid UUID of the BLE characteristic for transmitting data
     */
    void begin(
      BLEServer* pServer,
      const char* serviceUuid = SERVICE_UUID,
      const char* rxUuid = RX_UUID,
      const char* txUuid = TX_UUID
    )
    {
        BLEService* pService = pServer->getServiceByUUID(serviceUuid);
        if (pService == nullptr) {
            log_d("Creating BLE service with UUID '%s'", serviceUuid);
            pService = pServer->createService(serviceUuid);
        } else {
            log_w("BLE service with UUID '%s' already exists", serviceUuid);
        }

        // Store the service, so we know if we're managing it
        this->m_pService = pService;

        this->begin(pService, rxUuid, txUuid);

        pService->start();
        log_d("Started BLE service");
    }

    /**
     * Begin BLE serial. This will create and start BLE characteristics.
     *
     * @note If you want to create characteristics yourself, use the other begin().
     *
     * @param pService BLE service instance
     * @param rxUuid UUID of the BLE characteristic for receiving data
     * @param txUuid UUID of the BLE characteristic for transmitting data
     */
    void begin(BLEService* pService, const char* rxUuid = RX_UUID, const char* txUuid = TX_UUID)
    {
        auto* pRxCharacteristic = pService->getCharacteristic(rxUuid);
        if (pRxCharacteristic == nullptr) {
            log_d("Creating BLE characteristic with UUIDs '%s' (RX)", rxUuid);
#if defined(BLESERIAL_USE_NIMBLE) && BLESERIAL_USE_NIMBLE
            pRxCharacteristic =
              pService->createCharacteristic(rxUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
#else  // BLESERIAL_USE_NIMBLE
            pRxCharacteristic = pService->createCharacteristic(rxUuid, BLECharacteristic::PROPERTY_WRITE_NR);
#endif // BLESERIAL_USE_NIMBLE
        } else {
            log_w("BLE characteristic with UUID '%s' (RX) already exists", rxUuid);
        }

        auto* pTxCharacteristic = pService->getCharacteristic(txUuid);
        if (pTxCharacteristic == nullptr) {
            log_d("Creating BLE characteristic with UUIDs '%s' (TX)", txUuid);
#if defined(BLESERIAL_USE_NIMBLE) && BLESERIAL_USE_NIMBLE
            pTxCharacteristic = pService->createCharacteristic(txUuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
#else  // BLESERIAL_USE_NIMBLE
            pTxCharacteristic = pService->createCharacteristic(
              txUuid,
              BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
            );
#endif // BLESERIAL_USE_NIMBLE
        } else {
            log_w("BLE characteristic with UUID '%s' (TX) already exists", txUuid);
        }

        this->begin(pRxCharacteristic, pTxCharacteristic);
    }

    /**
     * Begin BLE serial. This will setup the BLE characteristics.
     *
     * @param pServer BLE server instance
     * @param pRxCharacteristic BLE characteristic instance for receiving data
     * @param pTxCharacteristic BLE characteristic instance for transmitting data
     */
    void begin(BLECharacteristic* pRxCharacteristic, BLECharacteristic* pTxCharacteristic);

#if !defined(BLESERIAL_USE_NIMBLE) && !BLESERIAL_USE_NIMBLE
    void end()
    {
        if (this->m_pService != nullptr) {
            this->m_pService->stop();
        }

        if (this->m_pServer != nullptr) {
            this->m_pServer->getAdvertising()->stop();
        }

        this->m_pServer = nullptr;
    }
#endif // BLESERIAL_USE_NIMBLE

    auto connected() -> bool { return m_pServer != nullptr && m_pServer->getConnectedCount() > 0; }

    auto getRxCharacteristic() -> BLECharacteristic* { return m_pRxCharacteristic; }

    auto getTxCharacteristic() -> BLECharacteristic* { return m_pTxCharacteristic; }
};

template<typename T>
class BLESerialServerCallbacks : public BLEServerCallbacks {
  public:
    explicit BLESerialServerCallbacks(BLESerial<T>* bleSerial) : bleSerial(bleSerial) {}

#if defined(BLESERIAL_USE_NIMBLE) && BLESERIAL_USE_NIMBLE
#  if defined(BLESERIAL_NIMBLE_VERSION_MAJOR) && BLESERIAL_NIMBLE_VERSION_MAJOR >= 2
// #   warning "Using NimBLE version 2 for BLESerialServerCallbacks"
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override
#  else // BLESERIAL_NIMBLE_VERSION_MAJOR >= 2
// #   warning "Using NimBLE version 1 for BLESerialServerCallbacks"
    void onDisconnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override
#  endif // BLESERIAL_NIMBLE_VERSION_MAJOR >= 2
#else // BLESERIAL_USE_NIMBLE
    void onDisconnect(BLEServer* pServer) override
#endif // BLESERIAL_USE_NIMBLE
    {
        auto* pAdvertising = pServer->getAdvertising();
        if (pAdvertising == nullptr) {
            return;
        }
        pAdvertising->start();
    }

  private:
    BLESerial<T>* bleSerial;
};

template<typename T>
class BLESerialCharacteristicCallbacks : public BLECharacteristicCallbacks {
  public:
    explicit BLESerialCharacteristicCallbacks(BLESerial<T>* bleSerial) : bleSerial(bleSerial) {}

#if defined(BLESERIAL_USE_NIMBLE) && BLESERIAL_USE_NIMBLE
#  if defined(BLESERIAL_NIMBLE_VERSION_MAJOR) && BLESERIAL_NIMBLE_VERSION_MAJOR >= 2
// #   warning "Using NimBLE version 2 for BLESerialCharacteristicCallbacks"
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override
#  else // BLESERIAL_NIMBLE_VERSION_MAJOR >= 2
// #   warning "Using NimBLE version 1 for BLESerialCharacteristicCallbacks"
    void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override
#  endif // BLESERIAL_NIMBLE_VERSION_MAJOR >= 2
#else // BLESERIAL_USE_NIMBLE
    void onWrite(BLECharacteristic* pCharacteristic) override
#endif // BLESERIAL_USE_NIMBLE
    {
        if (pCharacteristic != bleSerial->m_pRxCharacteristic) {
            return;
        }

        auto rxValue = pCharacteristic->getValue();
        for (int i = 0; i < rxValue.length(); i++) {
            bleSerial->m_receiveBuffer.push(rxValue[i]);
        }
    }

  private:
    BLESerial<T>* bleSerial;
};

template<typename T>
const char* BLESerial<T>::SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";

template<typename T>
const char* BLESerial<T>::RX_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";

template<typename T>
const char* BLESerial<T>::TX_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

template<typename T>
void BLESerial<T>::begin(const char* deviceName, const char* serviceUuid, const char* rxUuid, const char* txUuid)
{
    // Create the BLE Device
    log_d("Initializing BLE device with name '%s'", deviceName);
    BLEDevice::init(deviceName);

    log_d("Creating BLE server");
    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new BLESerialServerCallbacks<T>(this));

    // Store the server, so we know if we're managing it
    this->m_pServer = pServer;

    this->begin(pServer, serviceUuid, rxUuid, txUuid);

    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    log_d("Started BLE advertising");
}

template<typename T>
void BLESerial<T>::begin(BLECharacteristic* pRxCharacteristic, BLECharacteristic* pTxCharacteristic)
{
    // Store the characteristics, so we know if we're managing them
    this->m_pRxCharacteristic = pRxCharacteristic;
    this->m_pTxCharacteristic = pTxCharacteristic;

    this->m_pRxCharacteristic->setCallbacks(new BLESerialCharacteristicCallbacks<T>(this));

#if !defined(BLESERIAL_USE_NIMBLE) || !BLESERIAL_USE_NIMBLE
    // this->m_pRxCharacteristic->setAccessPermissions(ESP_GATT_PERM_WRITE_ENCRYPTED);
    this->m_pRxCharacteristic->addDescriptor(new BLE2902());
    this->m_pRxCharacteristic->setWriteProperty(true);
#endif

#if !defined(BLESERIAL_USE_NIMBLE) || !BLESERIAL_USE_NIMBLE
    // this->m_pTxCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED);
    this->m_pTxCharacteristic->addDescriptor(new BLE2902());
    this->m_pTxCharacteristic->setReadProperty(true);
    this->m_pTxCharacteristic->setNotifyProperty(true);
#endif
}

#endif // BLESERIAL_H