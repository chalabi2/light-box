/*
 * Battery Manager Implementation
 * Handles Max17048 fuel gauge, BQ25606 charging, and NTC temperature monitoring
 */

#include "battery_manager.h"
#include "led_control.h"
#include <Wire.h>

// Battery state variables - initialize to reasonable defaults to avoid showing 0% on startup
float batteryVoltage = 3.7;
float batteryPercentage = 75.0;  // Start with reasonable estimate instead of 50% to be safer
bool isCharging = false;
bool lowBatteryWarning = false;

// Internal state
bool fuelGaugeInitialized = false;
unsigned long lastBatteryUpdate = 0;
unsigned long lastAlertCheck = 0;
float temperatureC = 25.0;  // Battery temperature
bool alertPinTriggered = false;



// Alert interrupt flag
volatile bool fuelGaugeAlertFlag = false;

void initializeBatteryManager() {
  DEBUG_INFO("Initializing Battery Manager...");
  
  // Initialize pins per MAX17043 datasheet
  pinMode(FUEL_GAUGE_ALERT_PIN, INPUT_PULLUP);  // ALT pin - active low alert
  
  // QST pin setup per datasheet: has built-in 2.2kΩ pull-down, rising edge triggers reset
  pinMode(FUEL_GAUGE_QST_PIN, OUTPUT);
  digitalWrite(FUEL_GAUGE_QST_PIN, LOW);  // Keep low initially (default state)
  
  // CHARGING_STATUS_PIN not used - using voltage-based detection
  pinMode(BATTERY_ADC_PIN, INPUT);
  
  // Set up fuel gauge alert interrupt
  attachInterrupt(digitalPinToInterrupt(FUEL_GAUGE_ALERT_PIN), fuelGaugeAlertISR, FALLING);
  
  // Initialize fuel gauge
  initializeFuelGauge();
  
  // DON'T call updateBatteryManager() during init - it can overwrite our safe defaults
  // The main loop will call it after a few seconds, allowing the fuel gauge to stabilize
  
  DEBUG_INFO("Battery Manager initialized (no NTC thermistor - battery has built-in protection)");
  DEBUG_INFO("Starting with safe default battery level: " + String(batteryPercentage, 1) + "% (will update from fuel gauge in main loop)");
}

void initializeFuelGauge() {
  DEBUG_INFO("Initializing Max17048 fuel gauge...");
  
  // Give the fuel gauge time to stabilize after power-on (critical for proper readings)
  delay(100);
  
  // Skip quick start to preserve learned battery data
  // quickStartFuelGauge(); // Only use if readings are completely wrong
  
  // Test communication with retries for more robust initialization
  uint16_t version = 0;
  for (int attempt = 0; attempt < 5; attempt++) {
    version = readFuelGaugeVersion();
    if (version != 0 && version != 0xFFFF) {
      break; // Success
    }
    DEBUG_WARN("Fuel gauge communication attempt " + String(attempt + 1) + " failed, retrying...");
    delay(200); // Wait longer between attempts during initialization
  }
  
  if (version != 0 && version != 0xFFFF) {
    fuelGaugeInitialized = true;
    DEBUG_INFO("Max17048 detected (version: 0x" + String(version, HEX) + ")");
    
    // Set alert threshold to 10% (configurable)
    setLowBatteryThreshold(10.0);
    
    // Max17048 preserves learned battery characteristics in internal EEPROM
    // Quick start is ONLY needed if the chip is completely unresponsive or has corrupted data
    // Normal operation should NEVER require quick start - it resets valuable learned data
    
    // Wait a bit more for stable readings
    delay(100);
    
    float initialSOC = readFuelGaugeSOC();
    float initialVoltage = readFuelGaugeVoltage();
    
    // Validate initial readings but DON'T update global variables during init
    if (initialSOC >= 0 && initialSOC <= 100 && initialVoltage > 2.5 && initialVoltage < 5.0) {
      DEBUG_INFO("Fuel gauge test reading: " + String(initialSOC, 1) + "% SOC, " + String(initialVoltage, 2) + "V (not applied yet)");
      DEBUG_INFO("Max17048 fuel gauge initialized successfully - will use readings in main loop");
    } else {
      DEBUG_WARN("Fuel gauge test readings seem invalid: " + String(initialSOC, 1) + "% SOC, " + String(initialVoltage, 2) + "V");
      DEBUG_WARN("Will retry readings during normal operation");
    }
    // NOTE: We deliberately do NOT update batteryPercentage or batteryVoltage here
    // This preserves our safe 75% default until the main loop takes over
  } else {
    DEBUG_ERROR("Failed to initialize Max17048 fuel gauge after 5 attempts");
    fuelGaugeInitialized = false;
  }
}

