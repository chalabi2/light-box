# LED PANEL BUILD PLAN - ESP32 EDITION

## Overview
This build plan guides you through creating an ESP32-based 16x16 LED panel with battery management, gyroscope-controlled mazeball game, and wireless configuration. The design is significantly simpler and more cost-effective than the original Raspberry Pi version.

**New Components Available:**
- ESP32 Development Board (30-pin)
- 10000mAh 1S Li-ion Battery (3.7V)
- TPS61088 Boost Converter (5V/9V/12V selectable)
- Max17048 Battery Fuel Gauge
- BQ25606 USB-C Charging Module
- MPU-6050 Gyroscope/Accelerometer
- 16×16 WS2812B LED Panel
- 3x Push Buttons

---

## PHASE 1: BREADBOARD PROTOTYPING & BASIC FUNCTIONALITY
*Goal: Verify all components work together and establish basic LED control*

### Step 1.1: ESP32 Setup and LED Panel Connection
**Materials Needed:**
- Breadboard (large size recommended)
- Jumper wires (male-to-male, male-to-female)
- 330Ω resistor
- 1000µF capacitor
- Multimeter

**Tasks:**
1. **ESP32 Programming Environment Setup:**
   ```bash
   # Install Arduino IDE and ESP32 board package
   # Add ESP32 board manager URL: https://dl.espressif.com/dl/package_esp32_index.json
   # Install FastLED library for WS2812B control
   ```

2. **Basic LED Panel Test:**
   - Connect LED panel VCC to 5V power supply (temporary external supply)
   - Connect LED panel GND to common ground
   - Connect LED panel DIN to ESP32 GPIO16 through 330Ω resistor
   - Add 1000µF capacitor across LED power/ground
   - Upload basic LED test pattern

3. **Verify LED Operation:**
   - Test individual LED control
   - Verify color order (GRB for WS2812B)
   - Check for any dead pixels or connection issues

### Step 1.2: Power System Integration
**Tasks:**
1. **TPS61088 Configuration:**
   - Set output voltage to 5V using onboard adjustment
   - Connect input to temporary 3.7V source (or use adjustable supply)
   - Measure output voltage under no load and full LED load
   - Test PWM control pin functionality

2. **Battery Connection:**
   - Connect 1S battery to TPS61088 input
   - Verify boost converter maintains 5V output as battery depletes
   - Test current draw at various LED brightness levels

3. **ESP32 Power Integration:**
   - Power ESP32 from battery through VIN pin
   - Verify stable 3.3V regulation under load
   - Test ESP32 operation throughout battery voltage range (3.0V-4.2V)

### Step 1.3: Sensor Integration
**Tasks:**
1. **MPU-6050 Setup:**
   - Connect VCC to ESP32 3.3V
   - Connect GND to common ground
   - Connect SDA to GPIO21, SCL to GPIO22
   - Install Adafruit MPU6050 library
   - Test I2C communication and basic readings

2. **Max17048 Battery Gauge:**
   - Connect to same I2C bus as MPU-6050
   - Connect CELL+ and CELL- to battery terminals
   - Install SparkFun MAX1704x library
   - Verify battery percentage readings

3. **I2C Bus Testing:**
   - Scan I2C bus to confirm both devices detected
   - Test simultaneous operation of both sensors
   - Implement proper error handling

### Step 1.4: Button Interface
**Tasks:**
1. **Button Wiring:**
   - Connect buttons to GPIO18, GPIO19, GPIO23
   - Use internal pull-up resistors (INPUT_PULLUP mode)
   - Wire button other terminal to ground

2. **Interrupt Setup:**
   - Configure pin change interrupts for responsive button handling
   - Implement software debouncing
   - Test short press and long press detection

3. **Button Function Testing:**
   - Pattern cycling (GPIO18)
   - Battery display toggle (GPIO19)
   - Game mode toggle (GPIO23)

