# ULTRA-DETAILED WIRING GUIDE - ESP32 DevKit v1 Edition

## Overview
**COMPLETE** step-by-step wiring guide for ESP32 DevKit v1 based LED panel with every single wire, resistor, capacitor, and connection specified. Designed for **ONE-SHOT BUILD** success.

## Your Specific Hardware
- **ESP32:** DevKit v1 (30-pin) as shown in photo
- **Prototype PCB:** Standard breadboard-style perfboard
- **Wire Gauges Available:** 14AWG, 18AWG, 22AWG single-core
- **LED Panel:** 16x16 WS2812B with spliced power connections
- **NTC Thermistor:** Black thermistor with long wires (from existing kit)
- **Future:** Pogo pin for LED panel chaining (DOUT connection)

## ESP32 DevKit v1 Pinout Reference

```
ESP32 DevKit v1 (30-pin) - USB-C Facing Down:
┌─────────────────────────────────┐
│ EN                         D23  │ ← Button 3
│ VP                         D22  │ ← I2C SCL
│ VN                         TX0  │ 
│ D34                        RX0  │ 
│ D35                        D21  │ ← I2C SDA
│ D32                        D19  │ ← Button 2
│ D33                        D18  │ ← Button 1
│ D25                        D5   │ ← Battery ADC
│ D26                        TX2  │ 
│ D27                        RX2  │ ← Fuel Alert (Max17048)
│ D14                        D4   │ ← QST Pin (Fuel Gauge)
│ D12                        D2   │ ← LED Data Pin
│ D13                        D15  │ ← PWM Control (TPS61088)
│ GND                        GND  │ ← Ground
│ VIN                        3V3  │ ← 3.3V Output (500mA max)
└─────────────────────────────────┘
    USB-C Connector (bottom)
```

## Complete Component List with Part Numbers

### Power Management Components
- **BQ25606 USB-C Charging Module** (3A, 4.5-12V input)
- **TPS61088 Boost Converter Module** (10A, 1MHz switching)
- **Max17048 LiPo Fuel Gauge Module** (I2C, ±3% accuracy)
- **10000mAh 1S Li-ion Battery** (3.7V nominal, PH2.0 connector)
- **10kΩ NTC Thermistor** (black, with long wires - your existing one)

### Electronic Components Needed
- **330Ω Resistor** (1/4W, 5%) - LED data line protection
- **1000µF Electrolytic Capacitor** (16V+) - LED power decoupling
- **47kΩ Resistor** (1/4W, 5%) - Battery voltage divider (top)
- **22kΩ Resistor** (1/4W, 5%) - Battery voltage divider (bottom)
- **4.7kΩ Resistor × 2** (1/4W, 5%) - I2C pull-ups (if needed)
- **10kΩ Resistor × 3** (1/4W, 5%) - Button pull-ups (if using external)

### Mechanical Components
- **3x Tactile Push Buttons** (6mm × 6mm, momentary)
- **Pogo Pin Connector** (for LED panel chaining)
- **Heat Shrink Tubing** (various sizes)
- **3M Thermal Tape** (for NTC thermistor mounting)

## Detailed Perfboard Layout Strategy

### Perfboard Organization (31×26 holes, 7×9 CM)
```
Perfboard Layout (Component Side View):
┌──────────────────────────────────────────────────────────────────────────┐
│ 1  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 2  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 3  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 4  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 5  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 6  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 7  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 8  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 9  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│10  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│11  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│12  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│13  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│14  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│15  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│16  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│17  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│18  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│19  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│20  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│21  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│22  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│23  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│24  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│25  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│26  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
└──────────────────────────────────────────────────────────────────────────┘
   A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z AA BB CC DD EE

ESP32 Position: Rows 8-19, Columns K-U (spans 11 columns for A-K pins)
```

### Component Placement Strategy

#### PERFBOARD-MOUNTED Components (Soldered to PCB):
- **Columns A-J:** Power management modules (TPS61088, Max17048, MPU6050)
- **Columns K-U:** ESP32 DevKit v1 (11 columns wide, spans A-K pins)
- **Columns V-EE:** Resistors, capacitors, voltage divider, LED data circuit
- **Row 1:** +5V power rail (use 18AWG wire)
- **Row 26:** Ground rail (use 18AWG wire)
- **ESP32 Placement:** Center of board (Rows 8-19) for optimal routing

