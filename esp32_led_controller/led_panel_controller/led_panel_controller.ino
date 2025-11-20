/*
 * ESP32 LED Panel Controller - Production Ready
 * 16x16 WS2812B LED panel with gyroscope-controlled mazeball game
 * Battery management, auto-dimming, and wireless configuration
 * 
 * Hardware: ESP32 DevKit v1, Max17048, BQ25606, TPS61088, MPU6050
 * Version: 2.0 Production - One-Shot Build Ready
 */

#include <FastLED.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "battery_manager.h"
#include "led_control.h"
#include "pattern_engine.h"
#include "sensor_manager.h"
#include "web_server.h"
// Game engine removed - brightness control instead
#include "github_client.h"

// ==================== HARDWARE CONFIGURATION ====================

// LED Panel Configuration
#define LED_PIN 23              // GPIO23 - WS2812B data line (changed from 16)
#define NUM_LEDS 256            // 16x16 matrix = 256 LEDs
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB         // Important: GRB not RGB for WS2812B

// ==================== HARDWARE CONFIGURATION ====================
// All hardware configuration moved to config.h to avoid duplication

// ==================== WIFI CONFIGURATION ====================
// WiFi credentials are now defined in config.h

// ==================== GLOBAL VARIABLES ====================

// LED Painter mode
extern bool painterMode;
extern CRGB painterGrid[MATRIX_HEIGHT][MATRIX_WIDTH];
extern uint8_t painterBrightness;

// Manual brightness control variables
uint8_t manualBrightnessLevel = 0; // 0 = auto, 1-4 = manual levels
const uint8_t brightnessLevels[5] = {0, 80, 140, 180, 220}; // 0=auto, then low to high

// Button state tracking
struct ButtonState {
  bool pressed;
  bool lastPressed;
  unsigned long pressTime;
  unsigned long lastDebounceTime;
  bool longPressTriggered;
};

ButtonState button1, button2, button3;
bool lastButton1State = false;
bool lastButton2State = false; 
bool lastButton3State = false;

// Low battery automatic warning system
bool lowBatteryAutoDisplay = false;
unsigned long lastLowBatteryWarning = 0;
unsigned long startupTime = 0;
// All constants now defined in config.h

// Battery display state
bool manualBatteryDisplay = false;
unsigned long batteryDisplayStartTime = 0;
unsigned long lowBatteryDisplayStartTime = 0;

// Charging detection
bool chargingDetected = false;
unsigned long lastChargingCheck = 0;
#define CHARGING_CHECK_INTERVAL 5000  // Check charging status every 5 seconds

// Timing variables
unsigned long lastFrameTime = 0;

// External declarations for LED arrays (defined in led_control.cpp)
extern CRGB leds[NUM_LEDS];
extern CRGB displayBuffer[NUM_LEDS];
extern uint8_t currentBrightness;
extern bool ledPowerEnabled;

// MPU6050 variables now declared in sensor_manager.cpp
extern float gravityX, gravityY, gravityZ;
extern float calibrationOffsetX, calibrationOffsetY, calibrationOffsetZ;
extern bool gyroCalibrated;

// Pattern variables now declared in pattern_engine.cpp
extern PatternType currentPattern;
extern unsigned long lastPatternUpdate;

// Battery and charging status (declared in battery_manager.cpp)
extern float batteryVoltage;
extern float batteryPercentage;
extern bool isCharging;

// Web server instance (declared in web_server.cpp)
extern WebServer server;

// ==================== UTILITY FUNCTIONS ====================

void setPixel(int x, int y, CRGB color) {
  setLED(x, y, color);
}

void addPixel(int x, int y, CRGB color) {
  addLED(x, y, color);
}

CRGB getPixel(int x, int y) {
  return getLED(x, y);
}

void clearDisplay() {
  clearLEDs();
}

