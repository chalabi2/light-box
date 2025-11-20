# DYNAMIC LED PANEL PROJECT - ESP32 EDITION

## Project Vision
A 16×16 battery-powered addressable LED panel with motion sensing, button controls, and intelligent battery management. Features a gyroscope-controlled mazeball game, visual effects responding to tilt and orientation, and automatic power optimization. The device is housed in a custom 3D-printed frame and controlled via ESP32 with wireless configuration capabilities.

## Core Features

### 1. Interactive Mazeball Game
- **Gyroscope-Controlled Physics:**
  - Real-time ball movement using MPU-6050 accelerometer
  - Gravity simulation responds to panel orientation
  - Smooth ball physics with momentum and bouncing
  - Collision detection with maze walls
- **Infinite Level Generation:**
  - Procedurally generated mazes for endless gameplay
  - Increasing difficulty with level progression
  - Goal detection with celebration animations
  - Level completion effects and transitions
- **Game Modes:**
  - Classic mode: Navigate ball through generated mazes
  - Time trial: Complete levels within time limits
  - Free play: Ball physics sandbox with various effects

### 2. Visual Display Capabilities
- **16×16 WS2812B LED Matrix** (256 individually addressable pixels)
- **Pre-built Pattern Library:**
  - Ambient lighting modes (breathing, pulsing, gradient sweeps)
  - Geometric patterns (spirals, waves, cellular automata)
  - Weather-responsive effects (rain, snow, fire simulation)
  - Physics simulations (particle systems, liquid effects)
- **Motion-Responsive Effects:**
  - Gravity-based animations using gyroscope input
  - Tilt-activated transitions and pattern changes
  - Orientation-aware pattern rotation
- **Battery Status Integration:**
  - Visual battery indicator in corner LEDs
  - Color-coded battery levels (green/yellow/orange/red)
  - Low battery warning animations

### 3. Intelligent Power Management
- **Auto-Dimming System:**
  - Brightness automatically reduces as battery depletes
  - Maintains visual quality while extending runtime
  - 5 power levels: 100%, 80%, 60%, 40%, 20%
- **Smart Power Features:**
  - Hardware-level LED panel shutdown via TPS61088 PWM control
  - ESP32 deep sleep modes during inactivity
  - Wake-on-button for instant responsiveness
- **Battery Monitoring:**
  - Real-time battery percentage via Max17048 fuel gauge
  - Predictive runtime estimation
  - Critical battery warnings and graceful shutdown

### 4. Physical Control Interface
- **Three Button System:**
  - **Button 1:** Pattern/Mode cycling (short press), Settings menu (long press)
  - **Button 2:** Battery level display toggle, Brightness adjustment in settings
  - **Button 3:** Game mode toggle (Mazeball ↔ Patterns), Game reset
- **Responsive Controls:**
  - Interrupt-driven for immediate response
  - Debounced for reliable operation
  - Long press detection for additional functions
  - Visual feedback for button presses

### 5. Wireless Configuration Interface
- **ESP32 Web Server:**
  - Pattern selection and customization
  - Game settings and high scores
  - Device status monitoring
  - Battery health analytics
- **WiFi Connectivity:**
  - Automatic home network connection
  - Fallback AP mode for initial setup
  - OTA (Over-The-Air) firmware updates
- **Mobile-Optimized Interface:**
  - Responsive design for smartphones
  - Touch-friendly controls
  - Real-time preview of effects

## Technical Architecture

### Hardware Platform
- **Processing:** ESP32 dual-core @ 240MHz with integrated WiFi/Bluetooth
- **Power System:** 1S 10000mAh Li-ion with boost conversion and USB-C charging
- **Sensors:** I2C gyroscope/accelerometer for motion detection
- **Display:** Hardware-accelerated WS2812B LED matrix control
- **User Interface:** GPIO buttons with interrupt-driven response
- **Battery Management:** Dedicated fuel gauge IC with automatic monitoring

### Software Architecture
- **Real-Time OS:** FreeRTOS (built into ESP32 Arduino framework)
- **Multi-Core Processing:**
  - Core 0: LED rendering, pattern processing, game logic
  - Core 1: WiFi/web server, sensor reading, button handling
- **Modular Design:**
  - Separate files for game logic, patterns, battery management
  - Clean separation of concerns for maintainability
  - Object-oriented approach for game entities

### Power Efficiency Design
- **Hierarchical Power Management:**
  1. Auto-dimming based on battery level
  2. Pattern complexity reduction at low battery
  3. Frame rate reduction for power saving
  4. Hardware LED shutdown for emergency power conservation
- **Adaptive Performance:**
  - Dynamic frame rate adjustment
  - Simplified calculations at low battery
  - Background task prioritization based on power state

## Physical Design

### Compact Form Factor
- **Dimensions:** ~180mm × 180mm × 30mm (significantly reduced from Pi version)
- **Weight:** <500g including battery
- **Mounting Options:** Wall mount, desk stand, magnetic attachment

### 3D Printed Components
- **Main Frame:**
  - Integrated LED panel mounting with perfect alignment
  - ESP32 and component housing with easy access
  - Button integration with tactile feedback
  - Heat dissipation channels for power components
