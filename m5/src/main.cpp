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
#include <WiFi.h>


#define LED_SHARE_POSITIVE
#define RANGE_MIN   0
#define RANGE_MAX   250
#define SERVICE_UUID "B6935877-54BA-4F86-ABD7-09A4218799DF"
#define CHARACTERISTIC_UUID "3CDB6EAF-EC80-4CCF-9B3E-B78EFA3B28AD"
#define DECOUNCING_DELAY 1000
bool deviceConnected = false;

Preferences preferences;

// Replace with your network credentials
const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";

// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
String header;

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
template<typename T>
void println(T c) {
    Serial.println(c);
}

#   endif
#endif

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

int parse_value(const String &s, size_t idx) {
    sprintf(buf, "P: %s, %d", s.c_str(), idx);
    println(buf);
    int ret = 0;
    int i = 0;
    for (i = idx; i < s.length(); i++) {
        println(s[i]);
        if (s[i] >= '0' && s[i] <= '9') {
            ret = ret * 10 + (s[i] - '0');
        } else {
            break;
        }
    }
    if (i == 0 && ret == 0) {
        return -1;
    }
    return ret;
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

    void onNotify(BLECharacteristic *pCharacteristic) override {
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

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

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

    println("Connecting to ");
    println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        println(".");
    }
    println("");
    println("WiFi connected.");
    println("IP address: ");
    println(WiFi.localIP());
    server.begin();

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

void okResp(WiFiClient &client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:application/json");
    client.println("Connection: close");
    client.println();
}

void errResp(WiFiClient &client) {
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Connection: close");
    client.println();
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
        if (pCharacteristicLum) {
            pCharacteristicLum->notify();
        }
    }
    M5.update();

    WiFiClient client = server.available();   // Listen for incoming clients

    if (client) { // If a new client connects,
        currentTime = millis();
        previousTime = currentTime;
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
            currentTime = millis();
            if (client.available()) {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                header += c;
                if (c == '\n') { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        if (header.indexOf("GET /white") >= 0) {
                            okResp(client);
                            client.println(white);
                            client.println();
                        } else if (header.indexOf("GET /yellow") >= 0) {
                            okResp(client);
                            client.println(yellow);
                            client.println();
                        } else if (header.indexOf("GET /power") >= 0) {
                            okResp(client);
                            client.println(turnedOn ? "1" : "0");
                            client.println();
                        } else if (header.indexOf("PUT /white/") >= 0) {
                            int v = parse_value(header, header.indexOf("PUT /white/") + 11);
                            println("Got value");
                            println(v);
                            if (v >= 0 && v <= 255) {
                                white = v;
                                okResp(client);
                                client.println(white);
                                LampCallbacks::updateLamp();
                            } else {
                                errResp(client);
                            }
                        } else if (header.indexOf("PUT /yellow/") >= 0) {
                            int v = parse_value(header, header.indexOf("PUT /yellow/") + 12);
                            println("Got value");
                            println(v);
                            if (v >= 0 && v <= 255) {
                                yellow = v;
                                okResp(client);
                                client.println(yellow);
                                LampCallbacks::updateLamp();
                            } else {
                                errResp(client);
                            }
                        } else if (header.indexOf("PUT /power/") >= 0) {
                            int v = parse_value(header, header.indexOf("PUT /power/") + 11);
                            println("Got value");
                            println(v);
                            if (v == 0 || v == 1) {
                                turnedOn = (v == 1);
                                okResp(client);
                                client.println(turnedOn);
                                LampCallbacks::updateLamp();
                            } else {
                                errResp(client);
                            }
                        }
                        // Break out of the while loop
                        break;
                    } else { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                } else if (c != '\r') {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }
}
