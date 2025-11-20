/*
 * Pattern Engine Module Implementation
 * Handles all LED patterns and visual effects
 */

#include "pattern_engine.h"

// Pattern state variables
PatternType currentPattern = PATTERN_PLASMA_BLOB;
unsigned long lastPatternUpdate = 0;

PlasmaBlob blob;
RainDrop rainDrops[MAX_RAINDROPS];
uint8_t fireBuffer[MATRIX_HEIGHT][MATRIX_WIDTH];
Star stars[MAX_STARS];
float waveTime = 0;
uint16_t rainbowOffset = 0;
GitHubActivity githubActivity;

void initializePatterns() {
  // Initialize plasma blob
  blob.x = MATRIX_WIDTH / 2.0;
  blob.y = MATRIX_HEIGHT / 2.0;
  blob.vx = 0;
  blob.vy = 0;
  blob.size = 3.0;
  blob.color = CHSV(160, 255, 255);
  blob.intensity = 255;
  
  // Initialize rain drops
  for (int i = 0; i < MAX_RAINDROPS; i++) {
    rainDrops[i].active = false;
  }
  
  // Initialize fire buffer
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      fireBuffer[y][x] = 0;
    }
  }
  
  // Initialize stars
  for (int i = 0; i < MAX_STARS; i++) {
    stars[i].x = random(-MATRIX_WIDTH, MATRIX_WIDTH * 2);
    stars[i].y = random(-MATRIX_HEIGHT, MATRIX_HEIGHT * 2);
    stars[i].z = random(1, 15);
    stars[i].brightness = random(50, 255) / 255.0;
  }
  
  // Only initialize GitHub activity if it hasn't been initialized yet
  static bool githubInitialized = false;
  if (!githubInitialized) {
    setGitHubData("");
    githubInitialized = true;
  }
}

void updateCurrentPattern() {
  // Pattern functions handle their own clearing to prevent double-clear glitches
  
  switch (currentPattern) {
    case PATTERN_PLASMA_BLOB:
      updatePlasmaBlob();
      drawPlasmaBlob();
      break;
      
    case PATTERN_RAIN_MATRIX:
      updateRainMatrix();
      drawRainMatrix();
      break;
      

      
    case PATTERN_RAINBOW_WAVE:
      updateRainbowWave();
      break;
      
    case PATTERN_STARFIELD:
      updateStarfield();
      break;
      
    case PATTERN_RIPPLES:
      updateRipples();
      break;
      
    case PATTERN_GITHUB_ACTIVITY:
      updateGitHubActivity();
      break;
      
    case PATTERN_OFF:
      clearLEDs();
      break;
  }
}

void updatePlasmaBlob() {
  // Apply gravity to blob velocity
  float gravityStrength = 0.15;
  blob.vx += gravityX * gravityStrength;
  blob.vy += gravityY * gravityStrength;
  
  // Apply damping
  blob.vx *= 0.98;
  blob.vy *= 0.98;
  
  // Update position
  blob.x += blob.vx;
  blob.y += blob.vy;
  
  // Bounce off walls
  if (blob.x <= blob.size) {
    blob.x = blob.size;
    blob.vx = -blob.vx * 0.7;
  }
  if (blob.x >= MATRIX_WIDTH - blob.size) {
    blob.x = MATRIX_WIDTH - blob.size;
    blob.vx = -blob.vx * 0.7;
  }
  if (blob.y <= blob.size) {
    blob.y = blob.size;
    blob.vy = -blob.vy * 0.7;
  }
  if (blob.y >= MATRIX_HEIGHT - blob.size) {
    blob.y = MATRIX_HEIGHT - blob.size;
    blob.vy = -blob.vy * 0.7;
  }
  
  // Change color over time
  uint8_t hue = (millis() / 100) % 255;
  blob.color = CHSV(hue, 200, 255);
}

void drawPlasmaBlob() {
  clearLEDs();
  
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      float distance = sqrt((x - blob.x) * (x - blob.x) + (y - blob.y) * (y - blob.y));
      
      if (distance < blob.size * 2) {
        float intensity = exp(-distance * distance / (blob.size * blob.size));
        intensity = constrain(intensity, 0.0, 1.0);
        
        uint8_t r = (blob.color.r * intensity);
        uint8_t g = (blob.color.g * intensity);
        uint8_t b = (blob.color.b * intensity);
        
        setLED(x, y, CRGB(r, g, b));
      }
    }
  }
}

