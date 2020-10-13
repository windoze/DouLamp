#ifndef LAMP_H
#define LAMP_H

#include <functional>

class Lamp {
public:
    void begin();

    uint8_t getWhite() const;
    uint8_t getYellow() const;
    uint8_t getPower() const;

    void setWhite(uint8_t value);
    void setYellow(uint8_t value);
    void setPower(uint8_t value);
    void togglePower();

    void update();

    std::function<void()> updateCallback;
private:
    uint8_t white = 127;
    uint8_t yellow = 127;
    uint8_t turnedOn = 1;
};

extern Lamp lamp;

#endif