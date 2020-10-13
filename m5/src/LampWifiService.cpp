#ifdef ARDUINO_M5Atom
#include <M5Atom.h>
#endif
#ifdef ARDUINO_M5Stick_C
#include <M5StickC.h>
#endif

#include <Wire.h>
#include <WiFi.h>

#include "utils.h"

#include "lamp.h"
#include "LampWifiService.h"

#include "private_ssid.h"

// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
String header;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

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

int parse_value(const String &s, size_t idx) {
    int ret = 0;
    int i = 0;
    for (i = idx; i < s.length(); i++) {
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

void LampWIFIService::begin(Lamp *p) {
    pLamp = p;

    WiFi.begin(ssid, password);
    println("Connecting to ", ssid);
    while (WiFi.status() != WL_CONNECTED) {
        print('.');
        delay(500);
    }
    println("Connected to ", ssid);
    server.begin();
}

void LampWIFIService::onLoop() {
    WiFiClient client = server.available();   // Listen for incoming clients

    if (client) { // If a new client connects,
        currentTime = millis();
        previousTime = currentTime;
        IPAddress addr = client.remoteIP();
        println("New Client connection from: ", addr[0], '.', addr[1], '.', addr[2], '.', addr[3]); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
            currentTime = millis();
            if (client.available()) {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                print(c);        // print it out the serial monitor
                header += c;
                if (c == '\n') { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        if (header.indexOf("GET /white") >= 0) {
                            okResp(client);
                            client.println(pLamp->getWhite());
                            client.println();
                        } else if (header.indexOf("GET /yellow") >= 0) {
                            okResp(client);
                            client.println(pLamp->getYellow());
                            client.println();
                        } else if (header.indexOf("GET /power") >= 0) {
                            okResp(client);
                            client.println(pLamp->getPower() ? "1" : "0");
                            client.println();
                        } else if (header.indexOf("PUT /white/") >= 0) {
                            int v = parse_value(header, header.indexOf("PUT /white/") + 11);
                            if (v >= 0 && v <= 255) {
                                pLamp->setWhite(v);
                                pLamp->update();
                                okResp(client);
                                client.println(pLamp->getWhite());
                            } else {
                                errResp(client);
                            }
                        } else if (header.indexOf("PUT /yellow/") >= 0) {
                            int v = parse_value(header, header.indexOf("PUT /yellow/") + 12);
                            if (v >= 0 && v <= 255) {
                                pLamp->setYellow(v);
                                pLamp->update();
                                okResp(client);
                                client.println(pLamp->getYellow());
                            } else {
                                errResp(client);
                            }
                        } else if (header.indexOf("PUT /power/") >= 0) {
                            int v = parse_value(header, header.indexOf("PUT /power/") + 11);
                            if (v == 0 || v == 1) {
                                pLamp->setPower(v == 1);
                                pLamp->update();
                                okResp(client);
                                client.println(pLamp->getPower());
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
        println("Client disconnected.");
    }
}

LampWIFIService wifiService;