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
#include "Grove_Motor_Driver_TB6612FNG.h"
#include <Wire.h>

#define LED_SHARE_POSITIVE
#define RANGE_MIN   20
#define RANGE_MAX   250
#define SERVICE_UUID "B6935877-54BA-4F86-ABD7-09A4218799DF"
#define CHARACTERISTIC_UUID "3CDB6EAF-EC80-4CCF-9B3E-B78EFA3B28AD"
#define DECOUNCING_DELAY 1000
bool deviceConnected = false;

Preferences preferences;

BLECharacteristic *pCharacteristicLum = nullptr;

char buf[100];
uint8_t white = 127;
uint8_t yellow = 127;
uint8_t turnedOn = 1;

MotorDriver motor; // NOLINT(cert-err58-cpp)

// M5Atom doesn't have LCD display
#ifdef NDEBUG
template<typename T> void println(T c) {}
#else
#   ifdef ARDUINO_M5Stick_C
// Use LCD for debugging on M5Stick-C
static int line_count = 0;
template<typename T>
void println(T c) {
    M5.Lcd.println(c);
    line_count++;
    if (line_count > 10)
    {
        M5.Lcd.fillScreen(0);
        M5.Lcd.setCursor(0, 0);
        line_count = 0;
    }
}
#   else
// Use serial for debugging on M5Atom
template<typename T> void println(T c) {
    Serial.println(c);
}
#   endif
#endif

uint8_t validate_lum(uint8_t value) {
    if(value==0) value=1;
#ifdef LED_SHARE_POSITIVE
    value = 256-value;
#endif
    double fv = value;
    // range is [RANGE_MIN, RANGE_MAX]
    fv = fv/255.0 * (RANGE_MAX-RANGE_MIN+1) + RANGE_MIN;
    return uint8_t(fv);
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

    void onNotify(BLECharacteristic* pCharacteristic) override {
        uint8_t values[3];
        println("notify");
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
        preferences.putUChar("WHITE", white);
        preferences.putUChar("YELLOW", yellow);
        sprintf(buf, "Values: %d, %d, %d", white, yellow, turnedOn);
        println(buf);
        updateLamp();
    }

public:
    static void updateLamp() {
        if (turnedOn) {
            // if (white == 0) {
            //     motor.dcMotorStop(MOTOR_CHA);
            // } else {
                sprintf(buf, "W: %d, %d", white, validate_lum(white));
                println(buf);
                motor.dcMotorRun(MOTOR_CHA, validate_lum(white));
            // }
            // if (yellow == 0) {
            //     motor.dcMotorStop(MOTOR_CHB);
            // } else {
                sprintf(buf, "Y: %d, %d", yellow, validate_lum(yellow));
                println(buf);
                motor.dcMotorRun(MOTOR_CHB, validate_lum(yellow));
            // }
        } else {
            motor.dcMotorStop(MOTOR_CHA);
            motor.dcMotorStop(MOTOR_CHB);
        }
    }
};

void turnOffLcd() {
#if defined(ARDUINO_M5Stick_C) && defined(NDEBUG)
    M5.Axp.ScreenBreath(0);
    Wire1.beginTransmission(0x34);
    Wire1.write(0x12);
    Wire1.write(0b01001011);  // LDO2, aka OLED_VDD, off
    Wire1.endTransmission();
#endif
}

void setup() {
    preferences.begin("lamp-config");
    delay(10);
    white = preferences.getUChar("WHITE", 127);
    yellow = preferences.getUChar("YELLOW", 127);

    Wire.begin();
#ifndef NDEBUG
    Serial.begin(115200);
#endif
    motor.init();
    M5.begin();
    turnOffLcd();

    LampCallbacks::updateLamp();

    println("BLE start.");

    BLEDevice::init("豆豆的台灯");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristicLum = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE);
    pCharacteristicLum->setCallbacks(new LampCallbacks);
    pCharacteristicLum->addDescriptor(new BLE2902());

    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
}

void loop() {
    if (
#ifdef ARDUINO_M5Stick_C
        M5.BtnA.wasPressed() || M5.BtnB.wasPressed()
#else
        M5.Btn.wasPressed()
#endif
    ) {
        println("Button clicked.");
        turnedOn = (turnedOn == 0);
        LampCallbacks::updateLamp();
        if(pCharacteristicLum) {
            pCharacteristicLum->notify();
        }
    }
    M5.update();
}
