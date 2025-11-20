# LED PANEL WIRING DIAGRAM - ESP32 EDITION (Updated Pin Details)

## Overview
Complete wiring guide for ESP32-based 16x16 LED panel with battery management, charging, and sensor integration. **Updated with specific component pin details.**

## Component List
- ESP32 Development Board (30-pin)
- WS2812B 16x16 LED Panel
- 10000mAh 1S Li-ion Battery (3.7V)
- TPS61088 Boost Converter Module
- BQ25606 USB-C Charging Module **with NTC pads**
- Max17048 Battery Fuel Gauge **with specific pinout**
- MPU-6050 Gyroscope/Accelerometer
- 3x Push Buttons
- **10kΩ NTC Thermistor** (for battery temperature monitoring)
- Miscellaneous: resistors, capacitors, wires

## Power Distribution Architecture

```
┌─────────────────┐    ┌──────────────┐    ┌─────────────────┐
│   USB-C PD      │    │  BQ25606     │    │   10000mAh      │
│   (5V-12V)      │───▶│  Charging    │◄──▶│   1S Battery    │
│                 │    │  Module      │    │   (3.7V)        │
└─────────────────┘    │  +NTC sensor │    │   +NTC probe    │
                       └──────────────┘    └─────────────────┘
                                │                     │
                                │                     ▼
                                │           ┌─────────────────┐
                                │           │   Max17048      │
                                │           │   Fuel Gauge    │
                                │           │   +ALT pin      │
                                │           └─────────────────┘
                                ▼                     │
┌─────────────────┐    ┌──────────────┐    ┌─────────────────┐
│   WS2812B       │◄───│   TPS61088   │◄───│   Battery       │
│   LED Panel     │    │   Boost to   │    │   Output        │
│   (5V)          │    │   5V         │    │   (3.0-4.2V)    │
└─────────────────┘    └──────────────┘    └─────────────────┘
                              │
                              ▼ (3.7V)
                    ┌─────────────────┐
                    │     ESP32       │
                    │   (3.3V reg)    │
                    └─────────────────┘
                              │ (3.3V)
                              ▼
                    ┌─────────────────┐
                    │   Sensors &     │
                    │   Peripherals   │
                    └─────────────────┘
```

## ESP32 Pinout Configuration

### Power Connections
| ESP32 Pin | Connection | Notes |
|-----------|------------|-------|
| VIN (5V) | BQ25606 V+ (System Output) | **Updated:** Use system output, not battery direct |
| 3V3 | Sensor VCC | 3.3V regulated output |
| GND | Common Ground | Multiple ground connections |

### LED Panel
| ESP32 Pin | WS2812B Pin | Function |
|-----------|-------------|----------|
| GPIO16 | DIN | Data input for LED control |
| VIN/5V | VCC | **5V from BQ25606 V+ or TPS61088** |
| GND | GND | Ground connection |

### I2C Bus (Sensors)
| ESP32 Pin | Function | Connected To |
|-----------|----------|--------------|
| GPIO21 | SDA | MPU-6050 SDA, **Max17048 SDA** |
| GPIO22 | SCL | MPU-6050 SCL, **Max17048 SCL** |
| 3V3 | VCC | Sensor power |
| GND | GND | Sensor ground |

### **NEW: Alert and Status Pins**
| ESP32 Pin | Function | Connected To |
|-----------|----------|--------------|
| GPIO27 | **Fuel Gauge Alert** | **Max17048 ALT pin** |
| GPIO32 | **Quick Start Control** | **Max17048 QST pin** |
| GPIO33 | **Charging Status** | **BQ25606 status indicator** (optional) |

### Button Controls
| ESP32 Pin | Function | Button |
|-----------|----------|--------|
| GPIO18 | Pattern Cycle | Button 1 (with pull-up) |
| GPIO19 | Battery Display | Button 2 (with pull-up) |
| GPIO23 | Game Mode Toggle | Button 3 (with pull-up) |

### Power Control
| ESP32 Pin | Function | Connected To |
|-----------|----------|--------------|
| GPIO25 | PWM Control | TPS61088 PWM Pin |
| GPIO26 | Battery Voltage ADC | **Voltage divider from BQ25606 B+** |

## Detailed Component Wiring

### 1. **Max17048 Fuel Gauge (Updated Pinout)**

```
Max17048 Module Connections:
┌─────────────────────────────────┐
│  Max17048 Fuel Gauge Module    │
│                                 │
│  Battery Wire ──► Battery Pack  │
│  + pin ─────────► Battery +     │  (Redundant connection)
│  - pin ─────────► Battery -     │  (Redundant connection)
│                                 │
│  VCC ───────────► ESP32 3.3V    │
│  GND ───────────► ESP32 GND     │
│  SDA ───────────► ESP32 GPIO21  │
│  SCL ───────────► ESP32 GPIO22  │
│  ALT ───────────► ESP32 GPIO27  │  ⚠️ LOW BATTERY INTERRUPT
│  QST ───────────► ESP32 GPIO32  │  🔄 QUICK START CONTROL
└─────────────────────────────────┘

I2C Address: 0x36
Alert Function: Pulls LOW when battery < threshold
Quick Start: Pull LOW to force recalibration
```

