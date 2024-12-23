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

void loop()
{
    // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {

        auto client = std::make_unique<BearSSL::WiFiClientSecure>();

        client->setInsecure();

        HTTPClient https;

        Serial.print("[HTTPS] begin...\n");

        // Checks for incoming love
        String host = "https://" + String(jigsaw_host) + "/" + user_key + "?user_identifier=" + user_id;

        Serial.print("URL:");
        Serial.println(host);

        if (https.begin(*client, host))
        { // HTTPS

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
                    // String payload = https.getString(1024);  // optionally pre-reserve string to avoid reallocations in chunk mode
                    JsonDocument json_doc;

                    deserializeJson(json_doc, json);
                    Serial.println(payload);
                }
            }
            else
            {
                Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }

            https.end();
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect\n");
        }
    }

    Serial.println("Wait 10s before next round...");
    delay(10000);
}
