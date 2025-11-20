# DYNAMIC BATTERY POWERED LED PANEL - COMPONENT LIST (ESP32 EDITION)

## Project Overview
16x16 addressable LED panel with gyroscope tilt effects and button controls, featuring a mazeball game and battery monitoring. Powered by ESP32 with integrated Li-ion battery management.

## Core Components

### 1. LED Display
**WS2812B 16x16 Addressable LED Panel**
- **Pixels:** 256 total (16x16 grid)
- **IC Type:** WS2812B built-in
- **Color Order:** GRB (NOT RGB)
- **Input Voltage:** DC 5V
- **Dimensions:** 160mm × 160mm × 2mm
- **Power Consumption:**
  - Single color: 0.1W per LED (25.6W total)
  - Two colors: 0.2W per LED (51.2W total) 
  - Full white: 0.3W per LED (76.8W total) - NOT RECOMMENDED
  - **Typical Usage:** 15-25W for colorful patterns (auto-dimmed based on battery)
- **Operating Temperature:** -20°C to +50°C

### 2. Computing Platform
**ESP32 Development Board (30-pin)**
- **MCU:** Dual-core Xtensa LX6 @ 240MHz
- **Power Requirements:** 3.3V @ 500mA (1.65W typical)
- **GPIO:** 30 pins available for sensors, LEDs, buttons
- **Connectivity:** WiFi 802.11b/g/n + Bluetooth 4.2
- **Storage:** Flash memory + optional SD card
- **ADC:** Built-in for battery monitoring
- **Advantages:** Much lower power than Pi4, integrated wireless

### 3. Sensors

#### Gyroscope/Accelerometer
**MPU-6050 6-Axis Motion Sensor**
- **Interface:** I2C (SDA/SCL)
- **Power:** 3.3V @ 3.9mA (0.013W)
- **Features:** 3-axis gyroscope + 3-axis accelerometer
- **Use Case:** Mazeball game gravity simulation, tilt effects

### 4. Power System

#### Battery Pack
**10000mAh 1S Li-ion Flat Cell (3.7V)**
- **Configuration:** Single cell (1S) with built-in protection board
- **Nominal Voltage:** 3.7V
- **Capacity:** 10,000mAh (37Wh total energy)
- **Protection:** Built-in BMS with overcurrent/overvoltage protection
- **Connector:** PH2.0 plug
- **Form Factor:** Flat cell design for compact integration
- **Estimated Runtime:** 4-8 hours depending on LED usage and auto-dimming

#### Power Management
**TPS61088 Boost Converter Module**
- **Input:** 3.0V-5.5V (perfect for 1S Li-ion: 3.0-4.2V)
- **Output:** Adjustable 5V, 9V, 12V (set to 5V for LEDs)
- **Max Current:** 10A continuous
- **Switching Frequency:** 1MHz (high efficiency)
- **Special Features:**
  - **PWM Pin:** External PWM control for output enable/disable
  - **ADJ Pins:** Fine voltage adjustment points
  - **Use Case:** PWM pin connects to ESP32 for LED panel power control

#### Battery Monitoring
**Max17048 LiPo Fuel Gauge**
- **Interface:** I2C
- **Accuracy:** ±3% battery percentage
- **Features:** 
  - Real-time battery percentage calculation
  - Low battery alerting
  - Sleep mode for power conservation
- **Integration:** Displays battery level on LED panel corner

#### Charging System
**BQ25606 3A USB-C Charging Module**
- **Input:** USB-C PD 4.5V-12V
- **Charging Current:** Up to 3A
- **Features:**
  - Simultaneous charging and power delivery
  - Integrated protection (thermal, overcurrent, overvoltage)
  - Status indicators for charging state
- **Use Case:** Charges battery while device operates

### 5. User Interface

#### Physical Controls
**Push Buttons (3x)**
- **Button 1:** Pattern/Mode cycling
- **Button 2:** Battery level display toggle
- **Button 3:** Game mode toggle (Mazeball/Patterns)
- **Interface:** GPIO with internal pull-up resistors
- **Debouncing:** Software-based with interrupts

## Power Budget Analysis

| Component | Voltage | Current | Power |
|-----------|---------|---------|-------|
| LED Panel (auto-dimmed) | 5V | 3-5A | 15-25W |
| ESP32 | 3.3V | 150mA | 0.5W |
| MPU-6050 | 3.3V | 3.9mA | 0.013W |
| Max17048 | 3.3V | 23µA | 0.0001W |
| TPS61088 (quiescent) | - | 65µA | 0.0003W |
| **TOTAL TYPICAL** | **Mixed** | **~4A @ 5V** | **15.5-25.5W** |

