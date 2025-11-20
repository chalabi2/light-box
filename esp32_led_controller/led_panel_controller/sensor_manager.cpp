/*
 * Sensor Manager Module Implementation
 * Handles MPU6050 gyroscope/accelerometer sensor operations
 */

#include "sensor_manager.h"

// Hardware definitions
#ifndef MPU6050_I2C_ADDRESS
#define MPU6050_I2C_ADDRESS 0x68
#endif

// Gravity vector components
float gravityX = 0.0;
float gravityY = 1.0;
float gravityZ = 0.0;

// Gyroscope calibration offsets
float calibrationOffsetX = 0.0;
float calibrationOffsetY = 0.0;
float calibrationOffsetZ = 0.0;
bool gyroCalibrated = false;

// I2C bus coordination with fuel gauge
volatile bool pauseGyroscopeReads = false;

void initMPU6050() {
  Serial.println("🔧 Initializing MPU6050...");
  
  // Test communication first
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  uint8_t error = Wire.endTransmission();
  if (error != 0) {
    Serial.printf("❌ MPU6050 not found at address 0x%02X (error %d)\n", MPU6050_I2C_ADDRESS, error);
    return;
  }
  
  // Wake up the MPU6050
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  delay(50);
  
  // Set accelerometer range to ±2g for better sensitivity
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  Wire.write(0x1C);  // ACCEL_CONFIG register
  Wire.write(0x00);  // ±2g range
  Wire.endTransmission(true);
  delay(10);
  
  // Set gyroscope range to ±250°/s
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  Wire.write(0x1B);  // GYRO_CONFIG register
  Wire.write(0x00);  // ±250°/s range
  Wire.endTransmission(true);
  delay(10);
  
  Serial.println("✅ MPU6050 initialized successfully");
  delay(100);
  
  // Auto-calibrate gyroscope with timeout protection
  Serial.println("🎯 Starting gyroscope calibration...");
  calibrateGyroscope();
}

void calibrateGyroscope() {
  Serial.println("Calibrating gyroscope... Keep panel flat and still!");
  
  float sumX = 0, sumY = 0, sumZ = 0;
  int samples = 50; // Reduced from 100 to speed up calibration
  int validSamples = 0;
  
  for (int i = 0; i < samples && validSamples < samples; i++) {
    // Add error checking to prevent hanging
    Wire.beginTransmission(MPU6050_I2C_ADDRESS);
    Wire.write(0x3B);
    uint8_t error = Wire.endTransmission(false);
    
    if (error != 0) {
      Serial.printf("Calibration I2C error %d, retrying...\n", error);
      delay(100);
      continue;
    }
    
    Wire.requestFrom(MPU6050_I2C_ADDRESS, 6);
    
    // Check if data is available
    if (Wire.available() < 6) {
      Serial.println("Insufficient data during calibration, retrying...");
      delay(50);
      continue;
    }
    
    int16_t AcX = Wire.read() << 8 | Wire.read();
    int16_t AcY = Wire.read() << 8 | Wire.read();
    int16_t AcZ = Wire.read() << 8 | Wire.read();
    
    // Convert to g-force (±2g range, 16-bit)
    float accelX = (float)AcX / 16384.0;
    float accelY = (float)AcY / 16384.0;
    float accelZ = (float)AcZ / 16384.0;
    
    // Sanity check readings
    if (abs(accelX) > 3.0 || abs(accelY) > 3.0 || abs(accelZ) > 3.0) {
      Serial.println("Invalid reading during calibration, skipping...");
      continue;
    }
    
    sumX += accelX;
    sumY += accelY;
    sumZ += accelZ;
    validSamples++;
    
    // Progress indicator
    if (validSamples % 10 == 0) {
      Serial.printf("Calibration progress: %d/%d\n", validSamples, samples);
    }
    
    delay(20); // Increased delay for stability
  }
  
  if (validSamples >= 10) { // Require at least 10 valid samples
    // Calculate offsets (assuming panel is flat, Z should be ~1g, X&Y should be ~0)
    calibrationOffsetX = sumX / validSamples;
    calibrationOffsetY = sumY / validSamples;
    calibrationOffsetZ = (sumZ / validSamples) - 1.0; // Subtract expected 1g
    
    gyroCalibrated = true;
    
    Serial.printf("✅ Gyroscope calibrated! (%d samples) Offsets: X=%.3f, Y=%.3f, Z=%.3f\n", 
                  validSamples, calibrationOffsetX, calibrationOffsetY, calibrationOffsetZ);
  } else {
    Serial.printf("❌ Calibration failed - only %d valid samples. Using defaults.\n", validSamples);
    calibrationOffsetX = 0.0;
    calibrationOffsetY = 0.0;
    calibrationOffsetZ = 0.0;
    gyroCalibrated = false;
  }
}