#### HOUSING-MOUNTED Components (Connected via Wires):
- **BQ25606 USB-C Charger:** Mounted flush with housing bottom (USB-C port accessible)
- **10000mAh Battery:** Secured inside housing with foam padding
- **3x Tactile Buttons:** Mounted on housing top (accessible from outside)
- **16x16 LED Panel:** Front-facing, secured to housing front
- **NTC Thermistor:** Taped directly to battery surface

#### Wire Length Recommendations:
- **BQ25606 to Perfboard:** 6-8 inches (15-20cm) - allows positioning flexibility
- **Battery to BQ25606:** 4-6 inches (10-15cm) - keep power wires short
- **Buttons to Perfboard:** 4-8 inches (10-20cm) depending on housing height
- **LED Panel to Perfboard:** 6-10 inches (15-25cm) for front mounting
- **NTC Thermistor:** Use existing long wires (likely 8-12 inches)

## CRITICAL SAFETY NOTES FOR BEGINNERS

### ⚠️ BEFORE YOU START - READ THIS! ⚠️

#### Power Safety:
- **NEVER connect power while building** - only power on for testing
- **Always disconnect battery** when making changes
- **Use a multimeter** to verify connections before powering on
- **14AWG wire gets HOT** under high current - this is normal but be careful
- **If anything gets very hot or smells** - disconnect power immediately

#### Soldering Safety:
- **Use flux** - makes soldering 100× easier and prevents cold joints
- **Tin your wire ends** before soldering to pads
- **Clean your iron tip** frequently with damp sponge
- **Don't touch hot iron** - 400°C will burn you instantly
- **Work in ventilated area** - flux smoke is not healthy

#### Component Handling:
- **ESP32 is static sensitive** - touch ground before handling
- **BQ25606 pads are tiny** - use magnifying glass and steady hands
- **Battery can deliver 30A+** - short circuits will spark/burn
- **LEDs draw 15A maximum** - respect the current requirements

#### Testing Protocol:
1. **Visual inspection** - no shorts, clean solder joints
2. **Continuity test** - multimeter beep test for connections  
3. **Power rail test** - measure 5V and 3.3V before connecting devices
4. **Smoke test** - power on briefly, check for magic smoke
5. **Function test** - verify each subsystem works

#### Emergency Procedures:
- **If sparks/smoke:** Disconnect battery immediately
- **If very hot components:** Power off, let cool, check connections
- **If nothing works:** Check power rails first, then data connections
- **If intermittent issues:** Check solder joints for cold/cracked joints

## STEP-BY-STEP WIRING INSTRUCTIONS

### STEP 1: Power Rails (CRITICAL - Do This First)

#### 1.1: Create 5V Power Rail
```
Materials: 18AWG red wire, solder, flux
Location: Row 1 (entire length)

Connections:
1A ──[18AWG RED]── 1B ──[18AWG RED]── 1C ──[continue]── 1EE
│                  │                  │
└─[Solder Joint]───┴─[Solder Joint]───┴─[Continue pattern]

Result: Continuous 5V rail across entire Row 1
Current Capacity: 15A+ (sufficient for 10A LED + 3A ESP32)
```

#### 1.2: Create Ground Rail
```
Materials: 18AWG black wire, solder, flux
Location: Row 26 (entire length)

Connections:
26A ──[18AWG BLACK]── 26B ──[18AWG BLACK]── 26C ──[continue]── 26EE
│                     │                     │
└─[Solder Joint]──────┴─[Solder Joint]──────┴─[Continue pattern]

Result: Continuous Ground rail across entire Row 26
```

### STEP 2: Battery and Charging System

#### 2.1: BQ25606 USB-C Charging Module Connections

**IMPORTANT FOR BEGINNERS:** The BQ25606 is mounted in your housing (NOT on the perfboard) with the USB-C port accessible from outside. **This module ONLY charges your battery - it does NOT power your system.**

