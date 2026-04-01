Import("env")
from pathlib import Path
import shutil

PATCHED_CPP = r'''#if defined(ESP32)
#include "esp32BLEUtilities.h"
#include <string>

bool BLE_status = 0;
bool isDeviceConnected = false;
bool prevDeviceConnected = false;
uint8_t tx_Value = 0;
uint8_t rxdatalength = 0;
uint8_t bytesremaining = 0;
uint8_t* rxdataBuffer = NULL;
bool newDataReceived = 0;
BLEServer *bleServer = NULL;
BLECharacteristic *bleTxCharacteristic;

class BleServerCallbacks : public BLEServerCallbacks {
 public:
  void onConnect(BLEServer* bleServer) override {
    isDeviceConnected = true;
  }

  void onDisconnect(BLEServer* bleServer) override {
    isDeviceConnected = false;
  }
};

class BleCallbacks : public BLECharacteristicCallbacks {
 public:
  void onWrite(BLECharacteristic *bleCharacteristic) override {
    std::string rxStd = bleCharacteristic->getValue();
    String rx_Value(rxStd.c_str());

    if (newDataReceived == 1 && rxdataBuffer != NULL) {
      delete[] rxdataBuffer;
      rxdataBuffer = NULL;
    }

    newDataReceived = 1;

    if (rx_Value.length() > 0) {
      rxdataBuffer = new uint8_t[rx_Value.length()];
      for (int i = 0; i < rx_Value.length(); i++) {
        rxdataBuffer[i] = static_cast<uint8_t>(rx_Value[i]);
#ifdef DEBUG
        Serial.print(rxdataBuffer[i]);
        Serial.print(" ");
#endif
      }
#ifdef DEBUG
      Serial.println();
#endif
      rxdatalength = rx_Value.length();
      bytesremaining = rx_Value.length();
    } else {
      rxdatalength = 0;
      bytesremaining = 0;
    }
  }
};

void Esp32ble::begin(String a) {
  BLEDevice::init(std::string(a.c_str()));

  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new BleServerCallbacks());

  BLEService *bleService = bleServer->createService(UUID_Service);

  bleTxCharacteristic = bleService->createCharacteristic(
    UUID_Transmit,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  bleTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *bleRxCharacteristic = bleService->createCharacteristic(
    UUID_Receive,
    BLECharacteristic::PROPERTY_WRITE
  );
  bleRxCharacteristic->setCallbacks(new BleCallbacks());

  bleService->start();
  bleServer->getAdvertising()->start();
#ifdef DEBUG
  Serial.println("Waiting for a client connection...");
#endif
}

void Esp32ble::write(uint8_t a) {
  if (isDeviceConnected) {
    bleTxCharacteristic->setValue(&a, 1);
    bleTxCharacteristic->notify();
    delay(10);
  }
  if (!isDeviceConnected && prevDeviceConnected) {
    delay(500);
    bleServer->startAdvertising();
#ifdef DEBUG
    Serial.println("Start advertising");
#endif
    prevDeviceConnected = isDeviceConnected;
  }
}

void Esp32ble::write(String x) {
  if (isDeviceConnected) {
    bleTxCharacteristic->setValue(std::string(x.c_str()));
    bleTxCharacteristic->notify();
    delay(10);
  }
  if (!isDeviceConnected && prevDeviceConnected) {
    delay(500);
    bleServer->startAdvertising();
#ifdef DEBUG
    Serial.println("Start advertising");
#endif
    prevDeviceConnected = isDeviceConnected;
  }
}

void Esp32ble::write(int a) {
  if (isDeviceConnected) {
    bleTxCharacteristic->setValue(a);
    bleTxCharacteristic->notify();
    delay(10);
  }
  if (!isDeviceConnected && prevDeviceConnected) {
    delay(500);
    bleServer->startAdvertising();
#ifdef DEBUG
    Serial.println("Start advertising");
#endif
    prevDeviceConnected = isDeviceConnected;
  }
}

void Esp32ble::write(float a) {
  if (isDeviceConnected) {
    bleTxCharacteristic->setValue(a);
    bleTxCharacteristic->notify();
    delay(10);
  }
  if (!isDeviceConnected && prevDeviceConnected) {
    delay(500);
    bleServer->startAdvertising();
#ifdef DEBUG
    Serial.println("Start advertising");
#endif
    prevDeviceConnected = isDeviceConnected;
  }
}

uint8_t Esp32ble::available() {
  return bytesremaining;
}

uint8_t Esp32ble::read() {
  if (bytesremaining > 0 && rxdataBuffer != NULL) {
    uint8_t a = rxdataBuffer[rxdatalength - bytesremaining];
    bytesremaining--;
    return a;
  }
  return 0;
}

void Esp32ble::stop() {
  btStop();
}

Esp32ble esp32ble;
#endif
'''


def find_dabble_src() -> Path | None:
    base = Path(env.subst("$PROJECT_LIBDEPS_DIR")) / env.subst("$PIOENV")
    if not base.exists():
        return None
    # Direct folder name first
    direct = base / "DabbleESP32" / "src"
    if direct.exists():
        return direct
    # Fallback: search recursively for the library src dir
    for p in base.rglob("src"):
        if p.parent.name.lower().startswith("dabbleesp32"):
            return p
    return None


def backup_once(path: Path):
    bak = path.with_suffix(path.suffix + ".bak")
    if path.exists() and not bak.exists():
        shutil.copy2(path, bak)


def patch_dabble(*args, **kwargs):
    src = find_dabble_src()
    if src is None:
        print("[patch_dabble] DabbleESP32 source folder not found yet. Build once to install libdeps, then build again.")
        return

    cpp = src / "esp32BLEUtilities.cpp"
    led = src / "LedControlModule.cpp"
    motor = src / "motorControls.cpp"

    if cpp.exists():
        backup_once(cpp)
        cpp.write_text(PATCHED_CPP, encoding="utf-8")
        print(f"[patch_dabble] patched {cpp}")
    else:
        print("[patch_dabble] esp32BLEUtilities.cpp not found")

    for extra in (led, motor):
        if extra.exists():
            backup_once(extra)
            extra.unlink()
            print(f"[patch_dabble] removed unused incompatible source {extra.name}")


patch_dabble()