### 2. **BQ25606 USB-C Charging Module (Updated Pinout)**

```
BQ25606 Charging Module Connections:
┌─────────────────────────────────────────┐
│         BQ25606 Charging Module         │
│                                         │
│  USB-C ─────────► Power Input (5V-12V)  │
│                                         │
│  B+ ────────────► Battery + Terminal    │  🔋 BATTERY CONNECTION
│  B- ────────────► Battery - Terminal    │
│                                         │
│  V+ ────────────► System Power Out      │  ⚡ POWERS ESP32 & SYSTEM
│  V- ────────────► System Ground         │
│                                         │
│  U+ ────────────► USB Voltage Monitor   │  📊 (Optional monitoring)
│  U- ────────────► USB Ground Reference  │
│                                         │
│  NTC Pad 1 ─────► 10kΩ NTC Thermistor  │  🌡️ TEMPERATURE MONITORING
│  NTC Pad 2 ─────► 10kΩ NTC Thermistor  │  🚨 CRITICAL FOR SAFETY!
└─────────────────────────────────────────┘

⚠️ IMPORTANT: NTC thermistor must be physically attached to battery!
```

### 3. **NTC Temperature Monitoring (CRITICAL SAFETY FEATURE)**

#### **NTC Thermistor Installation:**
```
┌─────────────────┐     ┌──────────────────┐
│   10kΩ NTC      │     │    Li-ion        │
│   Thermistor    │────▶│    Battery       │
│                 │     │    (taped to     │
│   Temp Sensor   │     │     battery)     │
└─────────────────┘     └──────────────────┘
        │
        ▼
┌─────────────────┐
│  BQ25606 NTC    │
│  Pads (Pre-     │
│  soldered)      │
└─────────────────┘

Connection: NTC Thermistor between BQ25606 NTC pads
Placement: Thermistor must be physically against battery
Function: Automatic charge control based on battery temperature
```

#### **Temperature Safety Thresholds:**
- **Normal Charging:** 0°C to 45°C
- **Reduced Current:** 45°C to 55°C  
- **Charging Disabled:** >55°C or <0°C
- **Emergency Alert:** >60°C

### 4. **Power System Integration (Updated)**

#### **Power Path Priority:**
```
USB-C Connected (Charging):
USB-C → BQ25606 → V+/V- → ESP32 & System
                → B+/B- → Battery (charging)

USB-C Disconnected (Battery Operation):
Battery → BQ25606 → V+/V- → ESP32 & System
                 ↘ (pass-through mode)
```

#### **System Power Connections:**
```
BQ25606 V+ ──┬── ESP32 VIN
             │
             ├── TPS61088 VIN+ (for LED boost)
             │
             └── System 3.7V Rail

BQ25606 V- ──── Common Ground (ESP32 GND, TPS61088 GND)
```

### 5. **Updated I2C Bus with Alert Pins**

```
I2C Bus (GPIO21/GPIO22):
┌─────────────┐    ┌─────────────┐
│   MPU6050   │    │  Max17048   │
│  (0x68)     │    │  (0x36)     │
│             │    │             │
│ VCC ────────┼────┼─── 3.3V     │
│ GND ────────┼────┼─── GND      │
│ SDA ────────┼────┼─── GPIO21   │
│ SCL ────────┼────┼─── GPIO22   │
└─────────────┘    │             │
                   │ ALT ────────┼─── GPIO27 (Interrupt)
                   │ QST ────────┼─── GPIO32 (Control)
                   └─────────────┘

Pull-up Resistors: 4.7kΩ on SDA/SCL (usually built into ESP32 board)
Alert Pin: Active LOW interrupt for low battery warning
```

## Complete Wiring Schematic (Updated)