### Step 1.5: USB-C Charging Integration
**Tasks:**
1. **BQ25606 Setup:**
   - Connect BAT+ and BAT- to battery terminals
   - Connect USB-C input for charging
   - Test charging with various USB-C adapters (5V, 9V, 12V)

2. **Charging Safety:**
   - Verify charge current limiting
   - Test thermal protection
   - Monitor charging status indicators

3. **Power Path Management:**
   - Test operation while charging
   - Verify power path priority (charge vs. load)

**Phase 1 Completion Criteria:**
- [ ] ESP32 reliably controls LED panel
- [ ] Battery powers entire system for 4+ hours
- [ ] All sensors communicating on I2C bus
- [ ] Buttons responsive with proper debouncing
- [ ] USB-C charging functional and safe
- [ ] Basic tilt-responsive LED effects working

---

## PHASE 2: SOFTWARE ARCHITECTURE & CORE FEATURES
*Goal: Implement modular software architecture and core functionality*

### Step 2.1: Code Restructuring
**Tasks:**
1. **Multi-File Architecture:**
   - Create separate files for different modules:
     - `main.ino` - Main loop and setup
     - `led_control.h/.cpp` - LED pattern management
     - `battery_manager.h/.cpp` - Power and battery monitoring
     - `sensor_manager.h/.cpp` - MPU-6050 and sensor handling
     - `button_handler.h/.cpp` - Button debouncing and actions
     - `game_engine.h/.cpp` - Mazeball game logic
     - `web_server.h/.cpp` - WiFi and web interface

2. **Task Management:**
   - Implement FreeRTOS tasks for parallel processing
   - Core 0: LED rendering and game logic
   - Core 1: WiFi, sensors, and button handling

3. **Memory Management:**
   - Optimize RAM usage for smooth operation
   - Implement efficient pattern storage
   - Use PROGMEM for static data

### Step 2.2: Battery Management System
**Tasks:**
1. **Auto-Dimming Implementation:**
   ```cpp
   // Brightness levels based on battery percentage
   100-75%: 255 brightness (100%)
   75-50%:  200 brightness (80%)
   50-25%:  150 brightness (60%)
   25-10%:  100 brightness (40%)
   <10%:    50 brightness (20%)
   ```

2. **Power Control:**
   - Implement TPS61088 PWM control for LED shutdown
   - Deep sleep modes for ESP32 during inactivity
   - Wake-on-button functionality

3. **Battery Display:**
   - 4x4 LED area in top-right corner for battery indicator
   - Color-coded levels (green/yellow/orange/red)
   - Automatic display during low battery
   - Manual toggle with button press

### Step 2.3: Pattern Engine Enhancement
**Tasks:**
1. **Existing Pattern Optimization:**
   - Optimize plasma blob for better performance
   - Enhance fire effect with realistic physics
   - Improve rainbow wave smoothness

2. **New Patterns:**
   - Gravity-responsive liquid simulation
   - Particle systems with collision
   - Conway's Game of Life
   - Perlin noise landscapes

3. **Pattern Management:**
   - Smooth transitions between patterns
   - Pattern preview in corner before full switch
   - Brightness scaling for battery conservation

**Phase 2 Completion Criteria:**
- [ ] Modular code architecture implemented
- [ ] Auto-dimming working smoothly
- [ ] Battery indicator functional
- [ ] Enhanced pattern library (10+ patterns)
- [ ] Stable multi-core operation

---

## PHASE 3: MAZEBALL GAME DEVELOPMENT
*Goal: Implement complete mazeball game with physics and level generation*

### Step 3.1: Physics Engine
**Tasks:**
1. **Ball Physics:**
   ```cpp
   struct Ball {
     float x, y;          // Position (sub-pixel precision)
     float vx, vy;        // Velocity
     float radius;        // Collision radius
     CRGB color;         // Ball color
     float friction;     // Surface friction
   };
   ```

