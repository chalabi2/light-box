/*
 * Pattern Engine Module
 * Handles all LED patterns and visual effects
 */

#ifndef PATTERN_ENGINE_H
#define PATTERN_ENGINE_H

#include <Arduino.h>
#include <FastLED.h>

// Hardware definitions (if not included elsewhere)
#ifndef NUM_LEDS
#define NUM_LEDS 256
#endif
#ifndef MATRIX_WIDTH
#define MATRIX_WIDTH 16
#endif
#ifndef MATRIX_HEIGHT
#define MATRIX_HEIGHT 16
#endif
#ifndef MAX_RAINDROPS
#define MAX_RAINDROPS 32
#endif
#ifndef MAX_STARS
#define MAX_STARS 40
#endif

// Pattern types enumeration
enum PatternType {
  PATTERN_PLASMA_BLOB,
  PATTERN_RAIN_MATRIX,
  PATTERN_RAINBOW_WAVE,
  PATTERN_STARFIELD,
  PATTERN_RIPPLES,
  PATTERN_GITHUB_ACTIVITY,
  PATTERN_OFF
};

// Pattern-specific data structures
struct PlasmaBlob {
  float x, y, vx, vy, size;
  CRGB color;
  uint8_t intensity;
};

struct RainDrop {
  float x, y, velocity;
  uint8_t brightness;
  bool active;
};

struct Star {
  float x, y, z, brightness;
};

struct GitHubActivity {
  uint8_t contributionData[16][16];  // Contribution intensity (0-4 scale)
  unsigned long lastUpdate;
  bool showProfile;
  uint8_t profileScrollOffset;
  String username;
};

// External variables
extern PatternType currentPattern;
extern unsigned long lastPatternUpdate;
extern float gravityX, gravityY;

// Pattern state variables
extern PlasmaBlob blob;
extern RainDrop rainDrops[MAX_RAINDROPS];
extern uint8_t fireBuffer[MATRIX_HEIGHT][MATRIX_WIDTH];
extern Star stars[MAX_STARS];
extern float waveTime;
extern uint16_t rainbowOffset;
extern GitHubActivity githubActivity;

// External LED control functions
extern CRGB displayBuffer[NUM_LEDS];
void clearLEDs();
void setLED(int x, int y, CRGB color);
void addLED(int x, int y, CRGB color);

// Function declarations
void initializePatterns();
void updateCurrentPattern();

// Individual pattern functions
void updatePlasmaBlob();
void drawPlasmaBlob();
void updateRainMatrix();
void drawRainMatrix();
void updateFire();
void drawFire();
void updateRainbowWave();
void updateStarfield();
void updateRipples();
void updateGitHubActivity();
void drawGitHubContributions();

void setGitHubData(const String& jsonData);

#endif // PATTERN_ENGINE_H 