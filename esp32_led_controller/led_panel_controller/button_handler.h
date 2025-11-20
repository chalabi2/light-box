/*
 * Button Handler Module
 * Manages button input, debouncing, and callback functions
 */

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "config.h"

// Button states
enum ButtonState {
  BUTTON_RELEASED,
  BUTTON_PRESSED,
  BUTTON_LONG_PRESSED
};

// Button structure for tracking state
struct Button {
  int pin;
  ButtonState state;
  ButtonState lastState;
  unsigned long pressTime;
  unsigned long lastDebounceTime;
  bool longPressTriggered;
};

// Button instances
extern Button button1;  // Pattern/Mode cycling
extern Button button2;  // Battery display toggle
extern Button button3;  // Game mode toggle

// Function declarations
void initializeButtonHandler();
void handleButtons();
void updateButton(Button* btn);
bool isButtonPressed(int pin);

// Button callback functions (implemented in main file)
extern void onButton1ShortPress();
extern void onButton1LongPress();
extern void onButton2ShortPress();
extern void onButton2LongPress();
extern void onButton3ShortPress();
extern void onButton3LongPress();

// Interrupt handlers
void IRAM_ATTR button1ISR();
void IRAM_ATTR button2ISR();
void IRAM_ATTR button3ISR();

#endif // BUTTON_HANDLER_H 