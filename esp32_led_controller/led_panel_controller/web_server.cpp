/*
 * Web Server Module Implementation
 * Handles WiFi connection and web interface for remote control
 */

#include "web_server.h"
#include "config.h"
#include "led_control.h"

// Hardware definitions now in config.h
#ifndef BUTTON_PIN_1
#define BUTTON_PIN_1 14
#endif
#ifndef BUTTON_PIN_2
#define BUTTON_PIN_2 27
#endif
#ifndef BUTTON_PIN_3
#define BUTTON_PIN_3 26
#endif

// External variable declarations
extern bool isCharging;

extern float gravityX, gravityY;

// Pattern type declarations
enum PatternType {
  PATTERN_PLASMA_BLOB,
  PATTERN_RAIN_MATRIX,
  PATTERN_RAINBOW_WAVE,
  PATTERN_STARFIELD,
  PATTERN_RIPPLES,
  PATTERN_GITHUB_ACTIVITY,
  PATTERN_OFF
};
extern PatternType currentPattern;

// Forward declarations for battery functions
float getBatteryPercentage();
float getBatteryVoltage();
bool isFuelGaugeWorking();

// Web server instance  
WebServer server(80);

// LED Painter variables
bool painterMode = false;
CRGB painterGrid[MATRIX_HEIGHT][MATRIX_WIDTH];
uint8_t painterBrightness = 255;

int scanI2CDevices() {
  int deviceCount = 0;
  
  for (int address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      deviceCount++;
    }
  }
  
  return deviceCount;
}

String getI2CDeviceList() {
  String deviceList = "Found I2C devices: ";
  int deviceCount = 0;
  
  for (int address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      if (deviceCount > 0) deviceList += ", ";
      deviceList += "0x" + String(address, HEX);
      
      // Identify known devices
      if (address == 0x68) deviceList += " (MPU6050)";
      else if (address == 0x36) deviceList += " (Max17048)";
      
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    deviceList = "No I2C devices found! Check SDA/SCL wiring.";
  }
  
  return deviceList;
}