void updateRainMatrix() {
  // Determine gravity direction to find "up" side of panel
  float absGravityX = abs(gravityX);
  float absGravityY = abs(gravityY);
  
  // Spawn new raindrops from the "up" edge based on gravity
  static unsigned long lastRainSpawn = 0;
  if (millis() - lastRainSpawn > 150) {
    for (int i = 0; i < MAX_RAINDROPS; i++) {
      if (!rainDrops[i].active) {
        // Determine spawn position based on strongest gravity component
        if (absGravityY > absGravityX) {
          // Gravity is primarily vertical
          rainDrops[i].x = random(MATRIX_WIDTH);
          if (gravityY > 0) {
            // Gravity points down, spawn from top
            rainDrops[i].y = -1;
          } else {
            // Gravity points up, spawn from bottom
            rainDrops[i].y = MATRIX_HEIGHT;
          }
        } else {
          // Gravity is primarily horizontal
          rainDrops[i].y = random(MATRIX_HEIGHT);
          if (gravityX > 0) {
            // Gravity points right, spawn from left
            rainDrops[i].x = -1;
          } else {
            // Gravity points left, spawn from right
            rainDrops[i].x = MATRIX_WIDTH;
          }
        }
        
        rainDrops[i].velocity = 0.2 + random(50) / 100.0;
        rainDrops[i].brightness = 150 + random(105);
        rainDrops[i].active = true;
        lastRainSpawn = millis();
        break;
      }
    }
  }
  
  // Update existing raindrops - move in direction of gravity
  for (int i = 0; i < MAX_RAINDROPS; i++) {
    if (rainDrops[i].active) {
      // Apply gravity directly with velocity multiplier
      rainDrops[i].x += gravityX * (rainDrops[i].velocity + 0.2);
      rainDrops[i].y += gravityY * (rainDrops[i].velocity + 0.2);
      
      // Remove raindrops that go off any edge
      bool offScreen = false;
      if (rainDrops[i].x < -2 || rainDrops[i].x >= MATRIX_WIDTH + 2 ||
          rainDrops[i].y < -2 || rainDrops[i].y >= MATRIX_HEIGHT + 2) {
        offScreen = true;
      }
      
      if (offScreen) {
        rainDrops[i].active = false;
      }
      
      // Fade raindrops over time
      rainDrops[i].brightness = max(0, rainDrops[i].brightness - 1);
      if (rainDrops[i].brightness <= 10) {
        rainDrops[i].active = false;
      }
    }
  }
}

void drawRainMatrix() {
  // Fade existing pixels
  for (int i = 0; i < NUM_LEDS; i++) {
    displayBuffer[i].fadeToBlackBy(40);
  }
  
  // Determine gravity direction for trail effect
  float absGravityX = abs(gravityX);
  float absGravityY = abs(gravityY);
  
  // Draw active raindrops
  for (int i = 0; i < MAX_RAINDROPS; i++) {
    if (rainDrops[i].active) {
      int x = (int)rainDrops[i].x;
      int y = (int)rainDrops[i].y;
      
      // Only draw if within bounds
      if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
        CRGB color = CHSV(160, 255, rainDrops[i].brightness);
        addLED(x, y, color);
        
        // Trail effect opposite to gravity direction
        for (int j = 1; j <= 3; j++) {
          int trailX = x;
          int trailY = y;
          
          // Calculate trail position opposite to gravity
          if (absGravityY > absGravityX) {
            // Primarily vertical gravity
            if (gravityY > 0) {
              trailY = y - j; // Trail goes up when gravity points down
            } else {
              trailY = y + j; // Trail goes down when gravity points up
            }
          } else {
            // Primarily horizontal gravity
            if (gravityX > 0) {
              trailX = x - j; // Trail goes left when gravity points right
            } else {
              trailX = x + j; // Trail goes right when gravity points left
            }
          }
          
          // Draw trail pixel if within bounds
          if (trailX >= 0 && trailX < MATRIX_WIDTH && 
              trailY >= 0 && trailY < MATRIX_HEIGHT) {
            uint8_t trailBrightness = rainDrops[i].brightness / (j + 1);
            CRGB trailColor = CHSV(160, 255, trailBrightness);
            addLED(trailX, trailY, trailColor);
          }
        }
      }
    }
  }
}

void updateFire() {
  // Add heat at the bottom
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    fireBuffer[MATRIX_HEIGHT-1][x] = random(180, 255);
  }
  
  // Cool down and rise
  for (int y = MATRIX_HEIGHT-2; y >= 0; y--) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      int heatSum = 0;
      int count = 0;
      
      for (int dx = -1; dx <= 1; dx++) {
        for (int dy = 0; dy <= 1; dy++) {
          int nx = x + dx;
          int ny = y + dy;
          if (nx >= 0 && nx < MATRIX_WIDTH && ny >= 0 && ny < MATRIX_HEIGHT) {
            heatSum += fireBuffer[ny][nx];
            count++;
          }
        }
      }
      
      int avgHeat = heatSum / count;
      int cooling = random(5, 20);
      fireBuffer[y][x] = max(0, avgHeat - cooling);
    }
  }
}

