/*
 * GitHub Client Module
 * Handles fetching GitHub activity data via HTTP API
 */

#ifndef GITHUB_CLIENT_H
#define GITHUB_CLIENT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// GitHub API configuration
#define GITHUB_API_HOST "api.github.com"
#define GITHUB_UPDATE_INTERVAL_MS (15 * 1000)  // 15 seconds in milliseconds
#define GITHUB_USERNAME "chalabi2"

// GitHub activity data structure
struct GitHubContribution {
  String date;
  int count;
};

// Function declarations
void initializeGitHubClient();
void updateGitHubData();
void setGitHubPatternActive(bool active);
void drawGitHubLoadingAnimation();
bool fetchGitHubContributions(const String& username);
bool processProxyResponse(const String& jsonResponse);
void clearGitHubGrid();

unsigned long getLastGitHubUpdate();
bool isGitHubDataStale();

// External references
extern unsigned long lastGitHubUpdate;
extern bool gitHubUpdateInProgress;
extern bool gitHubPatternActive;
extern bool gitHubDataLoaded;
extern bool showGitHubLoading;

#endif // GITHUB_CLIENT_H 