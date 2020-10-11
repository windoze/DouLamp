//
// Created by Chen Xu on 2020/10/11.
//

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

#define EXTERNAL_NUM_INTERRUPTS 16
#define NUM_DIGITAL_PINS        40
#define NUM_ANALOG_INPUTS       16

#define analogInputToDigitalPin(p)  (((p)<20)?(esp32_adc2gpio[(p)]):-1)
#define digitalPinToInterrupt(p)    (((p)<40)?(p):-1)
#define digitalPinHasPWM(p)         (p < 34)

static const uint8_t SCL = 32;
static const uint8_t SDA = 26;

static const uint8_t SS    = 5;
static const uint8_t MOSI  = 15;
static const uint8_t MISO  = 36;
static const uint8_t SCK   = 13;

static const uint8_t G12 = 12;
static const uint8_t G19 = 19;
static const uint8_t G21 = 21;
static const uint8_t G22 = 22;
static const uint8_t G23 = 23;
static const uint8_t G25 = 25;
static const uint8_t G26 = 26;
static const uint8_t G27 = 27;
static const uint8_t G32 = 32;
static const uint8_t G33 = 33;
static const uint8_t G39 = 39;

#endif //Pins_Arduino_h
