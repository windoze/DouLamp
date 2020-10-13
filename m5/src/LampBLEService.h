#ifndef LAMP_BLE_SERVICE_H
#define LAMP_BLE_SERVICE_H

class Lamp;
class BLECharacteristic;

class LampBLEService {
public:
    void begin(Lamp *);
    void valueChanged();

private:
    Lamp *pLamp = nullptr;
    BLECharacteristic *pCharacteristicLum = nullptr;
};

extern LampBLEService bleService;

#endif