void displayPercentageDigits(float percentage) {
  // Simple 3x5 digit patterns (just show approximate percentage)
  // For simplicity, show dots representing tens digit
  int tens = (int)(percentage / 10);
  
  // Show tens as dots along the bottom edge
  for (int i = 0; i < tens && i < 10; i++) {
    int x = 1 + i;
    if (x < MATRIX_WIDTH - 1) {
      CRGB dotColor = CRGB::White;
      if (percentage >= 50.0) dotColor = CRGB::Green;
      else if (percentage >= 25.0) dotColor = CRGB::Yellow;
      else dotColor = CRGB::Red;
      
      setPixel(x, MATRIX_HEIGHT - 1, dotColor);
    }
  }
  
  // Show percentage as blinking if charging (if we can detect it)
  // This would need charging status from battery manager
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    // Could add charging animation here
    lastBlink = millis();
  }
}

void checkAutomaticLowBatteryWarning() {
  float batteryPercentage = getBatteryPercentage();
  
  // Skip warnings during startup grace period to allow fuel gauge calibration
  if (millis() - startupTime < STARTUP_GRACE_PERIOD) {
    return;
  }
  
  // Only trigger automatic warnings if battery is critically low
  if (batteryPercentage < LOW_BATTERY_THRESHOLD) {
    // Check if it's time for another warning
    if (millis() - lastLowBatteryWarning > LOW_BATTERY_WARNING_INTERVAL) {
      // Trigger automatic low battery display
      lowBatteryAutoDisplay = true;
      batteryDisplayStartTime = millis();
      lastLowBatteryWarning = millis();
      
      Serial.printf("🚨 AUTO LOW BATTERY WARNING: %.1f%% - Displaying for %d seconds (lowBatteryAutoDisplay=%s)\n", 
                    batteryPercentage, LOW_BATTERY_DISPLAY_DURATION / 1000, lowBatteryAutoDisplay ? "TRUE" : "FALSE");
    }
  } else {
    // Reset warning timer if battery is above threshold
    lastLowBatteryWarning = 0;
  }
}

// Charging status is now handled entirely by battery_manager.cpp
// This function just syncs the local variable for web interface compatibility
void checkChargingStatus() {
  // Simply sync with battery manager
  chargingDetected = getChargingStatus();
}

// scanI2CDevices() now in web_server.cpp

// getI2CDeviceList() now in web_server.cpp

void renderPainterMode() {
  // Don't override global brightness - respect battery auto-dimming
  // The painterBrightness is now only for white LED power management
  
  // Clear display first
  clearLEDs();
  
  // Copy painter grid to display buffer with Y-axis flip to match web app drawing
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      CRGB color = painterGrid[y][x];
      
      // Apply white LED power limiting if needed (but not battery dimming)
      if (color.r > 200 && color.g > 200 && color.b > 200) {
        // Scale down bright white colors based on painterBrightness (for power management)
        float scale = (float)painterBrightness / 255.0;
        color.r = (uint8_t)(color.r * scale);
        color.g = (uint8_t)(color.g * scale);
        color.b = (uint8_t)(color.b * scale);
      }
      
      // Flip Y coordinate to match web app drawing orientation
      setLED(x, MATRIX_HEIGHT - 1 - y, color);
    }
  }
}

