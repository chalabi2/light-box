/*
 * Sensor Manager Module
 * Handles MPU6050 gyroscope/accelerometer sensor operations
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

// Sensor update interval
#define SENSOR_UPDATE_MS 20

// External variables for gravity readings
extern float gravityX;
extern float gravityY;
extern float gravityZ;

// Calibration variables
extern float calibrationOffsetX;
extern float calibrationOffsetY;
extern float calibrationOffsetZ;
extern bool gyroCalibrated;

// Function declarations
void initMPU6050();
void calibrateGyroscope();
void updateGravity();

#endif // SENSOR_MANAGER_H 