```
BQ25606 Module Pinout:
┌─────────────────────────────────┐
│        BQ25606 Module           │
│  ┌─────────────────────────┐    │
│  │      USB-C Female       │    │ ← 5V-12V Input (mount flush with housing)
│  └─────────────────────────┘    │
│                                 │
│ V+  V-  B+  B-  U+  U-  NTC NTC │ ← Solder pads (tiny, use flux!)
└─────────────────────────────────┘

**BEGINNER TIPS:**
- V+ and V- are NOT used in this build (leave disconnected)
- B+ and B- connect directly to your battery (charging only)
- NTC pads connect to the thermistor taped on your battery
- U+ and U- are optional USB voltage monitoring (leave unconnected)

Detailed Connections:
┌──────────────┬─────────────┬──────────────┬─────────────────────┐
│ BQ25606 Pin  │ Wire Gauge  │ Destination  │ Notes               │
├──────────────┼─────────────┼──────────────┼─────────────────────┤
│ V+ (Sys Out) │ -           │ NOT USED     │ Leave unconnected   │
│ V- (Sys GND) │ -           │ NOT USED     │ Leave unconnected   │
│ B+ (Batt +)  │ 18AWG Red   │ Battery +    │ Direct (4" wire)    │
│ B- (Batt -)  │ 18AWG Black │ Battery -    │ Direct (4" wire)    │
│ U+ (USB +)   │ -           │ Not used     │ Leave unconnected   │
│ U- (USB -)   │ -           │ Not used     │ Leave unconnected   │
│ NTC Pad 1    │ 22AWG Any   │ NTC Therm    │ One thermistor wire │
│ NTC Pad 2    │ 22AWG Any   │ NTC Therm    │ Other thermistor wire│
└──────────────┴─────────────┴──────────────┴─────────────────────┘

**BATTERY WIRE UPGRADE (CRITICAL):**
- **JST connector wires are too thin** (22-24AWG) for high current
- **MUST upgrade to 18AWG** for safety - LEDs can draw 15A+
- **Desolder thin JST wires** from battery BMS carefully
- **Solder 18AWG wires directly** to BMS + and - pads
- **Test polarity with multimeter** before connecting
- **Use heat shrink tubing** to protect BMS connections

**SOLDERING TIPS:**
- Use flux on the tiny BQ25606 pads - they're small!
- Tin your wire ends before soldering to the pads
- Test continuity with multimeter after each connection
- The pads are labeled but tiny - use a magnifying glass if needed
```

### STEP 3: TPS61088 Boost Converter (MAIN POWER SOURCE)

#### 3.1: TPS61088 Module Connections

**IMPORTANT FOR BEGINNERS:** The TPS61088 is your main power source - it takes 3.7V from the battery and boosts it to 5V for your entire system.

```
TPS61088 Boost Converter Wiring:
┌─────────────────┬─────────────┬──────────────┬─────────────────┐
│ TPS61088 Pin    │ Wire Gauge  │ Source       │ Perfboard Loc   │
├─────────────────┼─────────────┼──────────────┼─────────────────┤
│ VIN+ (Input)    │ 18AWG Red   │ Battery +    │ Direct          │
│ VIN- (Input)    │ 18AWG Black │ Battery -    │ Direct          │
│ VOUT+ (5V Out)  │ 18AWG Red   │ 5V Power Rail│ 1F              │
│ VOUT- (Ground)  │ 18AWG Black │ Ground Rail  │ 26F             │
│ PWM Signal      │ 22AWG       │ ESP32 D15    │ 9F              │
│ PWM Ground      │ 22AWG Black │ Ground Rail  │ 26F             │
│ ADJ (Voltage)   │ -           │ Not used     │ Pre-set to 5V   │
└─────────────────┴─────────────┴──────────────┴─────────────────┘

**POWER FLOW EXPLANATION:**
- **Battery 3.7V** → TPS61088 VIN+/VIN-
- **TPS61088 VOUT+/VOUT-** → 5V Power Rails on perfboard
- **5V Power Rails** → ESP32, LED Panel, everything else

PWM Control Logic:
- PWM HIGH (3.3V): TPS61088 enabled → System powered
- PWM LOW (0V): TPS61088 disabled → System off (power saving)
- Default: Set PWM HIGH for normal operation

**PWM Connection Details (Two PWM Pads):**
- **PWM Signal Pad** → ESP32 D15 (22AWG wire)
- **PWM Ground Pad** → Ground Rail (22AWG black wire)
- **POLARITY MATTERS** - One pad is signal, one is ground reference
- **ADJ PADS SEPARATE** - Leave ADJ pads unconnected (pre-set to 5V)
- Usually marked as PWM+ (signal) and PWM- (ground) or similar
```