void showFullScreenBatteryDisplay() {
  // Add safety checks and yields to prevent crashes
  yield(); // Allow other tasks
  
  // Clear entire display first with bounds checking
  for (int i = 0; i < NUM_LEDS && i < 256; i++) {
    leds[i] = CRGB::Black;
  }
  
  yield(); // Allow other tasks
  
  // Get battery percentage with safety check
  float batteryPercentage = getBatteryPercentage();
  if (batteryPercentage < 0.0f) batteryPercentage = 0.0f;
  if (batteryPercentage > 100.0f) batteryPercentage = 100.0f;
  
  bool isLowBattery = (batteryPercentage < LOW_BATTERY_THRESHOLD);
  
  // Battery outline (white or blinking red for low battery)
  CRGB outlineColor = CRGB::White;
  if (isLowBattery && lowBatteryAutoDisplay) {
    // Blink red outline for automatic low battery warning
    outlineColor = (millis() % 1000 < 500) ? CRGB::Red : CRGB(50, 0, 0);
  }
  
  // Draw battery outline (6x10 rectangle centered)
  int batteryX = 5, batteryY = 3;
  int batteryWidth = 6, batteryHeight = 10;
  
  // Battery body outline - USE SAFE COORDINATE FUNCTIONS
  yield(); // Prevent watchdog timeout
  for (int x = batteryX; x < batteryX + batteryWidth; x++) {
    setPixel(x, batteryY, outlineColor);                    // Top
    setPixel(x, batteryY + batteryHeight - 1, outlineColor); // Bottom
  }
  yield(); // Prevent watchdog timeout
  for (int y = batteryY; y < batteryY + batteryHeight; y++) {
    setPixel(batteryX, y, outlineColor);                    // Left
    setPixel(batteryX + batteryWidth - 1, y, outlineColor); // Right
  }
  
  // Battery terminal (positive end) - WITH BOUNDS CHECKING
  if (batteryY > 0) {
    setPixel(batteryX + 2, batteryY - 1, outlineColor);
    setPixel(batteryX + 3, batteryY - 1, outlineColor);
  }
  
  // Fill color based on percentage
  CRGB fillColor;
  if (batteryPercentage > 60) fillColor = CRGB::Green;
  else if (batteryPercentage > 30) fillColor = CRGB::Yellow;
  else if (batteryPercentage > 15) fillColor = CRGB::Orange;
  else fillColor = CRGB::Red;
  
  // Blink fill for automatic low battery warning
  if (isLowBattery && lowBatteryAutoDisplay && (millis() % 800 < 400)) {
    fillColor = CRGB(fillColor.r / 3, fillColor.g / 3, fillColor.b / 3);
  }
  
  // Fill battery based on percentage - USE SAFE COORDINATE FUNCTIONS
  yield(); // Prevent watchdog timeout
  int fillHeight = (batteryHeight - 2) * batteryPercentage / 100.0;
  fillHeight = constrain(fillHeight, 0, batteryHeight - 2); // Safety clamp
  
  for (int y = 0; y < fillHeight; y++) {
    for (int x = batteryX + 1; x < batteryX + batteryWidth - 1; x++) {
      setPixel(x, batteryY + batteryHeight - 2 - y, fillColor);
    }
    if (y % 2 == 0) yield(); // Yield every few iterations
  }
  
  // Show charging indicator if charging (make it more visible)
  if (isCharging) {
    // Yellow lightning bolt pattern - larger and more visible
    CRGB lightningColor = CRGB::Yellow;
    int centerX = batteryX + batteryWidth / 2;
    int centerY = batteryY + batteryHeight / 2;
    
    // Enhanced lightning bolt pattern - override any fill - USE SAFE COORDINATES
    setPixel(centerX, centerY - 2, lightningColor);
    setPixel(centerX - 1, centerY - 1, lightningColor);
    setPixel(centerX, centerY - 1, lightningColor);
    setPixel(centerX, centerY, lightningColor);
    setPixel(centerX + 1, centerY, lightningColor);
    setPixel(centerX, centerY + 1, lightningColor);
    setPixel(centerX + 1, centerY + 1, lightningColor);
    setPixel(centerX, centerY + 2, lightningColor);
    
    // Add sides to make it more obvious
    setPixel(centerX - 1, centerY, lightningColor);
    setPixel(centerX + 1, centerY - 1, lightningColor);
  }
  
  // Display percentage as dots only during automatic low battery warnings
  // Skip this during manual battery display to avoid conflicts
  if (lowBatteryAutoDisplay) {
    displayPercentageDigits(batteryPercentage);
  }
  
  // Flash perimeter ONLY for automatic low battery warning - never during manual display
  if (lowBatteryAutoDisplay && !manualBatteryDisplay && isLowBattery && (millis() % 600 < 300)) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      setPixel(x, 0, CRGB::Red);
      setPixel(x, MATRIX_HEIGHT - 1, CRGB::Red);
    }
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      setPixel(0, y, CRGB::Red);
      setPixel(MATRIX_WIDTH - 1, y, CRGB::Red);
    }
  }
  
  yield(); // Final yield before function exit to prevent crashes
}

// ==================== BATTERY MANAGEMENT ====================

// Battery manager initialization moved to battery_manager.cpp

// All fuel gauge functions moved to battery_manager.cpp

// Battery management functions moved to battery_manager.cpp

// Auto-dimming and LED power functions moved to led_control.cpp