void updateBatteryManager() {
  if (millis() - lastBatteryUpdate < BATTERY_UPDATE_MS) {
    return;
  }
  
  if (fuelGaugeInitialized) {
    updateFuelGauge();
    // Periodic status reporting every 60 seconds (reduced from 30 to reduce I2C traffic)
    static unsigned long lastStatusReport = 0;
    if (millis() - lastStatusReport > 60000) {
      DEBUG_INFO("Battery Status: " + String(batteryPercentage, 1) + "% (" + String(batteryVoltage, 2) + "V) - Fuel Gauge");
      lastStatusReport = millis();
    }
  } else {
    updateBatteryVoltageADC();
    // Periodic status reporting every 60 seconds
    static unsigned long lastADCStatusReport = 0;
    if (millis() - lastADCStatusReport > 60000) {
      DEBUG_WARN("Battery Status: " + String(batteryPercentage, 1) + "% (" + String(batteryVoltage, 2) + "V) - ADC FALLBACK");
      lastADCStatusReport = millis();
    }
  }
  
  // Update charging status as part of battery management
  updateChargingStatus();
  
  checkBatteryWarnings();
  lastBatteryUpdate = millis();
}

void updateFuelGauge() {
  // Read battery percentage from Max17048
  float newSOC = readFuelGaugeSOC();
  
  // Read battery voltage from Max17048
  float newVoltage = readFuelGaugeVoltage();
  
  // Handle alert flag
  if (fuelGaugeAlertFlag) {
    handleFuelGaugeAlert();
    fuelGaugeAlertFlag = false;
  }
  
  // Validate readings - be very strict during startup to prevent garbage readings
  static int invalidReadingCount = 0;
  static int validReadingCount = 0;
  bool isStartupPeriod = (millis() < 30000); // First 30 seconds after boot
  
  // During startup, be MUCH more strict - reject readings that seem like garbage
  bool isValidReading = false;
  if (isStartupPeriod) {
    // Startup: Reject readings below 10% or above 95% as likely garbage
    isValidReading = (newSOC >= 10.0 && newSOC <= 95.0 && newVoltage > 3.0 && newVoltage < 4.5);
    if (!isValidReading) {
      DEBUG_WARN("Startup: Rejecting suspicious reading: " + String(newSOC, 1) + "% SOC, " + String(newVoltage, 2) + "V (keeping " + String(batteryPercentage, 1) + "%)");
    }
  } else {
    // Normal operation: More permissive validation
    isValidReading = (newSOC >= 0 && newSOC <= 100 && newVoltage > 2.5 && newVoltage < 5.0);
  }
  
  if (isValidReading) {
    // Valid readings - update battery state
    batteryPercentage = newSOC;
    batteryVoltage = newVoltage;
    invalidReadingCount = 0; // Reset invalid counter on successful reading
    validReadingCount++;
    
    if (isStartupPeriod && validReadingCount == 1) {
      DEBUG_INFO("First valid fuel gauge reading accepted: " + String(newSOC, 1) + "% SOC, " + String(newVoltage, 2) + "V");
    }
    
    // If fuel gauge was previously failed but now working, re-enable it
    static bool recoveryMessageShown = false;
    if (!fuelGaugeInitialized && !recoveryMessageShown) {
      DEBUG_INFO("Fuel gauge communication recovered - resuming fuel gauge readings");
      fuelGaugeInitialized = true;
      recoveryMessageShown = true;
    }
  } else {
    // Invalid readings - don't update battery state, but don't immediately fail over to ADC
    DEBUG_WARN("Invalid fuel gauge readings: " + String(newSOC, 1) + "% SOC, " + String(newVoltage, 2) + "V - keeping previous values");
    
    invalidReadingCount++;
    
    // During startup period with consistently very low readings, the fuel gauge might have corrupted data
    if (isStartupPeriod && invalidReadingCount > 3 && newSOC < 5.0 && newVoltage > 3.0) {
      DEBUG_ERROR("Fuel gauge consistently reporting very low SOC (" + String(newSOC, 1) + "%) but voltage seems normal (" + String(newVoltage, 2) + "V)");
      DEBUG_ERROR("This suggests corrupted fuel gauge learning data. Consider adding manual reset capability.");
      DEBUG_ERROR("For now, continuing with safe default until readings stabilize.");
      invalidReadingCount = 0; // Reset to prevent spam
    }
    
    // Only fall back to ADC after multiple consecutive invalid readings
    if (invalidReadingCount > 5) {
      DEBUG_WARN("Multiple invalid fuel gauge readings - using ADC fallback temporarily");
      updateBatteryVoltageADC();
      invalidReadingCount = 0; // Reset counter
    }
  }
}

