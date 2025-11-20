/*
 * LED Control Module Implementation
 * Handles LED panel initialization, basic operations, and power management
 */

#include "led_control.h"
#include "battery_manager.h"
#include "config.h"

// LED arrays
CRGB leds[NUM_LEDS];
CRGB displayBuffer[NUM_LEDS];

// State variables
uint8_t currentBrightness = BRIGHTNESS_100_PERCENT;
bool ledPowerEnabled = true;

void initializeLEDs() {
  DEBUG_INFO("Initializing LED panel...");
  
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(currentBrightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MW / 5);
  FastLED.clear();
  
  // TPS61088 PWM control not wired yet - boost converter runs continuously
  // pinMode(TPS61088_PWM_PIN, OUTPUT);
  // enableLEDPower();
  ledPowerEnabled = true;
  
  // Clear display buffer
  clearLEDs();
  
  DEBUG_INFO("LED panel initialized successfully (boost converter always on)");
}

void clearLEDs() {
  fill_solid(displayBuffer, NUM_LEDS, CRGB::Black);
}

void setLED(int x, int y, CRGB color) {
  if (isValidCoordinate(x, y)) {
    displayBuffer[xyToIndex(x, y)] = color;
  }
}

void addLED(int x, int y, CRGB color) {
  if (isValidCoordinate(x, y)) {
    displayBuffer[xyToIndex(x, y)] += color;
  }
}

CRGB getLED(int x, int y) {
  if (isValidCoordinate(x, y)) {
    return displayBuffer[xyToIndex(x, y)];
  }
  return CRGB::Black;
}

void copyBufferToLEDs() {
  memcpy(leds, displayBuffer, sizeof(CRGB) * NUM_LEDS);
}

void showLEDs() {
  copyBufferToLEDs();
  if (ledPowerEnabled) {
    FastLED.show();
  }
}

void updateAutoDimming() {
  #if ENABLE_AUTO_DIMMING
  float batteryPercentage = getBatteryPercentage();
  uint8_t newBrightness;
  
  if (batteryPercentage >= 75.0) {
    newBrightness = BRIGHTNESS_100_PERCENT;
  } else if (batteryPercentage >= 50.0) {
    newBrightness = BRIGHTNESS_75_PERCENT;
  } else if (batteryPercentage >= 25.0) {
    newBrightness = BRIGHTNESS_50_PERCENT;
  } else if (batteryPercentage >= 10.0) {
    newBrightness = BRIGHTNESS_25_PERCENT;
  } else {
    newBrightness = BRIGHTNESS_LOW_BATTERY;
  }
  
  // Only update if brightness changed
  if (newBrightness != currentBrightness) {
    setBrightness(newBrightness);
    DEBUG_INFO("Auto-dimming: Battery " + String(batteryPercentage, 1) + "% -> Brightness " + String(newBrightness));
  }
  #endif
}

void enableLEDPower() {
  // TPS61088 PWM control not wired yet - boost converter running continuously  
  // digitalWrite(TPS61088_PWM_PIN, HIGH);
  ledPowerEnabled = true;
  DEBUG_INFO("LED power enabled (boost converter always on)");
}

void disableLEDPower() {
  // TPS61088 PWM control not wired yet - can only disable LEDs, not boost converter
  // digitalWrite(TPS61088_PWM_PIN, LOW);
  ledPowerEnabled = false;
  FastLED.clear();
  FastLED.show();
  DEBUG_INFO("LED power disabled (boost converter still on)");
}

void setBrightness(uint8_t brightness) {
  currentBrightness = CLAMP(brightness, POWER_LEVEL_MIN, POWER_LEVEL_MAX);
  FastLED.setBrightness(currentBrightness);
}

uint8_t getCurrentBrightness() {
  return currentBrightness;
}

uint8_t getBatteryLimitedMaxBrightness(float batteryPercentage) {
  if (batteryPercentage >= 75.0) {
    return BRIGHTNESS_100_PERCENT;
  } else if (batteryPercentage >= 50.0) {
    return BRIGHTNESS_75_PERCENT;
  } else if (batteryPercentage >= 25.0) {
    return BRIGHTNESS_50_PERCENT;
  } else if (batteryPercentage >= 10.0) {
    return BRIGHTNESS_25_PERCENT;
  } else {
    return BRIGHTNESS_LOW_BATTERY;
  }
}

void overlayBatteryIndicator() {
  // This function is now replaced by showFullScreenBatteryDisplay() in main file
  // Keeping this for compatibility but functionality moved to main file
}

void drawBatteryIcon(int x, int y, float percentage) {
  // Simple 3x2 battery icon
  CRGB outlineColor = CRGB::White;
  CRGB fillColor = (percentage > 25.0) ? CRGB::Green : CRGB::Red;
  
  // Battery outline
  setLED(x, y, outlineColor);
  setLED(x + 1, y, outlineColor);
  setLED(x, y + 1, outlineColor);
  setLED(x + 1, y + 1, outlineColor);
  setLED(x + 2, y, outlineColor);     // Terminal
  
  // Battery fill based on percentage
  if (percentage > 50.0) {
    setLED(x, y, fillColor);
  }
  if (percentage > 0.0) {
    setLED(x + 1, y, fillColor);
  }
}

uint16_t xyToIndex(uint8_t x, uint8_t y) {
  // Convert X,Y coordinates to LED index
  // Assumes serpentine (zigzag) wiring pattern
  if (y & 0x01) {
    // Odd rows run backwards
    return (y * MATRIX_WIDTH) + (MATRIX_WIDTH - 1 - x);
  } else {
    // Even rows run forwards
    return (y * MATRIX_WIDTH) + x;
  }
}

bool isValidCoordinate(int x, int y) {
  return (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT);
}

void fadeToBlack(uint8_t fadeAmount) {
  for (int i = 0; i < NUM_LEDS; i++) {
    displayBuffer[i].fadeToBlackBy(fadeAmount);
  }
} 