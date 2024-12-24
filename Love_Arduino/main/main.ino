#include <Arduino.h>
#include "LedControl.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include "certs.h"
#include "credentials.h"

const int buttonPin = D1;		 // Pin connected to the button
const int ledPin = D4;			 // Pin connected to the LED
const long ledInterval = 120000; // 2 minutes

bool buttonClicked;
unsigned long previousMillisLoveChecked = 0; // Stores the last time the task ran
unsigned long currentMillisLoveChecked = 0;

unsigned long previousMillisLoveSentLED = 0; // Stores the last time the task ran
unsigned long currentMillisLoveSentLED = 0;
bool loveSent = false;

unsigned long currentMillisScreen = 0;
unsigned long previousMillisScreen = 0;

bool showHeart = false;
bool loveShared = false;
bool loveReceived = false;
const int heartbeatInterval = 1000;
const int listenerInterval = 10000;

#ifndef STASSID
#define STASSID wifi_ssid
#define STAPSK wifi_password
#endif
#define CLK D6
#define CS D7
#define DIN D8

ESP8266WiFiMulti WiFiMulti;
LedControl led_control = LedControl(DIN, CLK, CS, 1);

// Heart pattern
const byte HEART[8] = {B00000000, B01100110, B11111111, B11111111, B11111111, B01111110, B00111100, B00011000};

// X pattern
const byte X[8] = {B10000001, B01000010, B00100100, B00011000, B00011000, B00100100, B01000010, B10000001};

void setup()
{
	pinMode(buttonPin, INPUT_PULLUP); // Use internal pull-up resistor
	pinMode(ledPin, OUTPUT);		  // Set the LED pin as output

	Serial.begin(115200);

	Serial.println("Initializing...");

	buttonClicked = false;

	// Initialize the MAX7219 module
	led_control.shutdown(0, false); // Wake up the display
	led_control.setIntensity(0, 1); // Set low brightness
	led_control.clearDisplay(0);	// Clear the display

	Serial.println("Initialization complete.");

	WiFi.mode(WIFI_STA);
	WiFiMulti.addAP(STASSID, STAPSK);
	Serial.println("setup() done connecting to ssid '" STASSID "'");
}

void draw_heart()
{
	for (int i = 0; i < 8; i++)
	{
		led_control.setRow(0, i, HEART[i]); // Send each row of the HEART pattern
	}
}

// Draws an X on error
void draw_x()
{
	for (int i = 0; i < 8; i++)
	{
		led_control.setRow(0, i, X[i]); // Send each row of the X pattern
	}
}

void handle_heart()
{
	currentMillisScreen = millis();

	if (loveReceived)
	{
		// Check if it's time to update the display
		if (loveShared)
		{
			if (currentMillisScreen - previousMillisScreen >= heartbeatInterval)
			{
				previousMillisScreen = currentMillisScreen; // Update the timer

				if (showHeart)
				{
					draw_heart(); // Show the heart
				}
				else
				{
					led_control.clearDisplay(0); // Clear the display
				}

				showHeart = !showHeart; // Toggle the state
			}
		}
		else
		{
			draw_heart();
		}
	}
}

void ensure_connection()
{
	if ((WiFi.status() == 0))
	{
		if (WiFiMulti.run() != WL_CONNECTED)
		{
			Serial.println("WiFi not connected!");
		}
		else
		{
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

std::tuple<bool, bool> send_love()
{
	auto client = std::make_unique<BearSSL::WiFiClientSecure>();

	client->setInsecure();

	HTTPClient https;

	Serial.print("[HTTPS] begin...\n");

	// Checks for incoming love
	String host = "https://" + String(jigsaw_host) + "/send_love?key=" + user_key + "&user_identifier=" + user_id;

	Serial.print("URL:");
	Serial.println(host);

	if (https.begin(*client, host))
	{
		Serial.print("[HTTPS] POST...\n");
		// start connection and send HTTP header
		int httpCode = https.POST("");

		// httpCode will be negative on error
		if (httpCode > 0)
		{
			// HTTP header has been send and Server response header has been handled
			Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

			// file found at server
			if (httpCode == HTTP_CODE_CREATED || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
			{
				String payload = https.getString();
				JsonDocument response;

				deserializeJson(response, payload);

				bool love_sent = response["love_sent"];
				Serial.printf("Love Sent: %d\n", love_sent);

				return std::make_tuple(true, love_sent);
			}
		}
		else
		{
			Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
		}

		https.end();
	}

	return std::make_tuple(false, false);
}

void loop()
{
	// Make sure we are connected to the wifi
	ensure_connection();

	int buttonState = digitalRead(buttonPin); // Read the button state

	// Handle button listener
	if (buttonState == HIGH)
	{ // Button pressed (connected to GND)
		if (!buttonClicked)
		{
			std::tuple<bool, bool> loveSentResponse = send_love();

			if (std::get<0>(loveSentResponse) && std::get<1>(loveSentResponse))
			{
				buttonClicked = true;
				loveSent = true;

				previousMillisLoveSentLED = millis();
				Serial.println("Love sent");
			}
			else
			{
				draw_x();
			}
		}
	}
	else
	{
		buttonClicked = false;
	}

	// Handle love sent LED
	if (loveSent)
	{
		currentMillisLoveSentLED = millis();

		if (currentMillisLoveSentLED - previousMillisLoveSentLED >= ledInterval)
		{
			previousMillisLoveSentLED = currentMillisLoveSentLED; // Update the timer

			loveSent = false;

			digitalWrite(ledPin, LOW); // Turn off the LED
		}
		else
		{
			digitalWrite(ledPin, HIGH); // Turn on the LED
		}
	}

	currentMillisLoveChecked = millis();

	// More or equal time than the interval
	if (currentMillisLoveChecked - previousMillisLoveChecked >= listenerInterval)
	{
		std::tuple<bool, bool, bool> checkForLoveReponse = check_for_love();

		if (std::get<0>(checkForLoveReponse))
		{
			loveReceived = std::get<1>(checkForLoveReponse);
			loveShared = std::get<2>(checkForLoveReponse);

			Serial.println("Love received");
		}
		else
		{
			draw_x();
		}

		// Save the last time we listened
		previousMillisLoveChecked = currentMillisLoveChecked;
	}

	handle_heart();
}
