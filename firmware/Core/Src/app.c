#include <screen.h>
#include "app.h"
#include "button.h"
#include "motor.h"
#include "tim.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>

// What is currently on the OLED: loop() redraws only when the value changes,
// which keeps loop() fast (good for step timing) and stops constant I2C
// traffic. Starts empty so the first loop() paints the screen.
static char displayedString[8];

// Stub UI until the display functions exist: short presses cycle the function
// names, a long press flashes "LONG" for a second and a half (each function's
// real long-press action lands with that function's TODO item). Only CLOCK
// renders live data (the time); the rest render their name.
enum { FUNCTION_SPEED, FUNCTION_CLOCK, FUNCTION_VOLT, FUNCTION_ODO, FUNCTION_TRIP, FUNCTION_COUNT };
static const char *const functionNames[FUNCTION_COUNT] = {"SPEED", "CLOCK", "VOLT", "ODO", "TRIP"};
static uint8_t functionIndex = FUNCTION_SPEED;
static uint32_t longFlashUntil = 0;

static void showString(const char *str) {
  if (strcmp(str, displayedString) != 0) {
    clearScreen();
    displayString((char *)str);
    // Cache only after a successful flush, so a failed I2C write retries on
    // the next loop() instead of leaving a stale screen for a minute.
    if (updateScreen() == HAL_OK) {
      strcpy(displayedString, str);
    }
  }
}

void setup() {
  HAL_TIM_Base_Start(&htim1);
  initButton();
  initMotor();
  initScreen();
  // RTC_TimeTypeDef sTime;
  // sTime.Hours = 12;
  // sTime.Minutes = 34;
  // sTime.Seconds = 0;
  // HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}

void loop() {
  updateMotor();

  RTC_TimeTypeDef sTime;
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

  char timeString[8];
  snprintf(timeString, sizeof(timeString), "%02d:%02d", sTime.Hours, sTime.Minutes);

  switch (pollButton()) {
  case BUTTON_SHORT:
    functionIndex = (functionIndex + 1) % FUNCTION_COUNT;
    break;
  case BUTTON_LONG:
    longFlashUntil = HAL_GetTick() + 1500;
    break;
  default:
    break;
  }

  if (HAL_GetTick() < longFlashUntil) {
    showString("LONG");
  } else if (functionIndex == FUNCTION_CLOCK) {
    showString(timeString);
  } else {
    showString(functionNames[functionIndex]);
  }
}
