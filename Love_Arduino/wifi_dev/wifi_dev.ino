/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include "certs.h"
#include "credentials.h"

#ifndef STASSID
#define STASSID wifi_ssid
#define STAPSK wifi_password
#endif

ESP8266WiFiMulti WiFiMulti;

unsigned long previousMillis = 0;     // Stores the last time the task ran
const long listener_interval = 10000; // how long to listen to button press
unsigned long currentMillis = 0;

void setup()
{
    Serial.begin(115200);
    // Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(STASSID, STAPSK);
    Serial.println("setup() done connecting to ssid '" STASSID "'");
}

void ensure_connection()
{
    if ((WiFi.status() == 0))
    {
        if (WiFiMulti.run() != WL_CONNECTED) {
            Serial.println("WiFi not connected!");
        } else {
            Serial.println("WiFi connected");
        }
    }
}

std::tuple<bool, bool, bool> check_for_love()
{
    auto client = std::make_unique<BearSSL::WiFiClientSecure>();

    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");

    // Checks for incoming love
    String host = "https://" + String(jigsaw_host) + "/?key=" + user_key + "&user_identifier=" + user_id;

    Serial.print("URL:");
    Serial.println(host);

    if (https.begin(*client, host))
    {
        Serial.print("[HTTPS] GET...\n");
        // start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                String payload = https.getString();
                JsonDocument response;

                deserializeJson(response, payload);

                bool love_received = response["love_received"];
                bool sharing_love = response["sharing_love"];
                Serial.printf("Love Received: %d\n", love_received);
                Serial.printf("Sharing Love: %d\n", sharing_love);

                return std::make_tuple(true, love_received, sharing_love);
            }
        }
        else
        {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
    }

    return std::make_tuple(false, false, false);
}

void loop()
{
    currentMillis = millis();
    ensure_connection();

    // Less time than the interval
    if (currentMillis - previousMillis >= listener_interval)
    {
        check_for_love();

        // save the last time you listened
        previousMillis = currentMillis;
    }
}