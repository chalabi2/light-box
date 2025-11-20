/*
 * GitHub Client Module
 * Handles fetching GitHub activity data via local proxy server
 */

#include "github_client.h"
#include "pattern_engine.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Global variables
unsigned long lastGitHubUpdate = 0;
bool gitHubUpdateInProgress = false;
bool gitHubPatternActive = false;
bool gitHubDataLoaded = false;
bool showGitHubLoading = false;

void initializeGitHubClient() {
  Serial.println("[INFO] Initializing GitHub Client...");
  
  // Initialize GitHub activity data structure
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      githubActivity.contributionData[y][x] = 0;
    }
  }
  
  githubActivity.username = GITHUB_USERNAME;
  githubActivity.lastUpdate = 0;
  
  Serial.println("[INFO] GitHub Client initialized successfully");
}

void updateGitHubData() {
  // Only update GitHub data when the GitHub pattern is active
  if (!gitHubPatternActive) {
    return;
  }
  
  if (gitHubUpdateInProgress) {
    return;
  }
  
  if (!isGitHubDataStale()) {
    return;
  }
  
  gitHubUpdateInProgress = true;
  
  Serial.printf("🔄 Updating GitHub data for user: %s\n", GITHUB_USERNAME);
  
  bool success = fetchGitHubContributions(GITHUB_USERNAME);
  
  if (success) {
    Serial.println("✅ GitHub data updated successfully");
    gitHubDataLoaded = true;
    showGitHubLoading = false; // Turn off loading animation after successful data fetch
  } else {
    Serial.println("❌ GitHub data update failed");
  }
  
  gitHubUpdateInProgress = false;
}

void setGitHubPatternActive(bool active) {
  gitHubPatternActive = active;
  
  if (active) {
    Serial.println("🎯 GitHub pattern activated");
    
    if (!gitHubDataLoaded) {
      Serial.println("📡 First visit - showing loading animation");
      showGitHubLoading = true;
    }
    
    // Trigger immediate update if data is stale
    if (isGitHubDataStale()) {
      updateGitHubData();
    }
  } else {
    Serial.println("🎯 GitHub pattern deactivated - stopping all GitHub activity");
    showGitHubLoading = false;
    gitHubUpdateInProgress = false; // Stop any ongoing updates
  }
}

void drawGitHubLoadingAnimation() {
  // Simple loading animation while fetching data
  static unsigned long lastUpdate = 0;
  static int loadingStep = 0;
  
  if (millis() - lastUpdate > 200) {
    // Clear grid
    for (int y = 0; y < 16; y++) {
      for (int x = 0; x < 16; x++) {
        githubActivity.contributionData[y][x] = 0;
      }
    }
    
    // Draw loading pattern
    int centerX = 8;
    int centerY = 8;
    int radius = (loadingStep % 8) + 1;
    
    for (int y = 0; y < 16; y++) {
      for (int x = 0; x < 16; x++) {
        int distance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
        if (distance == radius) {
          githubActivity.contributionData[y][x] = 2;
        }
      }
    }
    
    loadingStep++;
    lastUpdate = millis();
  }
}