void updateGravity() {
  static unsigned long lastSensorUpdate = 0;
  static int consecutiveErrors = 0;  // Track I2C errors
  
  // Check if fuel gauge has requested I2C pause
  if (pauseGyroscopeReads) {
    return; // Skip this update - fuel gauge is using I2C bus
  }
  
  // Reduce update frequency significantly to prevent I2C bus conflicts
  if (millis() - lastSensorUpdate < 50) { // 20 FPS for stability and reduced I2C traffic
    return;
  }
  
  // Add staggered timing to avoid collision with fuel gauge reads
  // Fuel gauge reads every 5000ms now, so stagger gyro reads
  unsigned long currentTime = millis();
  if ((currentTime % 5000) < 200) { // Avoid first 200ms of every 5-second cycle
    return;
  }
  
  // Read accelerometer data from MPU6050 with error handling
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  Wire.write(0x3B); // Starting register for accelerometer data
  uint8_t error = Wire.endTransmission(false);
  
  if (error != 0) {
    consecutiveErrors++;
    if (consecutiveErrors > 5) { // Increased threshold to prevent constant reinit
      Serial.printf("⚠️ MPU6050 I2C error %d (attempts: %d) - backing off\n", error, consecutiveErrors);
      lastSensorUpdate = millis() + 500; // Back off for 500ms to let I2C bus settle
      consecutiveErrors = 0;
    }
    lastSensorUpdate = millis(); // Update timestamp to prevent rapid retries
    return;  // Skip this update
  }
  
  Wire.requestFrom(MPU6050_I2C_ADDRESS, 6);
  if (Wire.available() < 6) {
    consecutiveErrors++;
    return;  // Skip if insufficient data
  }
  
  consecutiveErrors = 0;  // Reset error count on success
  
  int16_t AcX = Wire.read() << 8 | Wire.read();
  int16_t AcY = Wire.read() << 8 | Wire.read();
  int16_t AcZ = Wire.read() << 8 | Wire.read();
  
  // Apply calibration offsets and convert to g-force (±2g range = 16384 LSB/g)
  float accelX = ((float)AcX / 16384.0) - calibrationOffsetX;
  float accelY = ((float)AcY / 16384.0) - calibrationOffsetY;
  float accelZ = ((float)AcZ / 16384.0) - calibrationOffsetZ;
  
  // Normalize raw readings first to prevent scaling issues
  float magnitude = sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
  if (magnitude > 0.1) {
    accelX /= magnitude;
    accelY /= magnitude;
    accelZ /= magnitude;
  }
  
  // Apply gentle smoothing using complementary filter to reduce jitter
  static float filteredX = 0.0, filteredY = 0.0;
  static bool firstReading = true;
  
  if (firstReading) {
    filteredX = accelX;
    filteredY = accelY;
    firstReading = false;
  } else {
    // Reduce filter strength to prevent jumpiness
    float filterStrength = 0.15; // Much gentler than 0.25
    filteredX = filterStrength * accelX + (1.0 - filterStrength) * filteredX;
    filteredY = filterStrength * accelY + (1.0 - filterStrength) * filteredY;
  }
  
  // Map to screen coordinates with deadzone to prevent micro-movements
  float deadzone = 0.05; // Small deadzone for stability
  float mappedX = (abs(filteredY) > deadzone) ? filteredY * 0.8 : 0.0; // Flipped for upside-down gyroscope mount
  float mappedY = (abs(filteredX) > deadzone) ? filteredX * 0.8 : 0.0;
  
  // Final gravity values
  gravityX = constrain(mappedX, -1.0, 1.0);
  gravityY = constrain(mappedY, -1.0, 1.0);
  
  lastSensorUpdate = millis();  // Update timing
} 