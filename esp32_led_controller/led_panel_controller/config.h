/*
 * Configuration Header File
 * Pin definitions, constants, and compile-time settings
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FastLED.h>

// ==================== HARDWARE CONFIGURATION ====================

// LED Panel Configuration
#define LED_PIN 23              // GPIO pin for WS2812B data line (changed from 16)
#define NUM_LEDS 256            // 16x16 matrix = 256 LEDs
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB         // Important: GRB not RGB for WS2812B

// Power Management Pins
// #define TPS61088_PWM_PIN 15     // PWM control for TPS61088 boost converter (not wired yet)
#define BATTERY_ADC_PIN 5       // Battery voltage monitoring via ADC (changed from 26)

// Button Pins
#define BUTTON_PIN_1 14         // Pattern/Mode cycling (changed from 18)
#define BUTTON_PIN_2 27         // Battery display toggle (changed from 19)
#define BUTTON_PIN_3 26         // Brightness control (changed from game mode)

// Battery Management & Alert Pins
#define FUEL_GAUGE_ALERT_PIN 16 // Max17043 ALT pin → RX2 (GPIO 16)
#define FUEL_GAUGE_QST_PIN 2    // Max17043 QST pin → D2 (GPIO 2)
#define CHARGING_STATUS_PIN 33  // BQ25606 charging status (optional)

// I2C Configuration
#define I2C_SDA_PIN 22          // I2C Data line (swapped from 21)
#define I2C_SCL_PIN 21          // I2C Clock line (swapped from 22)
#define I2C_FREQUENCY 100000    // 100kHz I2C frequency (reduced for better reliability)

// Sensor I2C Addresses
#define MPU6050_I2C_ADDRESS 0x68
#define MAX17048_I2C_ADDRESS 0x36

// ==================== POWER MANAGEMENT ====================

// Battery Configuration
#define BATTERY_MIN_VOLTAGE 3.0     // Minimum safe battery voltage
#define BATTERY_MAX_VOLTAGE 4.2     // Maximum battery voltage (fully charged)
#define BATTERY_NOMINAL_VOLTAGE 3.7 // Nominal voltage
#define BATTERY_EMERGENCY_VOLTAGE 2.8 // Emergency shutdown voltage (well above BMS ~2.5V)
#define BATTERY_CAPACITY_MAH 10000  // Battery capacity in mAh

// ADC Voltage Divider (47kΩ + 22kΩ)
#define VOLTAGE_DIVIDER_RATIO 0.319 // 22/(47+22) = 0.319
#define ADC_RESOLUTION 4095         // 12-bit ADC
#define ADC_REFERENCE_VOLTAGE 3.3   // ESP32 ADC reference

// Auto-dimming levels based on battery percentage
#define BRIGHTNESS_100_PERCENT 255  // 100-75% battery (full brightness)
#define BRIGHTNESS_75_PERCENT  220  // 75-50% battery (increased from 200)
#define BRIGHTNESS_50_PERCENT  180  // 50-25% battery (increased from 150)
#define BRIGHTNESS_25_PERCENT  140  // 25-10% battery (increased from 100)
#define BRIGHTNESS_LOW_BATTERY 80   // <10% battery (increased from 50)

// Power limits
#define MAX_POWER_MW 25000          // 25W maximum power draw
#define POWER_LEVEL_MIN 30          // Minimum brightness for visibility
#define POWER_LEVEL_MAX 255         // Maximum brightness

// ==================== TIMING CONFIGURATION ====================

// Frame rates and timing
#define TARGET_FPS 60               // Target frame rate for LED updates
#define FRAME_TIME_MS (1000/TARGET_FPS)
#define PATTERN_UPDATE_MS 20        // Pattern update interval
#define SENSOR_UPDATE_MS 15         // Sensor reading interval (balanced for responsiveness and stability)
#define BATTERY_UPDATE_MS 5000      // Battery monitoring interval (reduced to 5s to minimize I2C conflicts)

// Button debouncing
#define BUTTON_DEBOUNCE_MS 150      // Button debounce time (increased for stability)
#define BUTTON_LONG_PRESS_MS 2000   // Long press threshold
#define BUTTON_COOLDOWN_MS 500      // Cooldown between pattern changes

// Battery display
#define BATTERY_DISPLAY_DURATION 8000 // Show battery for 8 seconds (increased from 3)

// Low battery automatic warning system  
#define LOW_BATTERY_WARNING_INTERVAL 120000  // Show warning every 2 minutes (120 seconds)
#define LOW_BATTERY_DISPLAY_DURATION 20000   // Show for 20 seconds
#define LOW_BATTERY_THRESHOLD 10.0           // Trigger automatic warnings below 10%
#define STARTUP_GRACE_PERIOD 30000           // 30 second grace period after startup

// ==================== PATTERN CONFIGURATION ====================

// Pattern update speeds
#define PLASMA_SPEED 30
#define FIRE_SPEED 40
#define RAIN_SPEED 25
#define RAINBOW_SPEED 35
#define STARFIELD_SPEED 50
#define RIPPLES_SPEED 30

// Pattern-specific settings
#define MAX_RAINDROPS 32
#define MAX_STARS 40
#define MAX_FIRE_PARTICLES 64

// ==================== WIFI CONFIGURATION ====================

// WiFi Settings
#define WIFI_CONNECT_TIMEOUT_MS 20000 // 20 seconds to connect
#define WIFI_RETRY_DELAY_MS 500

// Access Point Settings (fallback)
#define AP_SSID "LED_Panel_ESP32"
#define AP_PASSWORD "ledpanel123"

// Web Server
#define WEB_SERVER_PORT 80
#define OTA_PASSWORD "ledpanel_ota"

// Replace these with your WiFi credentials
#define HOME_WIFI_SSID "QuantumFiber1776"
#define HOME_WIFI_PASSWORD "ce3ebcef66a6ey"

// ==================== MEMORY CONFIGURATION ====================

// Stack sizes for FreeRTOS tasks
#define LED_TASK_STACK_SIZE 8192
#define SENSOR_TASK_STACK_SIZE 4096
#define WEB_TASK_STACK_SIZE 8192

// Task priorities
#define LED_TASK_PRIORITY 2         // High priority for smooth animation
#define SENSOR_TASK_PRIORITY 1      // Medium priority
#define WEB_TASK_PRIORITY 1         // Medium priority

// ==================== DEBUG CONFIGURATION ====================

// Debug levels
#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_ERROR 1
#define DEBUG_LEVEL_WARN 2
#define DEBUG_LEVEL_INFO 3
#define DEBUG_LEVEL_DEBUG 4

// Set current debug level
#define DEBUG_LEVEL DEBUG_LEVEL_INFO

// Debug macros
#if DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
  #define DEBUG_ERROR(x) Serial.println("[ERROR] " x)
#else
  #define DEBUG_ERROR(x)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_WARN
  #define DEBUG_WARN(x) Serial.println("[WARN] " x)
#else
  #define DEBUG_WARN(x)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
  #define DEBUG_INFO(x) Serial.println("[INFO] " x)
#else
  #define DEBUG_INFO(x)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
  #define DEBUG_DEBUG(x) Serial.println("[DEBUG] " x)
#else
  #define DEBUG_DEBUG(x)
#endif

// ==================== FEATURE FLAGS ====================

// Enable/disable features for testing
#define ENABLE_WIFI 1
#define ENABLE_WEB_SERVER 1
#define ENABLE_OTA_UPDATES 1
#define ENABLE_BATTERY_MONITORING 1
#define ENABLE_AUTO_DIMMING 1
#define ENABLE_DEEP_SLEEP 1
// Game mode removed - now using brightness control

// Performance monitoring
#define ENABLE_PERFORMANCE_MONITORING 1
#define ENABLE_MEMORY_MONITORING 1

// ==================== UTILITY MACROS ====================

// Math utilities
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define MAP_FLOAT(x, in_min, in_max, out_min, out_max) \
  (((x) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))

// LED matrix utilities
#define XY(x, y) (((y) & 0x01) ? (((y) * MATRIX_WIDTH) + (MATRIX_WIDTH - 1 - (x))) : (((y) * MATRIX_WIDTH) + (x)))
#define PIXEL_COUNT NUM_LEDS

// Color utilities
#define BRIGHTNESS_SCALE(color, scale) \
  CRGB(((color).r * (scale)) / 255, ((color).g * (scale)) / 255, ((color).b * (scale)) / 255)



#endif // CONFIG_H 