#### 3.2: NTC Thermistor Installation (CRITICAL SAFETY)
```
Your Black NTC Thermistor Installation:
┌─────────────────────────────────────────────────────────────┐
│                    Battery Pack                             │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                                                     │    │
│  │              10000mAh Li-ion                        │    │
│  │                                                     │    │
│  │  [3M Thermal Tape] ← NTC Thermistor taped here     │    │
│  │  ┌─[Black NTC]─┐                                   │    │
│  │  │    ●●●●●    │ ← Thermistor body against battery │    │
│  │  └─────────────┘                                   │    │
│  └─────────────────────────────────────────────────────┘    │
│           │                                                 │
│           └─[Long Wires]─→ BQ25606 NTC Pads                │
└─────────────────────────────────────────────────────────────┘

NTC Connection Details:
- Use 3M thermal tape to attach NTC body directly to battery surface
- Route long wires to BQ25606 NTC pads
- **POLARITY DOESN'T MATTER** - NTC thermistor is a simple resistor
- **ONE LEG TO EACH PAD** - Connect one thermistor wire to each NTC pad
- Ensure good thermal contact for accurate temperature reading
- The BQ25606 measures resistance between the two NTC pads
```

### STEP 4: ESP32 DevKit v1 Connections

#### 4.1: Power Connections
```
ESP32 Power Wiring:
┌─────────────────┬─────────────┬──────────────┬─────────────────┐
│ ESP32 Pin       │ Wire Gauge  │ Source       │ Perfboard Loc   │
├─────────────────┼─────────────┼──────────────┼─────────────────┤
│ VIN (5V Input)  │ 18AWG Red   │ 5V Power Rail│ 1O              │
│ GND (Ground)    │ 18AWG Black │ Ground Rail  │ 26O             │
│ 3V3 (3.3V Out)  │ 22AWG Red   │ Sensor Power │ 2AA (sensor VCC)│
└─────────────────┴─────────────┴──────────────┴─────────────────┘

Critical Notes:
- VIN accepts 5V from BQ25606 (safer than direct battery)
- ESP32 internal regulator provides 3.3V for sensors
- Never connect 5V directly to GPIO pins (3.3V logic only)
```

#### 4.2: GPIO Connections
```
ESP32 GPIO Assignments:
┌─────────────┬─────────────┬──────────────────┬─────────────────┐
│ ESP32 Pin   │ Wire Gauge  │ Function         │ Destination     │
├─────────────┼─────────────┼──────────────────┼─────────────────┤
│ D2          │ 22AWG       │ LED Data         │ AA20 → LED DIN  │
│ D15         │ 22AWG       │ TPS61088 PWM     │ F9              │
│ D21         │ 22AWG       │ I2C SDA          │ AA13            │
│ D22         │ 22AWG       │ I2C SCL          │ AA14            │
│ D5          │ 22AWG       │ Battery ADC      │ Voltage Divider │
│ RX2         │ 22AWG       │ Fuel Alert       │ Max17048 ALT    │
│ D4          │ 22AWG       │ Fuel QST         │ Max17048 QST    │
│ D18         │ 22AWG       │ Button 1         │ BB15            │
│ D19         │ 22AWG       │ Button 2         │ BB16            │
│ D23         │ 22AWG       │ Button 3         │ BB17            │
└─────────────┴─────────────┴──────────────────┴─────────────────┘
```

### STEP 5: Max17048 Fuel Gauge

#### 5.1: Max17048 Module Connections
```
Max17048 Fuel Gauge Wiring:
┌─────────────────┬─────────────┬──────────────┬─────────────────┐
│ Max17048 Pin    │ Wire Gauge  │ Source       │ Perfboard Loc   │
├─────────────────┼─────────────┼──────────────┼─────────────────┤
│ VCC (Power)     │ 22AWG Red   │ ESP32 3.3V   │ 2AA             │
│ GND (Ground)    │ 22AWG Black │ Ground Rail  │ 26E             │
│ SDA (I2C Data)  │ 22AWG       │ ESP32 D21    │ 13AA (shared)   │
│ SCL (I2C Clock) │ 22AWG       │ ESP32 D22    │ 14AA (shared)   │
│ ALT (Alert)     │ 22AWG       │ ESP32 RX2    │ 12E             │
│ QST (QuickStart)│ 22AWG       │ ESP32 D4     │ 13E             │
│ + Pin (Batt+)   │ 22AWG Red   │ Battery +    │ Direct          │
│ - Pin (Batt-)   │ 22AWG Black │ Battery -    │ Direct          │
│ Battery Wire    │ -           │ Battery      │ Direct          │
└─────────────────┴─────────────┴──────────────┴─────────────────┘

I2C Address: 0x36 (different from MPU6050 at 0x68)
Alert Function: Pulls LOW when battery < 10% (configurable)
```

