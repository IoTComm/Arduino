#ifndef SEAT_STATUS_SENDER_H
#define SEAT_STATUS_SENDER_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

void setupWiFi();
String sendSeatStatus(String &payload);
String sendeEmptySeatJson();

#endif // SEAT_STATUS_SENDER_H
