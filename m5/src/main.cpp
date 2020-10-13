#ifdef ARDUINO_M5Atom
#include <M5Atom.h>
#endif
#ifdef ARDUINO_M5Stick_C
#include <M5StickC.h>
#endif
#include <Wire.h>

#include "utils.h"

#include "lamp.h"
#include "LampBLEService.h"
#include "LampWifiService.h"

void setup() {
    Wire.begin();
    Serial.begin(115200);
    M5.begin();
    turnOffLcd();

    lamp.begin();
    bleService.begin(&lamp);
    wifiService.begin(&lamp);
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
        lamp.togglePower();
    }
    M5.update();
    wifiService.onLoop();
}