void emergencyShutdown() {
  Serial.println("EMERGENCY SHUTDOWN - Critical battery level!");
  
  // Turn off LED panel power
  disableLEDPower();
  
  // Show critical battery warning
  for (int i = 0; i < 10; i++) {
    setPixel(0, 0, CRGB::Red);
    setPixel(1, 0, CRGB::Red);
    setPixel(0, 1, CRGB::Red);
    setPixel(1, 1, CRGB::Red);
    
    memcpy(leds, displayBuffer, sizeof(CRGB) * NUM_LEDS);
    FastLED.show();
    delay(200);
    
    clearDisplay();
    memcpy(leds, displayBuffer, sizeof(CRGB) * NUM_LEDS);
    FastLED.show();
    delay(200);
  }
  
  // Enter deep sleep
  Serial.println("Entering deep sleep mode...");
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0); // Wake on button 1 press (updated pin)
  esp_deep_sleep_start();
}

// overlayBatteryIndicator function moved to led_control.cpp

// ==================== MPU6050 FUNCTIONS ====================
// MPU6050 functions now in sensor_manager.cpp

// ==================== BATTERY MANAGEMENT ====================

// calibrateGyroscope() now in sensor_manager.cpp

// updateGravity() now in sensor_manager.cpp

// ==================== BUTTON HANDLING ====================

void initializeButtons() {
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  
  // Initialize charging status pin for hardware-based detection (if available)
  #ifdef CHARGING_STATUS_PIN
  pinMode(CHARGING_STATUS_PIN, INPUT_PULLUP); // BQ25606 charging status
  #endif
  
  button1.pressed = false;
  button1.lastPressed = false;
  button1.longPressTriggered = false;
  
  button2.pressed = false;
  button2.lastPressed = false;
  button2.longPressTriggered = false;
  
  button3.pressed = false;
  button3.lastPressed = false;
  button3.longPressTriggered = false;
  
  // Initialize simple state tracking
  lastButton1State = false;
  lastButton2State = false;
  lastButton3State = false;
  
  Serial.println("Buttons initialized");
}

