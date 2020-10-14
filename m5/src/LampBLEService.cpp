#ifdef ARDUINO_M5Atom
#include <M5Atom.h>
#endif
#ifdef ARDUINO_M5Stick_C
#include <M5StickC.h>
#endif

#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Wire.h>

#include "utils.h"

#include "lamp.h"
#include "LampBLEService.h"

#define SERVICE_UUID "B6935877-54BA-4F86-ABD7-09A4218799DF"
#define CHARACTERISTIC_UUID "3CDB6EAF-EC80-4CCF-9B3E-B78EFA3B28AD"

class LampServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) override {
        println("BLE connected.");
    };

    void onDisconnect(BLEServer *pServer) override {
        println("BLE disconnected.");
    }
};

class LampCallbacks : public BLECharacteristicCallbacks {
public:
    LampCallbacks(Lamp &l) : lamp(l) {}

private:
    void onRead(BLECharacteristic *pCharacteristic) override {
        uint8_t values[3];
        values[0] = lamp.getWhite();
        values[1] = lamp.getYellow();
        values[2] = lamp.getPower();
        println("BLE: Sending values: ", +values[0], " ", +values[1], " ", +values[2]);
        pCharacteristic->setValue(std::string(reinterpret_cast<char const *>(values), 3));
    }

    void onNotify(BLECharacteristic *pCharacteristic) override {
        uint8_t values[3];
        values[0] = lamp.getWhite();
        values[1] = lamp.getYellow();
        values[2] = lamp.getPower();
        println("BLE: Sending notification values: ", +values[0], " ", +values[1], " ", +values[2]);
        pCharacteristic->setValue(std::string(reinterpret_cast<char const *>(values), 3));
    }

    void onWrite(BLECharacteristic *pCharacteristic) override {
        std::string values = pCharacteristic->getValue();
        if (values.size() < 3) {
            return;
        }
        println("BLE: Received values: ", +values[0], " ", +values[1], " ", +values[2]);
        lamp.setWhite(values[0]);
        lamp.setYellow(values[1]);
        lamp.setPower(values[2]);
        lamp.update();
    }

    Lamp &lamp;
};

void LampBLEService::begin(Lamp *p) {
    pLamp = p;

    BLEDevice::init("豆豆的台灯");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new LampServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristicLum = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);
    pCharacteristicLum->setCallbacks(new LampCallbacks(*pLamp));
    pCharacteristicLum->addDescriptor(new BLE2902());

    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    println("Begin advertising");
    p->updateCallback = []{
        if (bleService.pCharacteristicLum) {
            bleService.pCharacteristicLum->notify();
        }
    };
}

LampBLEService bleService;