- **Battery Integration:**
  - Secure 10000mAh flat cell mounting
  - USB-C charging port accessibility
  - Battery status LED windows
  - Safety latches and protection

### User Experience Design
- **Intuitive Controls:**
  - Single button for common actions
  - Visual feedback for all interactions
  - Logical grouping of functions
- **Status Communication:**
  - LED-based battery indicator
  - Pattern preview on device
  - Clear charging status indication

## Game Design: Mazeball

### Core Mechanics
- **Physics Engine:**
  - Real-world gravity mapping to LED grid
  - Ball momentum and friction simulation
  - Collision response with energy conservation
  - Smooth sub-pixel movement for fluid motion

### Maze Generation Algorithm
- **Procedural Creation:**
  - Recursive backtracking for maze structure
  - Guaranteed solvable paths from start to finish
  - Adjustable complexity based on level
  - Varied start/end positions for replay value

### Progressive Difficulty
- **Level Scaling:**
  - Maze complexity increases with level number
  - Timer pressure introduced in higher levels
  - Special maze features (teleporters, moving walls)
  - Boss levels with unique challenges

### Visual Effects
- **Ball Rendering:**
  - Smooth color transitions based on speed
  - Trail effects for fast movement
  - Glow effects for enhanced visibility
- **Maze Visualization:**
  - Wall brightness based on proximity to ball
  - Dynamic lighting effects
  - Goal highlighting and celebration animations

## Development Implementation

### Phase 1: Hardware Integration ✓
- [x] ESP32 board setup and programming environment
- [x] LED panel connection and basic control
- [x] Power system integration and testing
- [x] Button wiring and interrupt configuration

### Phase 2: Core Systems
- [ ] Battery monitoring and auto-dimming implementation
- [ ] Pattern engine with smooth transitions
- [ ] Button control system with debouncing
- [ ] Basic web interface for configuration

### Phase 3: Mazeball Game Engine
- [ ] Physics engine for ball movement
- [ ] Maze generation algorithms
- [ ] Collision detection and response
- [ ] Level progression and scoring system

### Phase 4: Advanced Features
- [ ] Power optimization and sleep modes
- [ ] OTA update capability
- [ ] Advanced pattern library
- [ ] Performance analytics and monitoring

### Phase 5: Polish and Optimization
- [ ] 3D printed enclosure design and fabrication
- [ ] User interface refinement
- [ ] Performance optimization
- [ ] Documentation and user guide

## Unique Selling Points

### Compared to Raspberry Pi Version
- **60% Cost Reduction:** From $300+ to ~$150 total cost
- **5x Better Battery Life:** 4-8 hours vs 1.5-2 hours
- **10x Faster Boot:** Instant-on vs 30+ second boot time
- **Real-Time Performance:** No OS overhead for smooth animations
- **Integrated Wireless:** No additional dongles required

### Innovation Features
- **Adaptive Power Management:** First LED panel with intelligent auto-dimming
- **Physical Game Controller:** Gyroscope-based gaming without external controllers
- **Infinite Content:** Procedural maze generation provides endless gameplay
- **Professional Integration:** Hardware power control and battery monitoring

## Target Applications

### Entertainment
- **Personal Gaming Device:** Handheld maze game with visual effects
- **Party Interactive Display:** Group games and ambient lighting
- **Desktop Art Piece:** Dynamic patterns for workspace enhancement

### Educational
- **Programming Learning:** Arduino/ESP32 development platform
- **Physics Demonstration:** Real gravity simulation and motion physics
- **Electronics Education:** Complete embedded system example

### Professional
- **Prototype Platform:** Base for custom LED art installations
- **Conference Display:** Attention-grabbing booth decoration
- **Product Demonstration:** Showcase for embedded development capabilities

## Future Enhancement Roadmap

### Short Term (3-6 months)
- **Sound Integration:** Piezo buzzer for game audio feedback
- **Multiplayer Support:** Bluetooth connectivity for competitive gameplay
- **Advanced Patterns:** Audio-reactive visualizations with microphone

### Medium Term (6-12 months)
- **Mobile App:** Dedicated smartphone application with enhanced controls
- **Cloud Integration:** High score sharing and pattern synchronization
- **Custom Maze Editor:** Web-based tool for creating custom maze levels

### Long Term (1+ years)
- **Network Clustering:** Multiple panels synchronized for larger displays
- **AI Integration:** Machine learning for adaptive difficulty and pattern generation
- **IoT Platform:** Home automation integration and smart device connectivity

## Success Metrics

### Performance Targets
- **Battery Life:** 6+ hours continuous gameplay
- **Response Time:** <10ms input latency for button presses
- **Frame Rate:** 60fps smooth animation at all battery levels
- **Boot Time:** <2 seconds from power-on to operational

### User Experience Goals
- **Intuitive Operation:** No manual required for basic use
- **Reliability:** 99.9% uptime during normal operation
- **Portability:** Complete functionality without external dependencies
- **Engagement:** 30+ minutes average play session for mazeball game

This ESP32-based redesign transforms the project from a complex, expensive display into an affordable, portable gaming device while maintaining all the visual appeal of the original concept. 