void updateBatteryVoltageADC() {
  // Read battery voltage via ADC with voltage divider
  int adcReading = analogRead(BATTERY_ADC_PIN);
  float adcVoltage = (adcReading * ADC_REFERENCE_VOLTAGE) / ADC_RESOLUTION;
  batteryVoltage = adcVoltage / VOLTAGE_DIVIDER_RATIO;
  
  // Calculate percentage from voltage (rough estimate)
  batteryPercentage = MAP_FLOAT(batteryVoltage, 
                                BATTERY_MIN_VOLTAGE, 
                                BATTERY_MAX_VOLTAGE, 
                                0.0, 100.0);
  batteryPercentage = CLAMP(batteryPercentage, 0.0, 100.0);
}

void updateChargingStatus() {
  static float voltageHistory[5] = {0};
  static int historyIndex = 0;
  static unsigned long lastChargingUpdate = 0;
  
  // Only update charging status every 2 seconds for stability
  if (millis() - lastChargingUpdate < 2000) {
    return;
  }
  
  // Store voltage history for trend analysis
  voltageHistory[historyIndex] = batteryVoltage;
  historyIndex = (historyIndex + 1) % 5;
  
  // Calculate voltage trend (is it increasing?)
  float avgOld = (voltageHistory[0] + voltageHistory[1]) / 2.0;
  float avgNew = (voltageHistory[3] + voltageHistory[4]) / 2.0;
  bool voltageRising = (avgNew > avgOld + 0.02); // 20mV trend increase
  
  // Improved charging detection logic
  bool definitelyCharging = (batteryVoltage > 4.15); // High voltage = definitely charging
  bool likelyCharging = (batteryVoltage > 4.05 && voltageRising); // Voltage rising = starting to charge
  bool lowVoltage = (batteryVoltage < 3.9); // Clearly not charging
  
  // State machine approach: easier to start charging, harder to stop
  if (definitelyCharging || likelyCharging) {
    if (!isCharging) {
      DEBUG_INFO("Charging detected - voltage: " + String(batteryVoltage, 2) + "V");
    }
    isCharging = true;
  } else if (lowVoltage) {
    // Only stop charging detection if voltage drops significantly
    if (isCharging) {
      DEBUG_INFO("Charging stopped - voltage: " + String(batteryVoltage, 2) + "V");
    }
    isCharging = false;
  }
  // If voltage is between 3.9V and 4.05V, maintain current state (hysteresis)
  
  lastChargingUpdate = millis();
}