bool fetchGitHubContributions(const String& username) {
  Serial.printf("🚀 Starting GitHub data update for user: %s\n", username.c_str());
  
  // Check WiFi connection first
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("❌ WiFi not connected (status: %d). Cannot fetch GitHub data.\n", WiFi.status());
    return false;
  }
  
  Serial.printf("📡 WiFi connected (%s). Fetching from local proxy server\n", WiFi.localIP().toString().c_str());
  
  HTTPClient http;
  // Use local proxy server instead of GitHub API
  String url = "http://192.168.0.209:8080/github-activity/" + username + "/raw";
  
  Serial.printf("📄 Fetching from proxy: %s\n", url.c_str());
  Serial.printf("📱 Free heap before request: %d bytes\n", ESP.getFreeHeap());
  
  // Configure HTTP client with better settings
  http.begin(url);
  http.addHeader("Accept", "application/json");
  http.addHeader("User-Agent", "ESP32-LED-Panel/1.0");
  http.addHeader("Connection", "close");  // Ensure connection is closed after request
  http.setTimeout(15000); // Increased timeout to 15 seconds
  http.setConnectTimeout(5000); // Set connection timeout to 5 seconds
  
  int httpCode = http.GET();
  Serial.printf("📊 HTTP Response Code: %d\n", httpCode);
  
  // Add detailed error information for HTTP code -1
  if (httpCode == -1) {
    Serial.printf("❌ HTTP connection failed. Possible causes:\n");
    Serial.printf("   - Server not reachable at %s\n", url.c_str());
    Serial.printf("   - DNS resolution failed\n");
    Serial.printf("   - Connection timeout\n");
    Serial.printf("   - WiFi interference or weak signal\n");
    Serial.printf("📶 WiFi signal strength: %d dBm\n", WiFi.RSSI());
  }
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.printf("📊 Response length: %d bytes\n", response.length());
    Serial.printf("📄 Response preview: %s\n", response.substring(0, 100).c_str());
    
    // Trim whitespace from response
    response.trim();
    
    Serial.printf("🔍 After trim - starts with '[': %s, ends with ']': %s\n", 
                  response.startsWith("[") ? "YES" : "NO",
                  response.endsWith("]") ? "YES" : "NO");
    
    if (response.length() > 10) {
      Serial.printf("🔍 Last 10 chars: '%s'\n", response.substring(response.length()-10).c_str());
    }
    
    // Parse JSON array directly
    if (response.startsWith("[") && response.endsWith("]")) {
      bool success = processProxyResponse(response);
      if (success) {
        Serial.printf("✅ GitHub data updated from proxy server\n");
        return true;
      } else {
        Serial.printf("❌ Failed to process proxy response\n");
      }
    } else {
      Serial.printf("❌ Invalid response format from proxy\n");
      Serial.printf("❌ Response starts with: '%c', ends with: '%c'\n", 
                    response.length() > 0 ? response.charAt(0) : '?',
                    response.length() > 0 ? response.charAt(response.length()-1) : '?');
    }
  } else {
    Serial.printf("❌ HTTP Error %d from proxy server\n", httpCode);
  }
  
  http.end();
  Serial.printf("📱 Free heap after request: %d bytes\n", ESP.getFreeHeap());
  
  // Clear grid on failure
  clearGitHubGrid();
  return false;
}

bool processProxyResponse(const String& jsonResponse) {
  Serial.printf("🔍 Processing proxy response (%d bytes)...\n", jsonResponse.length());
  
  // Clear existing GitHub activity data first to prevent stale data
  Serial.printf("🧹 Clearing existing GitHub data before processing new response\n");
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      githubActivity.contributionData[y][x] = 0;
    }
  }
  
  // Parse JSON array: [0,1,2,0,3,1,0,0,2,1,3,0,1,2,0,0...]
  // Remove brackets and split by commas
  String data = jsonResponse;
  data.replace("[", "");
  data.replace("]", "");
  data.replace(" ", "");
  
  int position = 0;
  int arrayIndex = 0;
  
  while (position < data.length() && arrayIndex < 256) {
    int commaPos = data.indexOf(',', position);
    if (commaPos == -1) commaPos = data.length();
    
    String valueStr = data.substring(position, commaPos);
    int intensity = valueStr.toInt();
    
    // Map array index to grid position (left-to-right, top-to-bottom)
    int x = arrayIndex % 16;  // Column (0-15)
    int y = arrayIndex / 16;  // Row (0-11)
    
    githubActivity.contributionData[y][x] = intensity;
    
    arrayIndex++;
    position = commaPos + 1;
  }
  
  // Fill remaining positions with 0 if we have fewer than 256 values
  while (arrayIndex < 256) {
    int x = arrayIndex % 16;
    int y = arrayIndex / 16;
    githubActivity.contributionData[y][x] = 0;
    arrayIndex++;
  }
  
  githubActivity.username = GITHUB_USERNAME;
  githubActivity.lastUpdate = millis();
  
  int activeDays = 0;
  for (int i = 0; i < 256; i++) {
    int x = i % 16;
    int y = i / 16;
    if (githubActivity.contributionData[y][x] > 0) activeDays++;
  }
  
  Serial.printf("📊 Processed 256-day calendar: %d active days\n", activeDays);
  return true;
}

void clearGitHubGrid() {
  Serial.printf("🧹 Clearing GitHub activity grid\n");
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      githubActivity.contributionData[y][x] = 0;
    }
  }
  githubActivity.username = GITHUB_USERNAME;
  githubActivity.lastUpdate = millis();
}

unsigned long getLastGitHubUpdate() {
  return githubActivity.lastUpdate;
}

bool isGitHubDataStale() {
  return (millis() - githubActivity.lastUpdate) > GITHUB_UPDATE_INTERVAL_MS;
} 