void setupWebServer() {
  // Complete WiFi reset to fix state machine issues
  Serial.println("🔧 Initializing WiFi...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(2000); // Longer delay for complete reset
  
  // Use externally defined credentials to avoid duplication
  extern const char* home_ssid;
  extern const char* home_password;
  extern const char* ap_ssid;
  extern const char* ap_password;
  
  Serial.printf("🌐 Connecting to WiFi: %s\n", HOME_WIFI_SSID);
  
  // Set WiFi to station mode and configure - fixes "cannot set config" error
  WiFi.mode(WIFI_STA);
  delay(100); // Critical delay after mode change
  
  // Disable auto features that can cause state conflicts
  WiFi.setAutoReconnect(false);
  // Note: setAutoConnect() not available in ESP32 Arduino Core 3.3.0
  delay(100);
  
  // Additional ESP32-specific settings to prevent connection issues
  WiFi.setSleep(false); // Disable WiFi sleep mode
  delay(100);
  
  Serial.printf("📋 WiFi Mode: %d, Status: %d\n", WiFi.getMode(), WiFi.status());
  
  // Begin connection with timeout
  Serial.println("🔌 Starting WiFi connection...");
  WiFi.begin(HOME_WIFI_SSID, HOME_WIFI_PASSWORD);
  delay(500); // Allow connection to initialize
  
  // More robust connection attempts with proper state checking
  int attempts = 0;
  const int maxAttempts = 20; // Reduced max attempts
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(1000);
    Serial.print(".");
    attempts++;
    
    // Check WiFi status and reset if stuck
    if (attempts % 5 == 0) {
      wl_status_t status = WiFi.status();
      Serial.printf("\n🔍 WiFi Status: %d (attempt %d/%d)\n", status, attempts, maxAttempts);
      
      // If stuck in connecting state, force reset
      if (status == WL_DISCONNECTED || status == WL_CONNECTION_LOST || status == WL_NO_SSID_AVAIL) {
        Serial.println("🔄 WiFi stuck - performing hard reset...");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(1000);
        WiFi.mode(WIFI_STA);
        delay(500);
        WiFi.begin(HOME_WIFI_SSID, HOME_WIFI_PASSWORD);
      }
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ WiFi connected to home network!");
    Serial.printf("📡 IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("📶 Signal strength: %d dBm\n", WiFi.RSSI());
  } else {
    Serial.println();
    Serial.printf("❌ Failed to connect to '%s'\n", HOME_WIFI_SSID);
    Serial.printf("🔍 Final WiFi status: %d\n", WiFi.status());
    Serial.println("📋 Possible issues:");
    Serial.println("   - Incorrect WiFi credentials");
    Serial.println("   - WiFi network not in range");
    Serial.println("   - Router security settings");
    Serial.println("   - ESP32 antenna/hardware issue");
    Serial.println();
    Serial.println("🏠 Starting AP mode instead...");
    
    // Ensure clean transition to AP mode
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(1000);
    WiFi.mode(WIFI_AP);
    
    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
    if (apStarted) {
      Serial.println("✅ WiFi AP started successfully");
      Serial.printf("📍 AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
      Serial.printf("📡 Connect to network: %s\n", AP_SSID);
      Serial.printf("🔑 Password: %s\n", AP_PASSWORD);
    } else {
      Serial.println("❌ Failed to start AP mode");
    }
  }
  
  // Serve control page
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><title>LED Panel Controller</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;text-align:center;background:#1a1a1a;color:white;}";
    html += ".container{max-width:400px;margin:0 auto;padding:20px;}";
    html += "button{padding:15px 30px;margin:10px;font-size:16px;border:none;border-radius:5px;cursor:pointer;}";
    html += ".pattern-btn{background:#4CAF50;color:white;}";
    html += ".game-btn{background:#FF9800;color:white;}";
    html += ".status{margin:20px 0;padding:10px;background:#333;border-radius:5px;}</style></head>";
    html += "<body><div class='container'><h1>LED Panel Controller</h1>";
    html += "<div class='status'><h3>Mode: <span id='mode'>Pattern</span></h3>";
    html += "<p>Battery: <span id='battery'>Loading...</span>%</p>";
    html += "<p>Gravity: <span id='gravity'>Loading...</span></p>";
    html += "<p>GitHub: <span id='github-status'>Loading...</span></p></div>";
    html += "<h3>Brightness Control</h3>";
    html += "<button class='game-btn' onclick='cycleBrightness()'>Cycle Brightness Level</button>";
    html += "<p id='brightness-info'>Current: AUTO (battery controlled)</p>";
    html += "<h3>Patterns</h3>";
    html += "<button class='pattern-btn' onclick='setPattern(\"plasma\")'>Plasma Blob</button>";
    html += "<button class='pattern-btn' onclick='setPattern(\"rain\")'>Rain Matrix</button>";
    html += "<button class='pattern-btn' onclick='setPattern(\"rainbow\")'>Rainbow Wave</button>";
    html += "<button class='pattern-btn' onclick='setPattern(\"starfield\")'>Starfield</button>";
    html += "<button class='pattern-btn' onclick='setPattern(\"ripples\")'>Ripples</button>";
    html += "<button class='pattern-btn' onclick='setPattern(\"github\")'>GitHub Activity</button>";
    html += "<button class='pattern-btn' onclick='setPattern(\"off\")'>Off</button>";
    html += "<h3>LED Painter</h3>";
    html += "<a href='/painter' style='display:inline-block;padding:15px 30px;margin:10px;background:#9C27B0;color:white;text-decoration:none;border-radius:5px;'>LED Painter</a>";
    html += "<h3>Diagnostics</h3>";
    html += "<a href='/diagnostics' style='display:inline-block;padding:15px 30px;margin:10px;background:#666;color:white;text-decoration:none;border-radius:5px;'>System Diagnostics</a>";
    html += "<h3>Emergency Tools</h3>";
    html += "<button onclick='resetFuelGauge()' style='background:#ff4444;'>Reset Fuel Gauge</button>";
    html += "</div><script>";
    html += "function setPattern(pattern){fetch('/pattern?type='+pattern).then(()=>updateStatus());}";
    html += "function cycleBrightness(){fetch('/brightness').then(()=>updateStatus());}";
    html += "function updateStatus(){fetch('/status').then(r=>r.json()).then(data=>{";
    html += "document.getElementById('mode').textContent=data.mode;";
    html += "document.getElementById('battery').textContent=data.battery;";
    html += "document.getElementById('gravity').textContent='X:'+data.gravityX+' Y:'+data.gravityY;";
    html += "var githubStatus='Updated '+data.githubLastUpdate+'s ago';";
    html += "if(data.githubStale==='true')githubStatus+=' (stale)';";
    html += "document.getElementById('github-status').textContent=githubStatus;";
    html += "document.getElementById('brightness-info').textContent='Current: '+data.brightnessMode+' ('+data.currentBrightness+')';});}";
        html += "function resetFuelGauge(){if(confirm('WARNING: This will reset fuel gauge learning data. Continue?')){fetch('/reset-fuel-gauge').then(r=>r.text()).then(result=>alert(result));}}";  
    html += "setInterval(updateStatus,2000);updateStatus();</script></body></html>";
    
    server.send(200, "text/html", html);
  });
  
  // Pattern control
  server.on("/pattern", []() {
    String type = server.arg("type");
    if (type == "plasma") currentPattern = PATTERN_PLASMA_BLOB;
    else if (type == "rain") currentPattern = PATTERN_RAIN_MATRIX;
    else if (type == "rainbow") currentPattern = PATTERN_RAINBOW_WAVE;
    else if (type == "starfield") currentPattern = PATTERN_STARFIELD;
    else if (type == "ripples") currentPattern = PATTERN_RIPPLES;
    else if (type == "github") currentPattern = PATTERN_GITHUB_ACTIVITY;
    else if (type == "off") currentPattern = PATTERN_OFF;
    
    // Notify GitHub client if switching to/from GitHub pattern
    extern void setGitHubPatternActive(bool active);
    setGitHubPatternActive(currentPattern == PATTERN_GITHUB_ACTIVITY);
    
    painterMode = false; // Exit painter mode when selecting patterns
    server.send(200, "text/plain", "OK");
  });
  
  // Brightness control
  server.on("/brightness", []() {
    extern uint8_t manualBrightnessLevel;
    
    // Define brightness levels locally to avoid linker issues
    const uint8_t brightnessLevels[5] = {0, 80, 140, 180, 220};
    
    // Cycle through brightness levels
    manualBrightnessLevel = (manualBrightnessLevel + 1) % 5;
    
    if (manualBrightnessLevel == 0) {
      // Auto brightness mode
      updateAutoDimming();
    } else {
      // Manual brightness mode - respect battery limits
      float batteryPercentage = getBatteryPercentage();
      uint8_t maxAllowedBrightness = getBatteryLimitedMaxBrightness(batteryPercentage);
      uint8_t targetBrightness = brightnessLevels[manualBrightnessLevel];
      
      if (targetBrightness > maxAllowedBrightness) {
        targetBrightness = maxAllowedBrightness;
      }
      
      setBrightness(targetBrightness);
    }
    
    server.send(200, "text/plain", "OK");
  });

  // LED Painter page
  server.on("/painter", []() {
    String html = "<!DOCTYPE html><html><head><title>LED Panel Painter</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial;text-align:center;background:#1a1a1a;color:white;margin:0;padding:10px;}";
    html += ".container{max-width:700px;margin:0 auto;}";
    html += ".grid{display:grid;grid-template-columns:repeat(16,25px);grid-gap:1px;justify-content:center;margin:20px auto;background:#333;padding:5px;border-radius:5px;user-select:none;}";
    html += ".led{width:25px;height:25px;border:1px solid #666;cursor:pointer;border-radius:2px;background:#000000;}";
    html += ".led:hover{border-color:#fff;}";
    html += ".controls{margin:20px 0;}";
    html += ".color-palette{display:grid;grid-template-columns:repeat(12,35px);gap:5px;justify-content:center;margin:15px auto;max-width:450px;}";
    html += ".color-btn{width:35px;height:35px;border:2px solid #666;cursor:pointer;border-radius:5px;}";
    html += ".color-btn.active{border-color:#fff;box-shadow:0 0 10px rgba(255,255,255,0.5);}";
    html += ".brightness-control{margin:20px 0;}";
    html += ".brightness-slider{width:200px;margin:0 10px;}";
    html += "button{padding:10px 20px;margin:5px;border:none;border-radius:5px;cursor:pointer;background:#4CAF50;color:white;}";
    html += "button:hover{background:#45a049;}";
    html += ".clear-btn{background:#f44336;}";
    html += ".clear-btn:hover{background:#da190b;}";
    html += ".status{margin:10px 0;padding:8px;background:#333;border-radius:5px;font-size:14px;}";
    html += "</style></head>";
    html += "<body><div class='container'>";
    html += "<h1>LED Panel Painter</h1>";
    html += "<p>Select color and paint by clicking or dragging across LEDs</p>";
    
    // Full WS2812B color palette (based on COMPONENT_LIST.md - WS2812B with GRB color order)
    html += "<div class='controls'>";
    html += "<div style='margin:10px 0;'><strong>Color Palette:</strong></div>";
    html += "<div class='color-palette'>";
    
    // Essential colors - organized by groups
    String colors[] = {
      "#000000", // Black (off)
      "#ffffff", // White (max power)
      "#ff0000", // Red
      "#00ff00", // Green  
      "#0000ff", // Blue
      "#ffff00", // Yellow (red+green)
      "#ff00ff", // Magenta (red+blue)
      "#00ffff", // Cyan (green+blue)
      "#ff8000", // Orange
      "#8000ff", // Purple
      "#00ff80", // Spring green
      "#ff0080", // Hot pink
      "#80ff00", // Lime
      "#0080ff", // Sky blue
      "#ff8080", // Light red
      "#80ff80", // Light green
      "#8080ff", // Light blue
      "#ffff80", // Light yellow
      "#ff80ff", // Light magenta
      "#80ffff", // Light cyan
      "#804000", // Brown
      "#408000", // Olive
      "#004080", // Navy
      "#800040", // Maroon
      "#400080", // Indigo
      "#008040", // Dark green
      "#ff4040", // Bright red
      "#40ff40", // Bright green
      "#4040ff", // Bright blue
      "#808080", // Gray
      "#404040", // Dark gray
      "#c0c0c0", // Silver
      "#ffcc00", // Gold
      "#ff6600", // Orange red
      "#66ff00", // Yellow green
      "#0066ff"  // Blue cyan
    };
    
    for (int i = 0; i < 36; i++) {
      html += "<button class='color-btn' style='background:" + colors[i] + "' onclick='selectColor(\"" + colors[i] + "\")'></button>";
    }
    
    html += "</div></div>";
    
    // 16x16 LED grid
    html += "<div class='grid' id='led-grid'>";
    for (int y = 0; y < 16; y++) {
      for (int x = 0; x < 16; x++) {
        html += "<div class='led' id='led_" + String(x) + "_" + String(y) + "' onmousedown='startPaint(" + String(x) + "," + String(y) + ")' onmouseenter='continuePaint(" + String(x) + "," + String(y) + ")' onmouseup='stopPaint()'></div>";
      }
    }
    html += "</div>";
    
    // Controls
    html += "<div class='controls'>";
    html += "<div class='brightness-control'>";
    html += "<label>Base Brightness: <span id='brightness-value'>100</span>%</label><br>";
    html += "<input type='range' class='brightness-slider' id='brightness' min='10' max='100' value='100' onchange='updateBrightness()'>";
    html += "<div style='font-size:12px;color:#aaa;margin-top:5px;'>Auto-adjusts down for white LEDs</div>";
    html += "</div>";
    html += "<button onclick='clearAll()' class='clear-btn'>Clear All</button>";
    html += "<button onclick='window.location.href=\"/\"'>Back to Control</button>";
    html += "</div>";
    
    html += "<div class='status' id='status'>Ready - Paint mode active</div>";
    
    html += "</div>";
    
    // JavaScript for live painting
    html += "<script>";
    html += "let selectedColor = '#ff0000';";
    html += "let gridData = {};";
    html += "let isPainting = false;";
    html += "let lastBrightness = 100;";
    
    html += "function selectColor(color) {";
    html += "  selectedColor = color;";
    html += "  document.querySelectorAll('.color-btn').forEach(btn => btn.classList.remove('active'));";
    html += "  event.target.classList.add('active');";
    html += "}";
    
    html += "function startPaint(x, y) {";
    html += "  isPainting = true;";
    html += "  paintLED(x, y);";
    html += "}";
    
    html += "function continuePaint(x, y) {";
    html += "  if (isPainting) paintLED(x, y);";
    html += "}";
    
    html += "function stopPaint() {";
    html += "  isPainting = false;";
    html += "}";
    
    html += "function paintLED(x, y) {";
    html += "  gridData[x + '_' + y] = selectedColor;";
    html += "  document.getElementById('led_' + x + '_' + y).style.backgroundColor = selectedColor;";
    html += "  applyToPanel();"; // Instant update
    html += "}";
    
    html += "function updateBrightness() {";
    html += "  const brightness = document.getElementById('brightness').value;";
    html += "  document.getElementById('brightness-value').textContent = brightness;";
    html += "  lastBrightness = brightness;";
    html += "  applyToPanel();"; // Update panel when brightness changes
    html += "}";
    
    html += "function clearAll() {";
    html += "  gridData = {};";
    html += "  document.querySelectorAll('.led').forEach(led => led.style.backgroundColor = '#000000');";
    html += "  applyToPanel();"; // Clear panel immediately
    html += "}";
    
    html += "function applyToPanel() {";
    html += "  const brightness = document.getElementById('brightness').value || lastBrightness;";
    html += "  const data = JSON.stringify({grid: gridData, brightness: brightness});";
    html += "  fetch('/painter-apply', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: data})";
    html += "    .then(response => response.text())";
    html += "    .then(result => {";
    html += "      document.getElementById('status').textContent = result;";
    html += "    }).catch(err => {";
    html += "      document.getElementById('status').textContent = 'Update failed';";
    html += "    });";
    html += "}";
    
    // Prevent context menu and text selection during painting
    html += "document.addEventListener('contextmenu', e => e.preventDefault());";
    html += "document.addEventListener('selectstart', e => e.preventDefault());";
    html += "document.addEventListener('mouseup', stopPaint);";
    
    html += "selectColor('#ff0000');"; // Default to red
    html += "</script>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
  });

  // LED Painter apply endpoint - now handles live updates
  server.on("/painter-apply", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      
      // Parse JSON manually (simple parsing for this use case)
      int gridStart = body.indexOf("\"grid\":{") + 8;
      int gridEnd = body.indexOf("},\"brightness\"");
      int brightnessStart = body.indexOf("\"brightness\":\"") + 14;
      int brightnessEnd = body.indexOf("\"", brightnessStart);
      
      if (gridStart > 8 && gridEnd > gridStart && brightnessStart > 14 && brightnessEnd > brightnessStart) {
        String gridData = body.substring(gridStart, gridEnd);
        int brightness = body.substring(brightnessStart, brightnessEnd).toInt();
        
        // Clear the painter grid first
        for (int y = 0; y < MATRIX_HEIGHT; y++) {
          for (int x = 0; x < MATRIX_WIDTH; x++) {
            painterGrid[y][x] = CRGB::Black;
          }
        }
        
        // Count white LEDs for power management
        int whiteLEDCount = 0;
        int totalLEDs = 0;
        
        // Parse the grid data and apply colors
        int pos = 0;
        while (pos < gridData.length()) {
          int keyStart = gridData.indexOf("\"", pos);
          if (keyStart == -1) break;
          keyStart++;
          
          int keyEnd = gridData.indexOf("\"", keyStart);
          if (keyEnd == -1) break;
          
          String key = gridData.substring(keyStart, keyEnd);
          int valueStart = gridData.indexOf("\"#", keyEnd) + 1;
          int valueEnd = gridData.indexOf("\"", valueStart);
          
          if (valueStart > keyEnd && valueEnd > valueStart) {
            String colorHex = gridData.substring(valueStart, valueEnd);
            
            // Parse coordinates from key (format: "x_y")
            int underscorePos = key.indexOf("_");
            if (underscorePos > 0) {
              int x = key.substring(0, underscorePos).toInt();
              int y = key.substring(underscorePos + 1).toInt();
              
              if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
                // Parse hex color
                long colorValue = strtol(colorHex.substring(1).c_str(), NULL, 16);
                uint8_t r = (colorValue >> 16) & 0xFF;
                uint8_t g = (colorValue >> 8) & 0xFF;
                uint8_t b = colorValue & 0xFF;
                
                painterGrid[y][x] = CRGB(r, g, b);
                totalLEDs++;
                
                // Count white/bright LEDs for power management (only considers white, not battery level)
                if (r > 200 && g > 200 && b > 200) {
                  whiteLEDCount++;
                }
              }
            }
          }
          
          pos = valueEnd + 1;
        }
        
        // Smart power management ONLY for white LEDs (not battery-based)
        uint8_t baseBrightness = brightness * 255 / 100;
        uint8_t finalBrightness = baseBrightness;
        
        if (whiteLEDCount > 50) {
          // Many white LEDs - reduce brightness significantly to prevent overcurrent
          finalBrightness = baseBrightness * 40 / 100; // 40% max
        } else if (whiteLEDCount > 20) {
          // Some white LEDs - moderate reduction
          finalBrightness = baseBrightness * 70 / 100; // 70% max
        }
        // Note: Battery-based brightness is handled separately in main loop
        
        painterBrightness = finalBrightness;
        painterMode = true;
        
        // Build response without alerts/emojis
        String response = "Live update applied";
        if (totalLEDs > 0) {
          response += " (" + String(totalLEDs) + " LEDs";
          if (whiteLEDCount > 0) {
            response += ", " + String(whiteLEDCount) + " white";
            if (whiteLEDCount > 20) {
              response += " - brightness reduced for safety";
            }
          }
          response += ")";
        }
        
        server.send(200, "text/plain", response);
      } else {
        server.send(400, "text/plain", "Invalid data format");
      }
    } else {
      server.send(400, "text/plain", "No data received");
    }
  });

  // Status endpoint
  server.on("/status", []() {
    extern uint8_t manualBrightnessLevel;
    
    String json = "{";
    json += "\"mode\":\"Pattern\",";
    json += "\"battery\":" + String(getBatteryPercentage(), 1) + ",";
    json += "\"voltage\":" + String(getBatteryVoltage(), 2) + ",";
    json += "\"fuelGaugeWorking\":" + String(isFuelGaugeWorking() ? "true" : "false") + ",";
    json += "\"charging\":" + String(isCharging ? "true" : "false") + ",";
    json += "\"gravityX\":" + String(gravityX, 2) + ",";
    json += "\"gravityY\":" + String(gravityY, 2) + ",";
    
    // Add brightness status
    const char* levelNames[] = {"AUTO", "LOW", "MEDIUM", "HIGH", "MAX"};
    json += "\"brightnessMode\":\"" + String(levelNames[manualBrightnessLevel]) + "\",";
    json += "\"currentBrightness\":" + String(getCurrentBrightness()) + ",";
    
    // Add GitHub status
    extern unsigned long getLastGitHubUpdate();
    extern bool isGitHubDataStale();
    unsigned long lastUpdate = getLastGitHubUpdate();
    unsigned long timeSinceUpdate = (millis() - lastUpdate) / 1000; // seconds
    
    json += "\"githubLastUpdate\":" + String(timeSinceUpdate) + ",";
    json += "\"githubStale\":" + String(isGitHubDataStale() ? "true" : "false");
    json += "}";
    
    server.send(200, "application/json", json);
  });
  
  // Diagnostics endpoint
  server.on("/diagnostics", []() {
    String html = "<!DOCTYPE html><html><head><title>LED Panel Diagnostics</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;background:#1a1a1a;color:white;padding:20px;}";
    html += ".status{margin:10px 0;padding:10px;background:#333;border-radius:5px;}";
    html += ".good{border-left:5px solid #4CAF50;} .warning{border-left:5px solid #FF9800;} .error{border-left:5px solid #f44336;}</style></head>";
    html += "<body><h1>ESP32 LED Panel Diagnostics</h1>";
    
    // I2C Device Scan
    html += "<div class='status " + String((scanI2CDevices() > 0) ? "good" : "error") + "'>";
    html += "<h3>I2C Device Scan</h3>";
    html += "<p>" + getI2CDeviceList() + "</p>";
    html += "</div>";
    
    html += "<div class='status " + String(isFuelGaugeWorking() ? "good" : "error") + "'>";
    html += "<h3>Fuel Gauge Status</h3>";
    html += "<p>Working: " + String(isFuelGaugeWorking() ? "YES" : "NO (Using ADC fallback)") + "</p>";
    html += "</div>";
    
    html += "<div class='status " + String((getBatteryVoltage() > 2.5) ? "good" : "error") + "'>";
    html += "<h3>Battery Information</h3>";
    html += "<p>Voltage: " + String(getBatteryVoltage(), 2) + "V</p>";
    html += "<p>Percentage: " + String(getBatteryPercentage(), 1) + "%</p>";
    html += "<p>Source: " + String(isFuelGaugeWorking() ? "Max17048 Fuel Gauge" : "ADC Voltage Divider") + "</p>";
    html += "<p>Charging: " + String(isCharging ? "YES" : "NO") + "</p>";
    html += "<p>Raw ADC Reading: " + String(analogRead(BATTERY_ADC_PIN)) + " / 4095</p>";
    html += "</div>";
    
    html += "<div class='status good'>";
    html += "<h3>Button Status</h3>";
    html += "<p>Button 1 (Pattern): " + String(digitalRead(BUTTON_PIN_1) ? "Released" : "Pressed") + "</p>";
    html += "<p>Button 2 (Battery): " + String(digitalRead(BUTTON_PIN_2) ? "Released" : "Pressed") + "</p>";
    html += "<p>Button 3 (Game): " + String(digitalRead(BUTTON_PIN_3) ? "Released" : "Pressed") + "</p>";
    html += "</div>";
    
    html += "<div class='status good'>";
    html += "<h3>System Information</h3>";
    html += "<p>Free Heap: " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
    html += "<p>WiFi RSSI: " + String(WiFi.RSSI()) + " dBm</p>";
    html += "</div>";
    
    html += "<a href='/'>← Back to Control Panel</a>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
  });

  // Fuel gauge reset endpoint (emergency use only)
  server.on("/reset-fuel-gauge", []() {
    extern void resetFuelGaugeHardware();
    resetFuelGaugeHardware();
    server.send(200, "text/plain", "Fuel gauge reset complete. Battery readings should stabilize within 30 seconds.");
  });
  
  // GitHub activity data endpoint
  server.on("/github-data", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String jsonData = server.arg("plain");
      extern void setGitHubData(const String& jsonData);
      setGitHubData(jsonData);
      server.send(200, "text/plain", "GitHub data updated successfully");
    } else {
      server.send(400, "text/plain", "No JSON data provided");
    }
  });
  
  server.begin();
  Serial.println("Web server started");
} 