#include <Arduino.h>

#define LED_PIN 13

const int buttonPin = 2; // Pin connected to the button
bool buttonClicked;

void setup()
{
	pinMode(buttonPin, INPUT_PULLUP); // Use internal pull-up resistor

	// Set the LED pin as output
	pinMode(LED_PIN, OUTPUT);

	buttonClicked = false;
}

void loop()
{
	int buttonState = digitalRead(buttonPin); // Read the button state
	if (buttonState == HIGH)
	{ // Button pressed (connected to GND)
		if (!buttonClicked)
		{
			digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle the LED state
			buttonClicked = true;
		}
	}
	else
	{
		digitalWrite(LED_PIN, !digitalRead(LED_PIN));
		buttonClicked = false;
	}
}
