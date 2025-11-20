/*
 * Battery Manager Module
 * Handles battery monitoring, fuel gauge, and power management
 */

#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include "config.h"

// Battery state variables
extern float batteryVoltage;
extern float batteryPercentage;
extern bool isCharging;
extern bool lowBatteryWarning;

// Function declarations
void initializeBatteryManager();
void updateBatteryManager();

// Public interface functions
float getBatteryVoltage();
float getBatteryPercentage();
bool getChargingStatus();
bool isLowBattery();
bool isCriticalBattery();

// Fuel gauge status
bool isFuelGaugeWorking();

// Battery monitoring
void updateBatteryVoltageADC();
void updateFuelGauge();

// Battery warnings and protection
void checkBatteryWarnings();
void handleLowBatteryWarning();

// Power path management
void enableCharging();
void disableCharging();
bool canOperateWhileCharging();

// Fuel gauge functions
void initializeFuelGauge();
float readFuelGaugeSOC();
float readFuelGaugeVoltage();
uint16_t readFuelGaugeVersion();
uint16_t readFuelGaugeRegister(uint8_t reg);
void writeFuelGaugeRegister(uint8_t reg, uint16_t value);
void resetFuelGaugeHardware();
void quickStartFuelGauge();
void setLowBatteryThreshold(float percentage);
void clearFuelGaugeAlert();
void handleFuelGaugeAlert();

// Charging status
void updateChargingStatus();

// Interrupt service routine
void IRAM_ATTR fuelGaugeAlertISR();

#endif // BATTERY_MANAGER_H 