### STEP 6: MPU6050 Gyroscope/Accelerometer

#### 6.1: MPU6050 Module Connections
```
MPU6050 Sensor Wiring:
┌─────────────────┬─────────────┬──────────────┬─────────────────┐
│ MPU6050 Pin     │ Wire Gauge  │ Source       │ Perfboard Loc   │
├─────────────────┼─────────────┼──────────────┼─────────────────┤
│ VCC (Power)     │ 22AWG Red   │ ESP32 3.3V   │ 2AA             │
│ GND (Ground)    │ 22AWG Black │ Ground Rail  │ 26G             │
│ SDA (I2C Data)  │ 22AWG       │ ESP32 D21    │ 13AA (shared)   │
│ SCL (I2C Clock) │ 22AWG       │ ESP32 D22    │ 14AA (shared)   │
│ XDA (Aux SDA)   │ -           │ Not used     │ Leave open      │
│ XCL (Aux SCL)   │ -           │ Not used     │ Leave open      │
│ AD0 (Address)   │ -           │ Ground       │ 26G (for 0x68)  │
│ INT (Interrupt) │ -           │ Optional     │ Not connected   │
└─────────────────┴─────────────┴──────────────┴─────────────────┘

I2C Address: 0x68 (AD0 connected to ground)
Shared I2C Bus: Same SDA/SCL as Max17048 (different addresses)
```

### STEP 7: Button Interface

#### 7.1: Three Button Wiring

**IMPORTANT FOR BEGINNERS:** Buttons are mounted on the TOP of your housing (NOT on perfboard) so users can press them. You'll run wires from each button down to the perfboard.

```
Button Wiring (Using Internal Pull-ups - No External Resistors Needed!):
┌─────────────┬─────────────┬──────────────┬─────────────────┐
│ Button      │ Wire Gauge  │ ESP32 Pin    │ Perfboard Loc   │
├─────────────┼─────────────┼──────────────┼─────────────────┤
│ Button 1    │ 22AWG       │ D18          │ BB15            │
│ Button 2    │ 22AWG       │ D19          │ BB16            │
│ Button 3    │ 22AWG       │ D23          │ BB17            │
└─────────────┴─────────────┴──────────────┴─────────────────┘

**HOW BUTTONS WORK (Beginner Explanation):**
- Tactile buttons have 4 legs but only 2 connections matter
- When pressed: connects the two sides together (closes circuit)
- When released: disconnects the two sides (opens circuit)
- ESP32 "pull-up" means the pin reads HIGH (3.3V) normally
- When button pressed: pin connects to ground, reads LOW (0V)

Individual Button Connections (Each needs 2 wires):
Button 1 (Pattern Cycle) - 6mm tactile switch:
- Wire 1: ESP32 D18 → Perfboard BB15 → Button terminal
- Wire 2: Ground Rail 26BB → Button other terminal
- Function: Changes LED patterns when pressed

Button 2 (Battery Display) - 6mm tactile switch:
- Wire 1: ESP32 D19 → Perfboard BB16 → Button terminal  
- Wire 2: Ground Rail 26BB → Button other terminal
- Function: Shows battery level on LEDs when pressed

Button 3 (Game Toggle) - 6mm tactile switch:
- Wire 1: ESP32 D23 → Perfboard BB17 → Button terminal
- Wire 2: Ground Rail 26BB → Button other terminal
- Function: Starts/stops games when pressed

**BUTTON MOUNTING TIPS:**
- Use 6mm × 6mm tactile switches (standard size)
- Drill 6mm holes in housing top for button stems
- Buttons should "click" when pressed and spring back
- Test each button with multimeter before final assembly
- Use different colored button caps if available for easy identification
```

### STEP 8: LED Panel Connections

#### 8.1: WS2812B 16x16 Panel Wiring

**IMPORTANT FOR BEGINNERS:** The LED panel mounts on the FRONT of your housing (NOT on perfboard) so the LEDs are visible. You'll run 3-4 wires from the panel back to the perfboard.