void updateButton(ButtonState* btn, int pin) {
  bool reading = !digitalRead(pin); // Inverted because of pull-up
  
  // Store previous pressed state for release detection
  bool prevPressed = btn->pressed;
  
  // Debouncing
  if (reading != btn->lastPressed) {
    btn->lastDebounceTime = millis();
  }
  
  if ((millis() - btn->lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    if (reading != btn->pressed) {
      btn->pressed = reading;
      
      if (btn->pressed) {
        btn->pressTime = millis();
        btn->longPressTriggered = false;
      }
    }
    
    // Check for long press
    if (btn->pressed && !btn->longPressTriggered && 
        (millis() - btn->pressTime) > BUTTON_LONG_PRESS_MS) {
      btn->longPressTriggered = true;
    }
  }
  
  btn->lastPressed = reading;
}

void handleButtons() {
  // Store previous states for edge detection BEFORE reading new states
  bool prevButton1 = lastButton1State;
  bool prevButton2 = lastButton2State;
  bool prevButton3 = lastButton3State;
  
  // Read current button states
  button1.pressed = digitalRead(BUTTON_PIN_1) == LOW;
  button2.pressed = digitalRead(BUTTON_PIN_2) == LOW;
  button3.pressed = digitalRead(BUTTON_PIN_3) == LOW;
  
  // Update last states for next iteration
  lastButton1State = button1.pressed;
  lastButton2State = button2.pressed;
  lastButton3State = button3.pressed;
  
  // Debug button states (only when they change to reduce spam)
  static bool debugButton1 = false, debugButton2 = false, debugButton3 = false;
  static unsigned long lastButtonDebug = 0;
  
  // Limit button debug messages to prevent serial overflow
  if (millis() - lastButtonDebug > 100) { // Max one debug message per 100ms
    if (button1.pressed != debugButton1) {
      Serial.printf("🔘 Button 1: %s (Pin %d)\n", button1.pressed ? "PRESSED" : "RELEASED", BUTTON_PIN_1);
      debugButton1 = button1.pressed;
      lastButtonDebug = millis();
    } else if (button2.pressed != debugButton2) {
      Serial.printf("🔘 Button 2: %s (Pin %d)\n", button2.pressed ? "PRESSED" : "RELEASED", BUTTON_PIN_2);
      debugButton2 = button2.pressed;
      lastButtonDebug = millis();
    } else if (button3.pressed != debugButton3) {
      Serial.printf("🔘 Button 3: %s (Pin %d)\n", button3.pressed ? "PRESSED" : "RELEASED", BUTTON_PIN_3);
      debugButton3 = button3.pressed;
      lastButtonDebug = millis();
    }
  }

  // Button 1 - Pattern cycling - FIXED RELEASE DETECTION
  if (!button1.pressed && prevButton1) {
    Serial.println("🎨 Button 1 released - processing pattern change");
  
  if (painterMode) {
    // Exit painter mode
    painterMode = false;
    Serial.println("Exited Painter Mode - Back to Patterns");
  } else {
    // In pattern mode: cycle patterns (safer approach)
    static unsigned long lastPatternChange = 0;
    if (millis() - lastPatternChange > BUTTON_COOLDOWN_MS) {
      // Safer pattern cycling without modulo on enums
      switch (currentPattern) {
        case PATTERN_PLASMA_BLOB: currentPattern = PATTERN_RAIN_MATRIX; break;
        case PATTERN_RAIN_MATRIX: currentPattern = PATTERN_RAINBOW_WAVE; break;
        case PATTERN_RAINBOW_WAVE: currentPattern = PATTERN_STARFIELD; break;
        case PATTERN_STARFIELD: currentPattern = PATTERN_RIPPLES; break;
        case PATTERN_RIPPLES: currentPattern = PATTERN_GITHUB_ACTIVITY; break;
        case PATTERN_GITHUB_ACTIVITY: currentPattern = PATTERN_OFF; break;
        case PATTERN_OFF: 
        default: currentPattern = PATTERN_PLASMA_BLOB; break;
      }
      
      // Pattern name lookup for better debugging with bounds checking
      const char* patternNames[] = {
        "Plasma Blob", "Rain Matrix", "Rainbow Wave", "Starfield", 
        "Ripples", "GitHub Activity", "Off", "Unknown"
      };
      
      // Bounds check for safety
      int patternIndex = (currentPattern >= 0 && currentPattern <= 6) ? currentPattern : 7;
      
      Serial.printf("✅ Pattern changed to: %s (%d) (%.1fs since last)\n", 
                    patternNames[patternIndex], currentPattern, 
                    (millis() - lastPatternChange) / 1000.0);
      
      // Notify GitHub client if switching to/from GitHub pattern (with safety)
      yield(); // Yield before calling external function
      extern void setGitHubPatternActive(bool active);
      setGitHubPatternActive(currentPattern == PATTERN_GITHUB_ACTIVITY);
      yield(); // Yield after calling external function
      
      // Reset pattern state timestamp (removed initializePatterns() to prevent crashes)
      lastPatternUpdate = millis();
      lastPatternChange = millis();
      yield(); // Allow ESP32 to handle background tasks
    } else {
      // Provide feedback for ignored rapid clicks
      Serial.printf("⏱️ Button click ignored - too fast (%.0fms remaining)\n", 
                    BUTTON_COOLDOWN_MS - (millis() - lastPatternChange));
    }
  }
  }
  
  // Button 2 - Battery display toggle - FIXED RELEASE DETECTION
  if (!button2.pressed && prevButton2) {
    Serial.println("🔋 Button 2 released - processing battery toggle");
    
    static unsigned long lastBatteryButton = 0;
    if (millis() - lastBatteryButton > BUTTON_DEBOUNCE_MS) {
      
      if (painterMode) {
        // Exit painter mode
        painterMode = false;
        Serial.println("Exited Painter Mode - Back to Patterns");
      } else if (manualBatteryDisplay) {
        // Turn battery display OFF
        Serial.println("🔋 Manual battery display OFF");
        manualBatteryDisplay = false;
      } else {
        // Turn battery display ON
        Serial.println("🔋 Manual battery display ON");
        manualBatteryDisplay = true;
        batteryDisplayStartTime = millis();
      }
      
      yield(); // Prevent watchdog timeout
      lastBatteryButton = millis();
    }
  }
  
  // Button 3 - Brightness control - FIXED RELEASE DETECTION
  if (!button3.pressed && prevButton3) {
    Serial.println("🔆 Button 3 released - processing brightness change");
    static unsigned long lastBrightnessButton = 0;
    unsigned long timeSinceLastPress = millis() - lastBrightnessButton;
    Serial.printf("🔆 Time since last brightness button: %lu ms (debounce: %d ms)\n", 
                  timeSinceLastPress, BUTTON_DEBOUNCE_MS);
    
    if (timeSinceLastPress > BUTTON_DEBOUNCE_MS) {
      
      if (painterMode) {
        // Exit painter mode
        painterMode = false;
        Serial.println("Exited Painter Mode - Back to Patterns");
      } else {
        // Cycle through brightness levels (auto -> low -> med -> high -> max -> auto)
        Serial.printf("🔆 Current brightness level: %d, cycling to next...\n", manualBrightnessLevel);
        manualBrightnessLevel = (manualBrightnessLevel + 1) % 5;
        Serial.printf("🔆 New brightness level: %d\n", manualBrightnessLevel);
        
        if (manualBrightnessLevel == 0) {
          // Auto brightness mode
          Serial.println("💡 Brightness: AUTO (battery controlled)");
          updateAutoDimming(); // Apply current auto-brightness
        } else {
          // Manual brightness mode - respect battery limits
          float batteryPercentage = getBatteryPercentage();
          uint8_t maxAllowedBrightness = getBatteryLimitedMaxBrightness(batteryPercentage);
          uint8_t targetBrightness = brightnessLevels[manualBrightnessLevel];
          
          // Don't exceed battery-safe limits
          if (targetBrightness > maxAllowedBrightness) {
            targetBrightness = maxAllowedBrightness;
          }
          
          setBrightness(targetBrightness);
          yield(); // Prevent watchdog timeout
          
          const char* levelNames[] = {"AUTO", "LOW", "MEDIUM", "HIGH", "MAX"};
          Serial.printf("💡 Brightness: %s (%d) - Battery Limited to %d (%.1f%%)\n", 
                       levelNames[manualBrightnessLevel], targetBrightness, 
                       maxAllowedBrightness, batteryPercentage);
        }
      }
      
      lastBrightnessButton = millis();
    }
  }
}

// Helper function moved to led_control.cpp

// ==================== SENSOR UPDATE FUNCTION ====================

void updateSensors() {
  // Update gravity from MPU6050
  updateGravity(); // from sensor_manager.cpp
  
  // Update battery monitoring (less frequently)
  static unsigned long lastBatteryUpdate = 0;
  static bool batteryUpdateStarted = false;
  if (millis() - lastBatteryUpdate > BATTERY_UPDATE_MS && millis() > 5000) { // Wait 5 seconds after startup
    if (!batteryUpdateStarted) {
      Serial.println("🔋 Starting battery readings from fuel gauge (startup delay complete)");
      batteryUpdateStarted = true;
    }
    updateBatteryManager();
    lastBatteryUpdate = millis();
  }
  
  // Sync charging status for web interface
  static unsigned long lastChargingSync = 0;
  if (millis() - lastChargingSync > 1000) { // Sync once per second
    checkChargingStatus();
    lastChargingSync = millis();
  }
}

// ==================== PATTERN ENGINE ====================
// All pattern functions now in pattern_engine.cpp

// ==================== WEB SERVER ====================
// All web server functions now in web_server.cpp

// ==================== MAIN SETUP AND LOOP ====================

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 LED Panel Controller Starting...");
  Serial.println("Version 2.0 - Production Ready");
  
  // Initialize I2C
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
  
  // Initialize LED panel
  initializeLEDs();
  
  // Initialize battery manager
  initializeBatteryManager();
  
  // Initialize MPU6050 (from sensor_manager)
  initMPU6050();
  
  // Initialize buttons
  initializeButtons();
  
  // Initialize patterns (game logic removed)
  initializePatterns(); // from pattern_engine.cpp
  
  // Setup WiFi and web server
  setupWebServer(); // from web_server.cpp
  
  // Initialize GitHub client (after WiFi setup)
  initializeGitHubClient(); // from github_client.cpp
  
  // Print access information for debugging
  delay(2000);
  Serial.println("=== WiFi Access Information ===");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("🌐 Home WiFi Connected: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("🔗 Web Interface: http://%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.printf("📶 AP Mode Active: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.printf("🔗 Web Interface: http://%s\n", WiFi.softAPIP().toString().c_str());
  }
  Serial.println("================================");
  
  Serial.println("=== Setup Complete ===");
  Serial.println("Ready for operation!");
  
  // Record startup time for grace period
  startupTime = millis();
}