## Battery Runtime Analysis

**1S Li-ion Specifications:**
- Nominal Voltage: 3.7V
- Capacity: 10,000mAh
- Total Energy: 37Wh
- Usable Capacity: ~9,000mAh (90% depth of discharge)

**Runtime Scenarios:**

| Usage Pattern | Power Draw | Runtime | Auto-Dimming Level |
|---------------|------------|---------|-------------------|
| **Full Battery** | 25W | 1.5 hours | 100% brightness |
| **75% Battery** | 20W | 1.8 hours | 80% brightness |
| **50% Battery** | 15W | 2.4 hours | 60% brightness |
| **25% Battery** | 10W | 3.6 hours | 40% brightness |
| **Low Battery** | 5W | 7.2 hours | 20% brightness |

## Component Integration Details

### TPS61088 Power Control Features
**PWM Pin Control:**
- **Function:** Enable/disable boost converter output
- **ESP32 Connection:** GPIO pin with PWM capability
- **Use Case:** 
  - Complete LED panel shutdown for power saving
  - PWM dimming at hardware level
  - Emergency power conservation

**ADJ Pins:**
- **Function:** Fine voltage adjustment via external resistors
- **Current Use:** Set to 5V for LED panel
- **Future Use:** Could be modified for dynamic voltage control

### Max17048 Battery Display Integration
**LED Battery Indicator:**
- **Location:** Top-right corner (4x4 LED area)
- **Display Modes:**
  - Green: 75-100% battery
  - Yellow: 50-75% battery
  - Orange: 25-50% battery
  - Red: 0-25% battery
  - Blinking Red: <10% critical
- **Trigger:** Button press or automatic low battery warning

### Button Interface Design
**Debounced GPIO Implementation:**
- **Pull-up Resistors:** Internal ESP32 pull-ups enabled
- **Interrupt Driven:** Pin change interrupts for responsive control
- **Long Press Detection:** Different actions for short vs long press
- **Power Efficient:** Wake from deep sleep capability

## Wiring Summary

### Power Distribution
```
Battery (3.7V) → BQ25606 (charging) → TPS61088 (5V boost) → LED Panel
                      ↓
                ESP32 (3.3V via onboard regulator)
                      ↓
                Sensors (3.3V from ESP32)
```

### Communication Interfaces
- **I2C Bus:** ESP32 ↔ MPU-6050 ↔ Max17048
- **SPI/GPIO:** ESP32 → WS2812B LED Panel (single data line)
- **GPIO:** ESP32 ← Buttons (3x with interrupts)
- **PWM:** ESP32 → TPS61088 (power control)

## Safety and Protection Features

### Battery Protection
- **Built-in BMS:** Overcurrent, overvoltage, undervoltage protection
- **Thermal Protection:** In BQ25606 charging module
- **Software Monitoring:** Max17048 provides early warning alerts

### Power Management
- **Auto-dimming:** Reduces power consumption as battery depletes
- **Deep Sleep:** ESP32 can enter low-power mode between operations
- **Hardware Shutdown:** TPS61088 PWM control for complete LED shutdown

## Estimated Total Cost

- LED Panel: $40-60
- ESP32 Development Board: $15-25
- 10000mAh 1S Battery: $25-35
- TPS61088 Boost Module: $15-25
- Max17048 Fuel Gauge: $8-12
- BQ25606 Charging Module: $12-18
- MPU-6050 Sensor: $5-10
- **10kΩ NTC Thermistor:** $2-5
- Buttons and Misc: $10-15
- **Total: $132-205** (Significantly reduced from Pi-based design)

## Advantages of ESP32 Design

### Cost Reduction
- **50-60% lower cost** compared to Raspberry Pi 4 system
- **Integrated wireless** eliminates need for additional modules
- **Lower component count** reduces assembly complexity

### Power Efficiency
- **10x lower base power consumption** (0.5W vs 5W for Pi4)
- **Better battery life** with auto-dimming and sleep modes
- **Single voltage rail** simplifies power management

### Form Factor
- **Compact design** enables smaller enclosure
- **Fewer heat issues** due to lower power consumption
- **Direct GPIO control** eliminates level shifting complexity

### Development Advantages
- **Arduino IDE compatibility** for easier programming
- **Real-time operation** without OS overhead
- **Faster boot times** for immediate operation
- **Built-in wireless** for easy configuration and control