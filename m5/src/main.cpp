#ifdef ARDUINO_M5Atom
#include <M5Atom.h>
#endif
#ifdef ARDUINO_M5Stick_C
#include <M5StickC.h>
#endif
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "Grove_Motor_Driver_TB6612FNG.h"
#include <Wire.h>

#define SERVICE_UUID            "B6935877-54BA-4F86-ABD7-09A4218799DF"
#define CHARACTERISTIC_UUID     "3CDB6EAF-EC80-4CCF-9B3E-B78EFA3B28AD"
bool deviceConnected = false;

char buf[100];
uint8_t white = 127;
uint8_t yellow = 127;
uint8_t turnedOn = 0;

MotorDriver motor; // NOLINT(cert-err58-cpp)

static int line_count = 0;

template<typename T>
void println(T c) {
#ifdef ARDUINO_M5Stick_C
    M5.Lcd.println(c);
    line_count++;
    if (line_count > 10) {
        M5.Lcd.fillScreen(0);
        M5.Lcd.setCursor(0, 0);
        line_count = 0;
    }
#endif
}

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) override {
        println("connect");
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer) override {
        println("disconnect");
        deviceConnected = false;
    }
};

class LampCallbacks : public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pCharacteristic) override {
        uint8_t values[3];
        println("read");
        values[0] = white;
        values[1] = yellow;
        values[2] = turnedOn;
        pCharacteristic->setValue(std::string(reinterpret_cast<char const *>(values), 3));
    }

    void onWrite(BLECharacteristic *pCharacteristic) override {
        println("write");
        std::string value = pCharacteristic->getValue();
        if (value.size() < 3) {
            println("Wrong value.");
            return;
        }
        white = value[0];
        yellow = value[1];
        turnedOn = (value[2] > 0);
        sprintf(buf, "Values: %d, %d, %d", white, yellow, turnedOn);
        println(buf);
        updateLamp();
    }

    static void updateLamp() {
        if (turnedOn) {
            if (white == 0) {
                motor.dcMotorStop(MOTOR_CHA);
            } else {
                motor.dcMotorRun(MOTOR_CHA, white);
            }
            if (yellow == 0) {
                motor.dcMotorStop(MOTOR_CHB);
            } else {
                motor.dcMotorRun(MOTOR_CHB, yellow);
            }
        } else {
            motor.dcMotorStop(MOTOR_CHA);
            motor.dcMotorStop(MOTOR_CHB);
        }
    }
};

void setup() {
    Wire.begin();
    Serial.begin(115200);
    motor.init();
    M5.begin();

    println("BLE start.");

    BLEDevice::init("豆豆的台灯");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *pCharacteristicLum = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_INDICATE
    );
    pCharacteristicLum->setCallbacks(new LampCallbacks);
    pCharacteristicLum->addDescriptor(new BLE2902());

    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
}

void loop() {
    M5.update();
}
