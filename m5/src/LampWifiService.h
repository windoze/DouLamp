#ifndef LAMP_WIFI_SERVICE_H
#define LAMP_WIFI_SERVICE_H

class Lamp;

class LampWIFIService {
public:
    void begin(Lamp *);
    void onLoop();

private:
    Lamp * pLamp;
};

extern LampWIFIService wifiService;

#endif