```
                    ┌─────────────────────────────────────────────┐
                    │                ESP32                        │
                    │                                             │
     ┌──────────────┤ VIN                                   GPIO25├─── TPS61088 PWM
     │              │                                             │
     │       ┌──────┤ GND                                   GPIO26├─── Battery ADC
     │       │      │                                             │
     │       │   ┌──┤ 3V3                                   GPIO16├─── LED Data
     │       │   │  │                                             │
     │       │   │  │                                      GPIO21├─── SDA Bus
     │       │   │  │                                             │
     │       │   │  │                                      GPIO22├─── SCL Bus
     │       │   │  │                                             │
     │       │   │  │                                      GPIO27├─── Fuel ALT
     │       │   │  │                                             │
     │       │   │  │                                      GPIO32├─── Fuel QST
     │       │   │  │                                             │
     │       │   │  │                                      GPIO18├─── Button 1
     │       │   │  │                                             │
     │       │   │  │                                      GPIO19├─── Button 2
     │       │   │  │                                             │
     │       │   │  │                                      GPIO23├─── Button 3
     │       │   │  └─────────────────────────────────────────────┘
     │       │   │
     │       │   │  ┌─────────┐    ┌─────────┐
     │       │   └──┤ MPU6050 │    │Max17048 │
     │       │      │ VCC SDA │    │VCC SDA  │
     │       └──────┤ GND SCL │    │GND SCL  │
     │              └─────────┘    │ALT QST  │──── GPIO27/32
     │                             └─────────┘
     │                                  │
     │         ┌─────────────────────────┴─────────────┐
     │         │              Battery                  │
     │         │            Monitoring                 │
┌────▼────┐    │         ┌─────────────┐              │
│ BQ25606 │◄───┼─────────┤  10kΩ NTC   │              │
│Charging │    │         │ Thermistor  │              │
│ Module  │    │         │(on battery) │              │
│         │    │         └─────────────┘              │
│ V+ V-   │────┘                │                     │
│ B+ B-   │◄─────────────────────┼─────────────────────┘
│ NTC Pads│◄─────────────────────┘
│ U+ U-   │
└─────────┘
     ▲
USB-C PD Input
(5V-12V)
     │
┌────▼────┐     ┌─────────────┐
│ TPS61088│────▶│  WS2812B    │
│ Boost to│     │  LED Panel  │
│   5V    │     │   (16x16)   │
└─────────┘     └─────────────┘
```

## Safety and Protection Features (Updated)

### 1. **Enhanced Temperature Monitoring**
- **NTC Thermistor:** Real-time battery temperature monitoring
- **Automatic Protection:** BQ25606 reduces/stops charging if battery overheats
- **ESP32 Monitoring:** Can read charging status and take additional protective action

### 2. **Fuel Gauge Alert System**
- **Hardware Interrupt:** Max17048 ALT pin provides immediate low battery warning
- **Quick Start:** QST pin allows forced recalibration after battery replacement
- **Accurate Monitoring:** ±3% accuracy for precise battery management

### 3. **Comprehensive Protection Chain**
```
Level 1: Battery Internal BMS (overcurrent, overvoltage)
Level 2: NTC Temperature Protection (thermal)
Level 3: BQ25606 Charging Protection (thermal, overcurrent)
Level 4: Max17048 Low Battery Alert (undervoltage warning)
Level 5: ESP32 Software Protection (emergency shutdown)
```

## **Updated Pin Assignment Table**

| Component | Pin | ESP32 GPIO | Function | Notes |
|-----------|-----|------------|----------|-------|
| **Max17048** | VCC | 3V3 | Power | 3.3V regulated |
| | GND | GND | Ground | Common ground |
| | SDA | GPIO21 | I2C Data | Shared with MPU6050 |
| | SCL | GPIO22 | I2C Clock | Shared with MPU6050 |
| | **ALT** | **GPIO27** | **Alert Interrupt** | **Low battery warning** |
| | **QST** | **GPIO32** | **Quick Start** | **Force recalibration** |
| **BQ25606** | V+ | VIN | System Power | **Powers ESP32** |
| | V- | GND | System Ground | Common ground |
| | B+ | Battery+ | Battery Positive | Via monitoring divider |
| | B- | Battery- | Battery Negative | Direct connection |
| | **NTC** | **10kΩ NTC** | **Temperature** | **On battery surface** |
| **TPS61088** | PWM | GPIO25 | Enable Control | LED power on/off |
| **Buttons** | BTN1 | GPIO18 | Pattern Cycle | With pull-up |
| | BTN2 | GPIO19 | Battery Display | With pull-up |  
| | BTN3 | GPIO23 | Game Toggle | With pull-up |
| **ADC** | Battery | GPIO26 | Voltage Monitor | Via voltage divider |

## Testing and Verification (Updated)

### **Enhanced Safety Testing:**
1. **Temperature Response Test:**
   - Heat battery with hair dryer (carefully!)
   - Verify charging stops at high temperature
   - Check ESP32 receives temperature alerts

2. **Fuel Gauge Alert Test:**
   - Discharge battery to low level
   - Verify ALT pin triggers interrupt
   - Test quick start recalibration

3. **Power Path Test:**
   - Connect/disconnect USB-C while operating
   - Verify seamless power switching
   - Check charging status indicators

### **Required Components (Updated):**
- **10kΩ NTC Thermistor** (critical for safety)
- **3M thermal tape** (for thermistor mounting)  
- **Pull-up resistors** (4.7kΩ for I2C if not built-in)
- **Voltage divider resistors** (47kΩ + 22kΩ for battery monitoring)

This updated wiring diagram now includes all the specific pin connections and the critical NTC temperature monitoring system for safe Li-ion charging! 