```
LED Panel Power (High Current - VERY IMPORTANT!):
┌─────────────────┬─────────────┬──────────────┬─────────────────┐
│ LED Panel Pin   │ Wire Gauge  │ Source       │ Perfboard Loc   │
├─────────────────┼─────────────┼──────────────┼─────────────────┤
│ VCC (5V)        │ 14AWG Red   │ 5V Power Rail│ 1Y-1AA (3 pts)  │
│ GND (Ground)    │ 14AWG Black │ Ground Rail  │ 26Y-26AA (3 pts)│
│ DIN (Data In)   │ 22AWG       │ ESP32 D2     │ Via 330Ω res    │
│ DOUT (Data Out) │ 22AWG       │ Pogo Pin     │ Future chaining  │
└─────────────────┴─────────────┴──────────────┴─────────────────┘

**WHY THICK WIRES FOR POWER (Beginner Explanation):**
- 16×16 = 256 LEDs total
- Each LED can draw up to 60mA at full brightness
- 256 × 60mA = 15.36A maximum current!
- 14AWG wire can safely carry 15A+ without overheating
- Thinner wires would get HOT and could melt/start fires

**LED PANEL CONNECTIONS EXPLAINED:**
- VCC: +5V power input (RED wire, thick!)
- GND: Ground/negative power (BLACK wire, thick!)  
- DIN: Data input - receives color commands from ESP32
- DOUT: Data output - for chaining additional panels (future use)

Critical Power Connections:
- Use 14AWG wire for LED power (15A capacity needed)
- Connect VCC to multiple perfboard points (1Y, 1Z, 1AA) for current distribution
- Connect GND to multiple perfboard points (26Y, 26Z, 26AA) for current distribution
- This spreads high current across multiple solder joints (safety!)
- NEVER use thin wire for LED power - fire hazard!

**MOUNTING TIPS:**
- LED panel has arrows showing data direction (DIN → DOUT)
- Mount panel so you can see the LEDs from the front
- Secure with screws or strong double-sided tape
- Keep power wires as short as practical (6-8 inches max)
- Test panel with simple code before final mounting
```

#### 8.2: LED Data Line with Protection
```
LED Data Signal Chain:
ESP32 D2 ──[22AWG]── 20AA ──[330Ω Resistor]── LED Panel DIN
                     │
                     └──[1000µF Cap]── Ground Rail (26AA)

Component Details:
- 330Ω Resistor (1/4W): Protects ESP32 GPIO from LED current spikes
- 1000µF Capacitor (16V): Smooths LED power supply, prevents brownouts
- Place capacitor physically close to LED panel power connection
```

#### 8.3: Pogo Pin for Panel Chaining (Future)
```
Pogo Pin Connection:
LED Panel DOUT ──[22AWG]── Pogo Pin Connector
                          │
                          └── Future: Next LED Panel DIN

Installation:
- Mount pogo pin on edge of perfboard
- Connect to LED panel DOUT with 22AWG wire
- Allows daisy-chaining multiple panels later
- Not needed for initial single-panel operation
```

### STEP 9: Battery Voltage Monitoring (ADC Backup)

#### 9.1: Voltage Divider Circuit
```
Battery Voltage Divider:
Battery+ ──[47kΩ]── Measurement Point ──[22kΩ]── Ground
                           │
                           └──[22AWG]── ESP32 D5 (ADC)

Component Placement:
- 47kΩ Resistor: 21K to 21L
- 22kΩ Resistor: 21L to 26L
- Measurement tap: 21L to ESP32 D5
- Input: Battery+ to 21K
- Ground: 26L to ground rail

Calculation:
- Battery Range: 3.0V - 4.2V
- ADC Input Range: 0.96V - 1.34V (safe for ESP32 3.3V ADC)
- Voltage Ratio: 22kΩ/(47kΩ+22kΩ) = 0.319
```

### STEP 10: I2C Bus Pull-up Resistors (If Needed)

#### 10.1: I2C Pull-up Installation
```
I2C Pull-up Resistors (Only if communication fails):
3.3V ──[4.7kΩ]── SDA Line (13AA)
3.3V ──[4.7kΩ]── SCL Line (14AA)

Installation Points:
- SDA Pull-up: 2AA (3.3V) to 13AA (SDA)
- SCL Pull-up: 2AA (3.3V) to 14AA (SCL)

Note: Most ESP32 boards have internal pull-ups
Only add if I2C devices not detected in code
```

## COMPLETE PERFBOARD LAYOUT