void drawFire() {
  clearLEDs();
  
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      uint8_t heat = fireBuffer[y][x];
      
      CRGB color;
      if (heat < 64) {
        color = CRGB(heat * 4, 0, 0);
      } else if (heat < 128) {
        color = CRGB(255, (heat - 64) * 4, 0);
      } else if (heat < 192) {
        color = CRGB(255, 255, (heat - 128) * 3);
      } else {
        uint8_t whiteAmount = (heat - 192) * 4;
        color = CRGB(255, 255, min(255, 200 + whiteAmount));
      }
      
      setLED(x, y, color);
    }
  }
}

void updateRainbowWave() {
  clearLEDs();
  
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      float wave = sin((x * 0.4) + (y * 0.4) + (waveTime * 0.1));
      uint8_t hue = ((x * 15) + (y * 15) + (int)(wave * 60) + rainbowOffset) % 255;
      
      float intensity = (sin(waveTime * 0.05 + x * 0.3 + y * 0.3) + 1) / 2;
      uint8_t brightness = 50 + intensity * 200;
      
      CRGB color = CHSV(hue, 255, brightness);
      setLED(x, y, color);
    }
  }
  
  rainbowOffset += 2;
  waveTime += 1;
}

void updateStarfield() {
  clearLEDs();
  
  for (int i = 0; i < MAX_STARS; i++) {
    stars[i].z -= 0.15;
    if (stars[i].z <= 0) {
      stars[i].z = 15;
      stars[i].x = random(-MATRIX_WIDTH, MATRIX_WIDTH * 2);
      stars[i].y = random(-MATRIX_HEIGHT, MATRIX_HEIGHT * 2);
      stars[i].brightness = random(50, 255) / 255.0;
    }
    
    int screenX = (int)((stars[i].x - MATRIX_WIDTH/2) / stars[i].z * 8 + MATRIX_WIDTH/2);
    int screenY = (int)((stars[i].y - MATRIX_HEIGHT/2) / stars[i].z * 8 + MATRIX_HEIGHT/2);
    
    if (screenX >= 0 && screenX < MATRIX_WIDTH && screenY >= 0 && screenY < MATRIX_HEIGHT) {
      float brightness = stars[i].brightness / stars[i].z * 8;
      brightness = min(1.0f, brightness);
      
      uint8_t colorValue = (uint8_t)(255 * brightness);
      CRGB color = CRGB(colorValue, colorValue, colorValue);
      setLED(screenX, screenY, color);
    }
  }
}

void updateRipples() {
  clearLEDs();
  
  float centerX = MATRIX_WIDTH / 2.0;
  float centerY = MATRIX_HEIGHT / 2.0;
  float timeFactor = millis() * 0.003;
  
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      float distance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
      
      float ripple = sin(distance * 0.8 - timeFactor * 3) * 0.5 + 0.5;
      float ripple2 = sin(distance * 0.4 - timeFactor * 4.5) * 0.3 + 0.5;
      
      float combined = (ripple + ripple2) / 2;
      
      uint8_t hue = (uint8_t)((distance * 20 + timeFactor * 50)) % 255;
      uint8_t brightness = (uint8_t)(combined * 255);
      
      CRGB color = CHSV(hue, 255, brightness);
      setLED(x, y, color);
    }
  }
}

void updateGitHubActivity() {
  clearLEDs();
  
  // Check if we should show loading animation
  extern bool showGitHubLoading;
  extern void drawGitHubLoadingAnimation();
  
  static unsigned long lastDebugOutput = 0;
  if (millis() - lastDebugOutput > 5000) { // Debug every 5 seconds
    Serial.printf("🎨 GitHub Activity Pattern - Loading: %s, Data Age: %lu ms\n", 
                  showGitHubLoading ? "YES" : "NO", 
                  millis() - githubActivity.lastUpdate);
    lastDebugOutput = millis();
  }
  
  if (showGitHubLoading) {
    // Show loading animation on first visit
    drawGitHubLoadingAnimation();
  } else {
    // Show the contribution graph
    drawGitHubContributions();
  }
}