void checkBatteryWarnings() {
  // Don't trigger low battery warnings when charging
  if (isCharging) {
    lowBatteryWarning = false;
    return;
  }
  
  // Check for low battery conditions (only when not charging)
  bool wasLowBattery = lowBatteryWarning;
  lowBatteryWarning = (batteryPercentage < 15.0);
  
  // Alert on new low battery condition
  if (lowBatteryWarning && !wasLowBattery) {
    handleLowBatteryWarning();
  }
  
  // Check for critical battery (emergency shutdown) - only when not charging
  if (batteryVoltage < BATTERY_EMERGENCY_VOLTAGE) {
    DEBUG_ERROR("Critical battery voltage detected!");
    // Emergency shutdown handled in main loop
  }
}

void handleLowBatteryWarning() {
  DEBUG_WARN("Low battery warning: " + String(batteryPercentage, 1) + "%");
  
  // Just log the warning - visual indication now handled by button-triggered display
  // This prevents constant battery overlay but still notifies via serial
}

void handleFuelGaugeAlert() {
  DEBUG_WARN("Fuel gauge alert triggered!");
  
  // Read alert status register
  uint16_t alertFlags = readFuelGaugeRegister(0x1A); // STATUS register
  
  if (alertFlags & 0x0020) { // SOC change alert
    DEBUG_INFO("Battery SOC changed");
  }
  
  if (alertFlags & 0x0010) { // Low SOC alert
    DEBUG_WARN("Low battery SOC alert");
    lowBatteryWarning = true;
  }
  
  // Clear alert flags
  clearFuelGaugeAlert();
}

// Fuel gauge I2C communication functions
float readFuelGaugeSOC() {
  uint16_t soc = readFuelGaugeRegister(0x04); // SOC register
  
  // Check for I2C communication error
  if (soc == 0xFFFF) {
    DEBUG_WARN("Fuel gauge SOC read failed (I2C error)");
    return -1.0; // Return invalid value to trigger fallback
  }
  
  return (float)soc / 256.0; // Convert to percentage
}

float readFuelGaugeVoltage() {
  uint16_t voltage = readFuelGaugeRegister(0x02); // VCELL register
  
  // Check for I2C communication error
  if (voltage == 0xFFFF) {
    DEBUG_WARN("Fuel gauge voltage read failed (I2C error)");
    return -1.0; // Return invalid value to trigger fallback
  }
  
  return (float)voltage * 78.125 / 1000000.0; // Convert to volts
}

uint16_t readFuelGaugeVersion() {
  return readFuelGaugeRegister(0x08); // VERSION register
}

uint16_t readFuelGaugeRegister(uint8_t reg) {
  static unsigned long lastI2CAccess = 0;
  static int consecutiveErrors = 0;
  
  // Signal to gyroscope to pause during fuel gauge operations
  extern volatile bool pauseGyroscopeReads;
  pauseGyroscopeReads = true;
  
  // Increase spacing between I2C operations to prevent NACK errors and bus conflicts
  // Fuel gauge needs more time due to shared I2C bus with gyroscope
  if (millis() - lastI2CAccess < 100) { // Increased from 50ms to 100ms for better reliability
    delay(100 - (millis() - lastI2CAccess));
  }
  
  // Retry mechanism for I2C communication
  for (int attempt = 0; attempt < 3; attempt++) {
    Wire.beginTransmission(MAX17048_I2C_ADDRESS);
    Wire.write(reg);
    uint8_t error = Wire.endTransmission(false);
    
    if (error != 0) {
      consecutiveErrors++;
      DEBUG_WARN("Fuel gauge I2C error " + String(error) + " on attempt " + String(attempt + 1) + "/3 (consecutive errors: " + String(consecutiveErrors) + ")");
      
      if (attempt == 2) { // Last attempt
        DEBUG_ERROR("Fuel gauge I2C failed after 3 attempts - this suggests I2C bus contention with gyroscope");
        
        // If too many consecutive errors, fall back to ADC
        if (consecutiveErrors > 10) {
          DEBUG_ERROR("Too many fuel gauge errors (" + String(consecutiveErrors) + ") - switching to ADC fallback");
          fuelGaugeInitialized = false;
        }
        
        lastI2CAccess = millis();
        return 0xFFFF;
      }
      delay(50); // Increased delay before retry to reduce I2C bus conflicts
      continue;
    }
    
    // Request data with timeout
    Wire.requestFrom(MAX17048_I2C_ADDRESS, 2);
    unsigned long timeout = millis() + 100; // 100ms timeout
    while (Wire.available() < 2 && millis() < timeout) {
      delay(1);
    }
    
    if (Wire.available() >= 2) {
      uint16_t result = (Wire.read() << 8) | Wire.read();
      consecutiveErrors = 0; // Reset error count on success
      lastI2CAccess = millis();
      pauseGyroscopeReads = false; // Re-enable gyroscope reads on success
      return result;
    }
    
    if (attempt == 2) {
      DEBUG_WARN("Fuel gauge I2C timeout after 3 attempts");
    }
    delay(25); // Increased delay before retry for I2C stability
  }
  
  lastI2CAccess = millis();
  pauseGyroscopeReads = false; // Re-enable gyroscope reads
  return 0xFFFF; // Return error value after all retries failed
}

