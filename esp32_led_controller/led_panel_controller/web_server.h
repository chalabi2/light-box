/*
 * Web Server Module
 * Handles WiFi connection and web interface for remote control
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

// External references to web server
extern WebServer server;

// Function declarations
void setupWebServer();
String getI2CDeviceList();
int scanI2CDevices();

#endif // WEB_SERVER_H 