void drawGitHubContributions() {
  // Draw GitHub-style contribution calendar
  // 16x16 grid = 256 days (about 8.5 months of data)
  // Most recent on right, oldest on left
  
  static unsigned long lastDebugPrint = 0;
  
  // Print debug info every 30 seconds
  if (millis() - lastDebugPrint > 30000) {
    Serial.println("📅 Drawing GitHub contribution calendar...");
    
    // Count intensity levels for debugging
    int intensityCounts[5] = {0, 0, 0, 0, 0};
    int totalContributions = 0;
    
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      for (int y = 0; y < MATRIX_HEIGHT; y++) {
        uint8_t intensity = githubActivity.contributionData[y][x];
        if (intensity <= 4) {
          intensityCounts[intensity]++;
          totalContributions += intensity;
        }
      }
    }
    
    Serial.printf("📊 Calendar Stats - Total: %d contributions\n", totalContributions);
    Serial.printf("📊 Distribution: None=%d, Low=%d, Med=%d, High=%d, Max=%d\n",
                  intensityCounts[0], intensityCounts[1], intensityCounts[2], 
                  intensityCounts[3], intensityCounts[4]);
    
    // Show recent activity (rightmost column)
    Serial.printf("📈 Recent activity (last 7 days): ");
    for (int y = 0; y < 7; y++) {
      Serial.printf("%d ", githubActivity.contributionData[y][15]);
    }
    Serial.println();
    
    lastDebugPrint = millis();
  }
  
  // Draw the full contribution calendar (all 16x16 grid)
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) { // Use full 16x16 grid
      uint8_t intensity = githubActivity.contributionData[y][x];
      
      CRGB color;
      
      // 4 brightness levels of green + black for no contributions
      switch (intensity) {
        case 0:
          color = CRGB::Black;        // No contributions - empty/black
          break;
        case 1:
          color = CRGB(0, 80, 0);     // Low activity - dim green
          break;
        case 2:
          color = CRGB(0, 140, 0);    // Medium activity - medium green
          break;
        case 3:
          color = CRGB(0, 200, 0);    // High activity - bright green
          break;
        case 4:
        default:
          color = CRGB(0, 255, 0);    // Max activity - full bright green
          break;
      }
      
      setLED(x, y, color);
    }
  }
}



void setGitHubData(const String& jsonData) {
  // Parse JSON data and populate contribution matrix
  // This is a simplified parser - in production you'd use a proper JSON library
  
  githubActivity.username = "chalabi2"; // Set default username
  githubActivity.lastUpdate = millis();
  
  Serial.printf("🎲 setGitHubData called with %d bytes of data\n", jsonData.length());
  
  // Initialize with sample data if no real data provided
  if (jsonData.length() == 0) {
    Serial.println("📊 Generating sample GitHub contribution data...");
    
    // Generate sample contribution data representing 256 days (16x16 grid)
    // Arrange chronologically: oldest on left (x=0), newest on right (x=15)
    // Each column represents ~16 days, each row represents a day within that period
    
    int totalGenerated = 0;
    int intensityCount[5] = {0, 0, 0, 0, 0};
    
    Serial.println("📊 Generating chronological contribution data...");
    Serial.println("   Layout: Oldest (left) → Newest (right)");
    Serial.println("   Each column ≈ 16 days, 256 total days");
    
    for (int x = 0; x < 16; x++) {      // Columns = time periods (oldest to newest)
      for (int y = 0; y < 16; y++) {    // Rows = days within each period (full 16x16 grid)
        // Calculate day number (0 = oldest, 255 = newest)
        int dayNumber = x * 16 + y;
        
        // Simulate realistic GitHub activity patterns
        int dayOfWeek = dayNumber % 7;
        int intensity = 0;
        
        // More activity on weekdays
        if (dayOfWeek < 5) { // Monday-Friday
          intensity = random(0, 5);
        } else { // Weekend
          intensity = random(0, 2);
        }
        
        // Add some streaks and patterns
        if (dayNumber > 50 && dayNumber < 80) {
          intensity = min(4, intensity + 2); // Active period
        }
        if (dayNumber > 150 && dayNumber < 180) {
          intensity = min(4, intensity + 1); // Another active period
        }
        
        // Recent activity boost (last 30 days)
        if (dayNumber > 225) {
          intensity = min(4, intensity + 1);
        }
        
        githubActivity.contributionData[y][x] = intensity;
        intensityCount[intensity]++;
        totalGenerated++;
        
        // Add watchdog yield every few iterations to prevent crashes
        if ((x * 16 + y) % 32 == 0) {
          yield();
        }
      }
    }
    
    Serial.printf("✅ Sample data generated - Distribution: 0=%d, 1=%d, 2=%d, 3=%d, 4=%d\n",
                  intensityCount[0], intensityCount[1], intensityCount[2], 
                  intensityCount[3], intensityCount[4]);
    
    // Show first few values for verification
    Serial.printf("🔍 First 8 values: ");
    for (int i = 0; i < 8; i++) {
      Serial.printf("%d ", githubActivity.contributionData[0][i]);
    }
    Serial.println();
    
  } else {
    Serial.println("📡 Processing real GitHub API data...");
    // TODO: Parse actual JSON data from GitHub API
    // For now, just use sample data
    Serial.println("⚠️ JSON parsing not implemented yet, using sample data");
  }
} 