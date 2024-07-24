# Arduino Serial BLE

This library allows using Nordic UART Service (NUS) with ESP32 Arduino.

<b>Get involved: 💬 [Discord](https://discord.gg/YUtRKAqty2) • 🌐 [Website](https://senseshift.io) • 🐛 [Issues](https://github.com/senseshift/arduino-ble-serial/issues) • 📢 [Twitter](https://twitter.com/senseshiftio) • 💎 [Patreon](https://www.patreon.com/senseshift)</b>

[![Discord Widget](https://discord.com/api/guilds/966090258104062023/widget.png?style=banner2)](https://discord.gg/YUtRKAqty2)

[![PlatformIO Registry](https://badges.registry.platformio.org/packages/leon0399/library/Serial_BLE.svg)](https://registry.platformio.org/libraries/leon0399/Serial_BLE)
[![Arduino Library](https://www.ardu-badge.com/badge/Serial_BLE.svg?)](https://www.ardu-badge.com/Serial_BLE)
[![GitHub release](https://img.shields.io/github/v/release/senseshift/arduino-ble-serial)](https://github.com/senseshift/arduino-ble-serial/releases/latest)

[![PlatformIO CI](https://github.com/senseshift/arduino-ble-serial/actions/workflows/platformio-ci.yml/badge.svg)](https://github.com/senseshift/arduino-ble-serial/actions/workflows/platformio-ci.yml)
[![PlatformIO CI](https://github.com/senseshift/arduino-ble-serial/actions/workflows/arduino-ci.yml/badge.svg)](https://github.com/senseshift/arduino-ble-serial/actions/workflows/arduino-ci.yml)

[![MIT](https://img.shields.io/github/license/senseshift/arduino-ble-serial)](/LICENSE)
[![GitHub contributors](https://img.shields.io/github/contributors/senseshift/arduino-ble-serial)](https://github.com/senseshift/arduino-ble-serial/graphs/contributors)
[![GitHub](https://img.shields.io/github/stars/senseshift/arduino-ble-serial.svg)](https://github.com/senseshift/arduino-ble-serial)

## Features

- [x] [`HardwareSerial`](https://www.arduino.cc/reference/en/language/functions/communication/serial/)-compatible API
- [x] [ETL (Embedded Template Library)](https://github.com/ETLCPP/etl) support
- [x] NimBLE support through [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) library.
- [x] Custom Server and Characteristics configuration

## Installation

### PlatformIO

```diff
lib_deps =
+  leon0399/Serial_BLE
```

## Client-side usage

- Android:
  - [nRF connect for mobile](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)
  - [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal)
- iOS:
  - [nRF connect for mobile](https://apps.apple.com/es/app/nrf-connect-for-mobile/id1054362403)

## Usage

For all examples, take a look at the [`examples`](./examples) folder.

### Basic Example

```ino
#include <BLESerial.h>

BLESerial<> SerialBLE;

void setup() {
    Serial.begin(9600);
    SerialBLE.begin("ESP32-BLE-Slave");
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
```

### Custom UART characteristics

Using custom UUIDs for [Microchip BM70/RN4870 Transparent UART](https://developerhelp.microchip.com/xwiki/bin/view/applications/ble/android-development-for-bm70rn4870/transparent-uart-service-for-bm70rn4870/)

```ino
// ...

void setup() {
    // ...

    SerialBLE.begin(
      "ESP32-BLE-Slave",
      "49535343-FE7D-4AE5-8FA9-9FAFD205E455",
      "49535343-1E4D-4BD9-BA61-23C647249616",
      "49535343-8841-43F4-A8D4-ECBE34729BB3"
    );

    // ...
}

// ...
```

#### Alternative


```ino
// ...

void setup() {
    // ...

    BLEDevice::init("ESP32-BLE-Slave");
    BLEServer* pServer = BLEDevice::createServer();

    auto pService = pServer->createService("49535343-FE7D-4AE5-8FA9-9FAFD205E455");

    auto pRxCharacteristic = pService->createCharacteristic("49535343-1E4D-4BD9-BA61-23C647249616", BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_NOTIFY);
    auto pTxCharacteristic = pService->createCharacteristic("49535343-8841-43F4-A8D4-ECBE34729BB3", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

    SerialBLE.begin(pRxCharacteristic, pTxCharacteristic);

    // ...
}

// ...
```

### Custom Read Buffer

#### ETL (Embedded Template Library)

Using [ETL](https://github.com/ETLCPP/etl) provides a way to use this library without dynamic memory allocation.

```ino
#include <Embedded_Template_Library.h>
#include <etl/queue.h>
#include <etl/circular_buffer.h>

BLESerial<etl::queue<uint8_t, 255, etl::memory_model::MEMORY_MODEL_SMALL>> SerialBLE;
BLESerial<etl::circular_buffer<uint8_t, 255>> SerialBLE;
```

## NimBLE

Using the NimBLE library saves a significant amount of RAM and Flash memory.

<table>
    <tr>
        <td>
            ESP32 BLE
        </td>
        <td>
            <pre>RAM:   [=         ]  11.9% (used 39124 bytes from 327680 bytes)
Flash: [========= ]  85.9% (used 1125553 bytes from 1310720 bytes)</pre>
        </td>
    </tr>
    <tr>
        <td>
            NimBLE-Arduino
        </td>
        <td>
            <pre>RAM:   [=         ]   9.3% (used 30548 bytes from 327680 bytes)
Flash: [====      ]  44.2% (used 579158 bytes from 1310720 bytes)</pre>
        </td>
    </tr>
</table>

### Arduino IDE

Change the following line in `BLESerial.h`:

```diff
- #    define BLESERIAL_USE_NIMBLE false
+ #    define BLESERIAL_USE_NIMBLE true
```

### PlatformIO

Change your `platformio.ini` file to the following settings:

```diff
lib_deps = 
+  NimBLE-Arduino

build_flags = 
+  -D BLESERIAL_USE_NIMBLE=true
```