### Final Component Placement Map
```
Perfboard Layout (Component Placement):
┌──────────────────────────────────────────────────────────────────────────┐
│ 1 [5V RAIL]──────────────────────────────────────────────────────────────│
│ 2  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ 3V3○ ○ ○ ○ ○ │
│ 3  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 4  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 5  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 6  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 7  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ 8  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○[ESP32 DevKit v1 - Row 8-19, Col K-U] ○ ○ ○ ○ │
│ 9  ○ ○ ○ ○ ○PWM○ ○ ○ ○ ○                                      ○ ○ ○ ○ │
│10  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                      ○ ○ ○ ○ │
│11  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                      ○ ○ ○ ○ │
│12  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                      ○ ○ ○ ○ │
│13  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                     SDA○ ○ ○ ○ │
│14  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                     SCL○ ○ ○ ○ │
│15  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                      ○BT1○ ○ ○ │
│16  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                      ○BT2○ ○ ○ │
│17  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                      ○BT3○ ○ ○ │
│18  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                      ○ ○ ○ ○ │
│19  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                                      ○ ○ ○ ○ │
│20  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○LED○ ○ ○ ○ │
│21  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○47K○22K○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│22  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│23  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│24  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│25  ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│26 [GND RAIL]─────────────────────────────────────────────────────────────│
└──────────────────────────────────────────────────────────────────────────┘
   A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z AA BB CC DD EE

Legend:
3V3 = ESP32 3.3V output tap (2AA)
SDA/SCL = I2C bus connections (13AA/14AA)
PWM = TPS61088 control (9F)
BT1/2/3 = Button connections (15BB/16BB/17BB)
47K/22K = Voltage divider resistors (21K/21L)
LED = LED data connection with 330Ω resistor (20AA)
```

## ASSEMBLY ORDER (CRITICAL SEQUENCE FOR BEGINNERS)

### Phase 1: Perfboard Power Infrastructure (Do First!)
1. **Install Power Rails** (A-row 5V, Z-row GND) with 18AWG wire
2. **Test Rail Continuity** with multimeter across entire rails (should beep)
3. **Solder ESP32 to Perfboard** (center position, rows F-Q, columns 10-20)
4. **Test ESP32 Power** - connect 5V to VIN, GND to GND, should boot up