2. **Gravity Implementation:**
   - Map MPU-6050 accelerometer to gravity vector
   - Apply gravity force to ball velocity
   - Implement momentum and inertia
   - Smooth gravity filtering to reduce noise

3. **Collision Detection:**
   - Ball-to-wall collision with proper bounce physics
   - Energy conservation during bounces
   - Friction and damping for realistic movement

### Step 3.2: Maze Generation
**Tasks:**
1. **Recursive Backtracking Algorithm:**
   ```cpp
   void generateMaze(int width, int height, int level) {
     // Increasing complexity with level
     // Ensure solvable path from start to goal
     // Add difficulty-based features
   }
   ```

2. **Progressive Difficulty:**
   - Level 1-5: Simple mazes, wide paths
   - Level 6-10: Narrow passages, dead ends
   - Level 11+: Moving walls, teleporters, time limits

3. **Visual Enhancement:**
   - Wall brightness based on ball proximity
   - Goal highlighting with pulsing animation
   - Path hints for very difficult levels

### Step 3.3: Game Logic
**Tasks:**
1. **Level Management:**
   - Procedural level generation
   - Level completion detection
   - Score system and high scores
   - Level celebration animations

2. **Game Modes:**
   - Classic: Complete maze to advance
   - Time Trial: Beat the clock
   - Free Play: Physics sandbox mode

3. **Game State Management:**
   - Menu system using LEDs
   - Game pause/resume functionality
   - Reset and restart options

**Phase 3 Completion Criteria:**
- [ ] Smooth ball physics with gyroscope control
- [ ] Infinite procedural maze generation
- [ ] Progressive difficulty scaling
- [ ] Complete game loop with scoring
- [ ] Multiple game modes functional

---

## PHASE 4: WEB INTERFACE & WIRELESS FEATURES
*Goal: Implement wireless configuration and monitoring capabilities*

### Step 4.1: WiFi and Web Server
**Tasks:**
1. **Network Configuration:**
   - Auto-connect to known networks
   - Fallback AP mode for setup
   - WiFi credential storage in EEPROM

2. **Web Interface Development:**
   - Remove brightness slider (auto-dimming only)
   - Add game control interface
   - Battery status dashboard
   - Pattern configuration panel

3. **Real-time Updates:**
   - WebSocket connection for live updates
   - Real-time battery monitoring
   - Game score display
   - Pattern preview

### Step 4.2: OTA Updates
**Tasks:**
1. **Firmware Update System:**
   - Web-based firmware upload
   - Version checking and rollback
   - Safe update process with verification

2. **Configuration Management:**
   - Save/load game settings
   - Pattern customization
   - Network configuration backup

**Phase 4 Completion Criteria:**
- [ ] Responsive web interface working
- [ ] OTA firmware updates functional
- [ ] Real-time status monitoring
- [ ] Game controls via web interface

---

## PHASE 5: ENCLOSURE DESIGN & FINAL ASSEMBLY
*Goal: Professional enclosure with integrated components*

### Step 5.1: 3D Design and Prototyping
**Tasks:**
1. **Measurements and CAD:**
   - Precise component measurements
   - Design compact 180x180x30mm enclosure
   - Integration of buttons, charging port, status LEDs

2. **Ventilation and Thermal:**
   - Heat dissipation for TPS61088
   - Airflow channels for ESP32
   - Component spacing optimization

3. **User Interface Design:**
   - Tactile button placement
   - USB-C port accessibility
   - Status indicator visibility

### Step 5.2: Manufacturing and Assembly
**Tasks:**
1. **3D Printing:**
   - Print test sections for fit verification
   - Iterate design based on test fits
   - Final production printing

2. **Final Assembly:**
   - Transfer all components to enclosure
   - Implement proper cable management
   - Secure mounting for all components

3. **Quality Testing:**
   - Full system functionality test
   - Drop test for durability
   - Thermal testing under full load
   - User experience validation

