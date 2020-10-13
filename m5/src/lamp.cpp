#ifdef ARDUINO_M5Atom
#include <M5Atom.h>
#endif
#ifdef ARDUINO_M5Stick_C
#include <M5StickC.h>
#endif
#include <Preferences.h>
#include "Grove_Motor_Driver_TB6612FNG.h"

#include "utils.h"

#include "lamp.h"

#define LED_SHARE_POSITIVE
#define RANGE_MIN   0
#define RANGE_MAX   250

static Preferences preferences;
static MotorDriver motor; // NOLINT(cert-err58-cpp)

uint8_t validate_lum(uint8_t value) {
    if (value == 0) value = 1;
#ifdef LED_SHARE_POSITIVE
    value = 256 - value;
#endif
    double fv = value;
    // range is [RANGE_MIN, RANGE_MAX]
    fv = fv / 255.0 * (RANGE_MAX - RANGE_MIN + 1) + RANGE_MIN;
    return uint8_t(fv);
}

void Lamp::begin() {
    preferences.begin("lamp-config");
    delay(10);
    white = preferences.getUChar("WHITE", 127);
    yellow = preferences.getUChar("YELLOW", 127);

    // Turn on on powerup
    turnedOn = 1;

    motor.init();

    update();
}

uint8_t Lamp::getWhite() const {
    return white;
}

uint8_t Lamp::getYellow() const {
    return yellow;
}

uint8_t Lamp::getPower() const {
    return turnedOn;
}

void Lamp::setWhite(uint8_t value) {
    white = value;
    println("Lamp: setting white, raw value ", +value, ", converted ", +validate_lum(white));
    preferences.putUChar("WHITE", white);
}

void Lamp::setYellow(uint8_t value) {
    yellow = value;
    println("Lamp: setting yellow, raw value ", +value, ", converted ", +validate_lum(yellow));
    preferences.putUChar("YELLOW", yellow);
}

void Lamp::setPower(uint8_t value) {
    turnedOn = (value>0);
    println("Lamp: setting power value ", +turnedOn);
}

void Lamp::togglePower() {
    setPower(getPower()==0);
}

void Lamp::update() {
    if (turnedOn) {
        motor.dcMotorRun(MOTOR_CHA, validate_lum(white));
        motor.dcMotorRun(MOTOR_CHB, validate_lum(yellow));
    } else {
        motor.dcMotorStop(MOTOR_CHA);
        motor.dcMotorStop(MOTOR_CHB);
    }
    if(updateCallback) {
        updateCallback();
    }
}

Lamp lamp;