#include "LedControl.h"
#include "binary.h"

#define CLK D3
#define CS D4
#define DIN D5

LedControl led_control = LedControl(DIN, CLK, CS, 1);

const int heartbeatInterval = 1000;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
bool showHeart = false;
bool loveShared = true;

// Heart pattern
const byte HEART[8] = {B00000000, B01100110, B11111111, B11111111, B11111111, B01111110, B00111100, B00011000};

void setup()
{
    Serial.begin(9600);
    Serial.println("Initializing...");

    // Initialize the MAX7219 module
    led_control.shutdown(0, false); // Wake up the display
    led_control.setIntensity(0, 1); // Set low brightness
    led_control.clearDisplay(0);    // Clear the display

    Serial.println("Initialization complete.");
}

void draw_screen()
{
    for (int i = 0; i < 8; i++)
    {
        led_control.setRow(0, i, HEART[i]); // Send each row of the HEART pattern
    }
}

void loop()
{
    currentMillis = millis();

    // Check if it's time to update the display
    if (loveShared)
    {
        if (currentMillis - previousMillis >= heartbeatInterval)
        {
            previousMillis = currentMillis; // Update the timer

            if (showHeart)
            {
                draw_screen(); // Show the heart
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
        draw_screen();
    }
}