**Phase 5 Completion Criteria:**
- [ ] Professional-looking enclosure
- [ ] All components securely mounted
- [ ] Easy access for maintenance
- [ ] Passes durability testing

---

## Step-by-Step Wiring Guide

### Basic Connections (Phase 1)
1. **Power System:**
   ```
   Battery+ ──► BQ25606 BAT+ (charging path)
   Battery+ ──► TPS61088 VIN+ (power path)
   Battery- ──► Common GND
   TPS61088 VOUT+ ──► LED Panel VCC
   TPS61088 3.7V ──► ESP32 VIN
   ```

2. **Communication:**
   ```
   ESP32 GPIO21 ──► MPU-6050 SDA, Max17048 SDA
   ESP32 GPIO22 ──► MPU-6050 SCL, Max17048 SCL
   ESP32 GPIO16 ──► 330Ω ──► LED Panel DIN
   ```

3. **Control:**
   ```
   ESP32 GPIO25 ──► TPS61088 PWM
   ESP32 GPIO26 ──► Battery voltage divider
   ESP32 GPIO18,19,23 ──► Buttons (with pull-ups)
   ```

### Advanced Connections (Later Phases)
- Status LEDs for charging indication
- External antenna for better WiFi range (optional)
- Expansion connector for future features

## Testing Protocol

### Component Testing Sequence
1. **Individual Component Test:**
   - ESP32 programming and basic operation
   - LED panel basic patterns
   - Sensor I2C communication
   - Battery charging verification

2. **Integration Testing:**
   - Power system under full load
   - Multi-component operation
   - Thermal performance
   - Battery life verification

3. **Software Testing:**
   - Pattern smoothness and transitions
   - Game physics accuracy
   - Web interface responsiveness
   - Button response timing

### Performance Benchmarks
- **Battery Life:** 6+ hours continuous operation
- **Frame Rate:** 60fps for smooth animations
- **Response Time:** <10ms button to LED response
- **Network:** <1s web page load time
- **Boot Time:** <3s from power-on to operational

## Safety Checklist

### Electrical Safety
- [ ] All power connections properly insulated
- [ ] No exposed high-current conductors
- [ ] Proper fusing and overcurrent protection
- [ ] Battery BMS functional

### Mechanical Safety
- [ ] Secure component mounting
- [ ] No sharp edges or pinch points
- [ ] Proper strain relief for cables
- [ ] Drop-test validation

### Thermal Safety
- [ ] Adequate ventilation for power components
- [ ] Thermal monitoring in software
- [ ] Emergency shutdown capability
- [ ] Component temperature limits respected

## Troubleshooting Guide

### Common Issues and Solutions

#### Power Problems
- **ESP32 not booting:** Check battery voltage and connections
- **LEDs flickering:** Verify 5V supply stability under load
- **Short battery life:** Check for excessive current draw

#### Communication Issues
- **I2C sensors not found:** Verify SDA/SCL connections and pull-ups
- **LED patterns not displaying:** Check data line and ground connections
- **Buttons not responsive:** Verify GPIO assignments and pull-up configuration

#### Performance Issues
- **Slow pattern updates:** Optimize code for ESP32 dual-core architecture
- **Jerky animations:** Implement proper timing and frame rate control
- **WiFi connectivity problems:** Check antenna placement and signal strength

### Debug Tools
- Serial monitor for runtime debugging
- I2C scanner for device detection
- Multimeter for voltage/current measurement
- Oscilloscope for signal integrity (advanced debugging)

## Timeline Estimation

- **Phase 1:** 1-2 weekends (breadboard prototype)
- **Phase 2:** 2-3 weekends (software architecture)
- **Phase 3:** 2-4 weekends (game development)
- **Phase 4:** 1-2 weekends (web interface)
- **Phase 5:** 2-3 weekends (enclosure and assembly)

**Total: 8-14 weekends** depending on complexity and feature scope

This ESP32-based build plan provides a more streamlined path to completion while adding significant new functionality like the mazeball game and intelligent power management. 