void loop() {
  // Handle web server
  server.handleClient();
  
  // Update sensors (gravity, battery, etc.)
  updateSensors();
  
  // Handle button inputs
  handleButtons();
  
  // Update auto-dimming only if in auto mode
  if (manualBrightnessLevel == 0) {
    static unsigned long lastAutoDimmingUpdate = 0;
    if (millis() - lastAutoDimmingUpdate > 10000) { // Check every 10 seconds
      updateAutoDimming();
      lastAutoDimmingUpdate = millis();
    }
  }
  
  // Handle battery display logic
  bool shouldShowBattery = false;
  
  // Manual battery display with safety timeout
  if (manualBatteryDisplay) {
    // Auto-turn off after timeout OR if too much time has passed (safety)
    if (millis() - batteryDisplayStartTime < BATTERY_DISPLAY_DURATION && 
        millis() - batteryDisplayStartTime < 30000) { // 30 second max safety limit
      shouldShowBattery = true;
    } else {
      manualBatteryDisplay = false;
      Serial.println("🔋 Battery display auto-timeout");
    }
  }
  
  // Auto low battery warning
  static unsigned long lastLowBatteryCheck = 0;
  if (millis() - lastLowBatteryCheck > 30000) { // Check every 30 seconds
    float batteryPercentage = getBatteryPercentage();
    bool isStartupGracePeriod = (millis() - startupTime) < STARTUP_GRACE_PERIOD;
    
    if (batteryPercentage < LOW_BATTERY_THRESHOLD && !isStartupGracePeriod) {
      static unsigned long lastAutoWarning = 0;
      if (millis() - lastAutoWarning > LOW_BATTERY_WARNING_INTERVAL) {
        Serial.printf("🚨 AUTO LOW BATTERY WARNING: %.1f%% - Displaying for %d seconds (lowBatteryAutoDisplay=TRUE)\n", 
                      batteryPercentage, LOW_BATTERY_DISPLAY_DURATION / 1000);
        lowBatteryAutoDisplay = true;
        lowBatteryDisplayStartTime = millis();
        lastAutoWarning = millis();
      }
    }
    
    lastLowBatteryCheck = millis();
  }
  
  // Auto low battery display timeout
  if (lowBatteryAutoDisplay) {
    if (millis() - lowBatteryDisplayStartTime < LOW_BATTERY_DISPLAY_DURATION) {
      shouldShowBattery = true;
    } else {
      lowBatteryAutoDisplay = false;
    }
  }
  
  if (shouldShowBattery) {
    // Only print battery status every 5 seconds to avoid serial spam
    static unsigned long lastBatteryStatusPrint = 0;
    if (millis() - lastBatteryStatusPrint > 5000) {
      Serial.printf("📱 Battery display active: Manual=%s, Auto=%s, Battery=%.1f%%\n",
                    manualBatteryDisplay ? "YES" : "NO",
                    lowBatteryAutoDisplay ? "YES" : "NO",
                    getBatteryPercentage());
      lastBatteryStatusPrint = millis();
    }
    showFullScreenBatteryDisplay();
  } else if (painterMode) {
    // Painter mode is handled via web interface
    // LED updates happen through web server requests
  } else {
    // Normal pattern mode
    static unsigned long lastPatternUpdateTime = 0;
    if (millis() - lastPatternUpdateTime >= PATTERN_UPDATE_MS) {
      updateCurrentPattern();
      lastPatternUpdateTime = millis();
    }
  }
  
  // Always update LEDs
  showLEDs();
  
  // Update GitHub data if pattern is active (with less frequency to prevent crashes)
  static unsigned long lastGitHubUpdate = 0;
  if (millis() - lastGitHubUpdate > 1000) { // Only check every second
    updateGitHubData();
    lastGitHubUpdate = millis();
  }
  
  // Yield instead of delay for better performance
  yield();
}
