/*
 * LED Control Module
 * Handles LED panel initialization, basic operations, and power management
 */

#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "config.h"
#include <FastLED.h>

// LED array and display buffer
extern CRGB leds[NUM_LEDS];
extern CRGB displayBuffer[NUM_LEDS];

// Current brightness and power settings
extern uint8_t currentBrightness;
extern bool ledPowerEnabled;

// Function declarations
void initializeLEDs();
void clearLEDs();
void setLED(int x, int y, CRGB color);
void addLED(int x, int y, CRGB color);
CRGB getLED(int x, int y);
void copyBufferToLEDs();
void showLEDs();

// Power management
void updateAutoDimming();
void enableLEDPower();
void disableLEDPower();
void setBrightness(uint8_t brightness);
uint8_t getCurrentBrightness();
uint8_t getBatteryLimitedMaxBrightness(float batteryPercentage);

// Battery display overlay
void overlayBatteryIndicator();
void drawBatteryIcon(int x, int y, float percentage);

// Utility functions
uint16_t xyToIndex(uint8_t x, uint8_t y);
bool isValidCoordinate(int x, int y);
void fadeToBlack(uint8_t fadeAmount);

#endif // LED_CONTROL_H 