void writeFuelGaugeRegister(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(MAX17048_I2C_ADDRESS);
  Wire.write(reg);
  Wire.write((value >> 8) & 0xFF);
  Wire.write(value & 0xFF);
  Wire.endTransmission();
}

void resetFuelGaugeHardware() {
  DEBUG_INFO("Performing hardware reset of MAX17043 via QST pin...");
  
  // Per datasheet: QST pin rising edge triggers hardware reset
  // QST has built-in 2.2kΩ pull-down, so we create rising edge
  digitalWrite(FUEL_GAUGE_QST_PIN, LOW);  // Ensure low state
  delay(10);
  digitalWrite(FUEL_GAUGE_QST_PIN, HIGH); // Rising edge triggers reset
  delay(100);  // Hold high briefly
  digitalWrite(FUEL_GAUGE_QST_PIN, LOW);  // Return to default low state
  delay(200);  // Allow reset to complete
  
  DEBUG_INFO("Hardware reset complete - fuel gauge should be in default state");
}

void quickStartFuelGauge() {
  DEBUG_WARN("Quick start called - this resets learned battery characteristics!");
  
  // Method 1: Hardware reset via QST pin (preferred)
  resetFuelGaugeHardware();
  
  // Method 2: Software quick start via MODE register (if hardware reset fails)
  delay(100);
  writeFuelGaugeRegister(0x06, 0x4000); // Quick start bit in MODE register
  delay(500);
  
  DEBUG_WARN("Quick start complete - all learned battery data has been reset");
}

void setLowBatteryThreshold(float percentage) {
  // Convert percentage to register value
  uint16_t threshold = (uint16_t)(percentage * 256.0 / 100.0);
  
  // Set RCOMP and alert threshold
  uint16_t config = 0x9700 | (threshold & 0x1F); // RCOMP=0x97, threshold in lower bits
  writeFuelGaugeRegister(0x0C, config); // CONFIG register
  
  DEBUG_INFO("Low battery threshold set to " + String(percentage, 1) + "%");
}

void clearFuelGaugeAlert() {
  // Clear alert by reading STATUS register
  readFuelGaugeRegister(0x1A);
}

// Interrupt service routine
void IRAM_ATTR fuelGaugeAlertISR() {
  fuelGaugeAlertFlag = true;
}

// Public interface functions
float getBatteryVoltage() {
  return batteryVoltage;
}

float getBatteryPercentage() {
  return batteryPercentage;
}

bool getChargingStatus() {
  return isCharging;
}

bool isLowBattery() {
  return lowBatteryWarning;
}

bool isCriticalBattery() {
  return (batteryVoltage < BATTERY_EMERGENCY_VOLTAGE);
}

void enableCharging() {
  // BQ25606 handles charging automatically
  // This could control an external charging enable pin if available
  DEBUG_INFO("Charging enabled (automatic via BQ25606)");
}

void disableCharging() {
  // Emergency charging disable (would need additional hardware)
  DEBUG_WARN("Charging disable requested - requires hardware modification");
}

bool canOperateWhileCharging() {
  // BQ25606 supports power path management
  return true;
}

bool isFuelGaugeWorking() {
  return fuelGaugeInitialized;
} 