### Phase 2: Housing-Mounted Power System
5. **Mount BQ25606 in Housing** (USB-C port accessible from outside)
6. **Install NTC Thermistor** on battery with 3M thermal tape
7. **Connect Battery to BQ25606** (B+/B- terminals, keep wires short!)
8. **Wire BQ25606 to Perfboard** (V+/V- to power rails, 6-8" wires)
9. **Test Charging System** - should read 5V on rails when USB-C connected

### Phase 3: Perfboard-Mounted Modules
10. **Install TPS61088 on Perfboard** (columns 1-9 area)
11. **Connect TPS61088 Power** (input from rails, output to LED power area)
12. **Install Max17048 on Perfboard** (I2C sensor area)
13. **Install MPU6050 on Perfboard** (shares I2C bus)
14. **Test I2C Communication** with bus scanner code (should find 0x36 and 0x68)

### Phase 4: Housing-Mounted User Interface
15. **Mount Buttons in Housing Top** (drill 6mm holes, test fit)
16. **Wire Buttons to Perfboard** (signal + ground wires, 4-8" length)
17. **Test Button Response** with simple code (serial monitor)
18. **Install Voltage Divider** for battery monitoring (perfboard-mounted)

### Phase 5: LED Panel Integration  
19. **Mount LED Panel in Housing Front** (visible from outside)
20. **Install LED Data Circuit** (330Ω resistor + 1000µF capacitor on perfboard)
21. **Wire LED Panel Power** (14AWG wires to perfboard, multiple connection points)
22. **Wire LED Panel Data** (22AWG wire through 330Ω resistor)
23. **Test LED Panel** with basic pattern (start with low brightness!)

### Phase 6: Final Integration & Testing
24. **Upload Complete Code** 
25. **Test All Functions Systematically** (power, charging, buttons, LEDs, sensors)
26. **Install Pogo Pin** for future expansion (optional)
27. **Final Assembly in Housing** with proper wire management
28. **Complete System Test** with battery operation and all features

### ⚠️ IMPORTANT ASSEMBLY NOTES:
- **Test each phase** before moving to the next
- **Keep wires organized** - use different colors for different functions
- **Label your wires** with masking tape if needed
- **Take photos** of your progress in case you need to troubleshoot later
- **Don't rush** - better to spend extra time getting it right

## TESTING CHECKLIST

### Power System Tests
- [ ] **5V Rail Continuity**: Multimeter across A1 to A24
- [ ] **Ground Rail Continuity**: Multimeter across J1 to J24  
- [ ] **BQ25606 Output**: 5V between V+ and V- when USB connected
- [ ] **ESP32 Power**: 3.3V on ESP32 3V3 pin
- [ ] **TPS61088 Output**: 5V on VOUT when PWM enabled
- [ ] **Battery Charging**: Battery voltage increases when USB connected

### Communication Tests  
- [ ] **I2C Bus Scan**: Detects 0x36 (Max17048) and 0x68 (MPU6050)
- [ ] **Max17048 Communication**: Reads battery percentage
- [ ] **MPU6050 Communication**: Reads accelerometer values
- [ ] **Button Response**: All three buttons register presses
- [ ] **LED Panel**: Displays test pattern

### Safety Tests
- [ ] **NTC Temperature**: BQ25606 responds to heated battery
- [ ] **Low Battery Alert**: Max17048 ALT pin triggers at 10%
- [ ] **Emergency Shutdown**: System shuts down at critical voltage
- [ ] **Overcurrent Protection**: System handles LED current spikes

## BEGINNER TROUBLESHOOTING GUIDE

### Common Problems and Solutions:

#### "Nothing Powers On"
1. **Check battery charge** - use multimeter, should read 3.7V+
2. **Check BQ25606 connections** - V+/V- wired to perfboard rails?
3. **Check power rail continuity** - multimeter should beep across rails
4. **Check ESP32 VIN connection** - should connect to 5V rail (A15)

#### "ESP32 Won't Program"
1. **Check USB cable** - try different cable (data, not just charging)
2. **Check drivers** - install CP2102 or CH340 drivers for your ESP32
3. **Hold BOOT button** while clicking upload in Arduino IDE
4. **Check power** - ESP32 needs stable 5V on VIN pin

#### "I2C Devices Not Found"
1. **Check wiring** - SDA to D21, SCL to D22, VCC to 3.3V, GND to ground
2. **Check addresses** - Max17048 = 0x36, MPU6050 = 0x68
3. **Try pull-up resistors** - 4.7kΩ from SDA/SCL to 3.3V
4. **Check solder joints** - cold joints cause intermittent connections

#### "LEDs Don't Light Up"
1. **Check power wires** - 14AWG from TPS61088 to LED VCC/GND
2. **Check data wire** - 22AWG from ESP32 D2 through 330Ω resistor to LED DIN
3. **Check TPS61088 enable** - PWM pin (D15) should be HIGH
4. **Start simple** - test with 1 LED at low brightness first

#### "Buttons Don't Work"
1. **Check wiring** - one wire to ESP32 pin, other to ground
2. **Test with multimeter** - should show continuity when pressed
3. **Check code** - pins set to INPUT_PULLUP mode?
4. **Check solder joints** - button connections often break

#### "Battery Won't Charge"
1. **Check USB-C cable** - needs to supply 5V+ (phone charger works)
2. **Check BQ25606 connections** - B+/B- to battery, V+/V- to system
3. **Check NTC thermistor** - should be taped to battery surface
4. **Check battery health** - old batteries may not accept charge

#### "System Randomly Resets"
1. **Check power supply** - insufficient current causes brownouts
2. **Check LED current limiting** - reduce brightness in code
3. **Check solder joints** - intermittent power connections
4. **Add capacitors** - 1000µF near LED power connection

### Multimeter Usage for Beginners:
- **Continuity mode** (beep symbol): Tests if wires are connected
- **DC Voltage mode** (V with straight line): Measures battery/power voltages  
- **Resistance mode** (Ω symbol): Tests resistor values
- **Red probe = positive, Black probe = negative/ground**

### When to Ask for Help:
- **Magic smoke appears** - something is seriously wrong, stop immediately
- **Components get burning hot** - disconnect power, check for shorts
- **Multiple systems fail** - might be fundamental wiring error
- **Intermittent weird behavior** - often power supply or grounding issues

This ultra-detailed guide should enable a successful build! Take your time, test frequently, and don't hesitate to double-check connections. 