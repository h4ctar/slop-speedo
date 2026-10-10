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

// Display functions: short presses cycle; the needle always shows the speed
// (speed arrives with the speed-sensor TODO). Only CLOCK renders live data so
// far; the others render their name until their TODO items land.
enum { FUNCTION_SPEED, FUNCTION_CLOCK, FUNCTION_VOLT, FUNCTION_ODO, FUNCTION_TRIP, FUNCTION_COUNT };
static const char *const functionNames[FUNCTION_COUNT] = {"SPEED", "CLOCK", "VOLT", "ODO", "TRIP"};
static uint8_t functionIndex = FUNCTION_SPEED;
static uint32_t longFlashUntil = 0;

// Clock set mode: long press in the CLOCK function enters it. Short presses
// increment the field shown (H = hours, M = minutes), long presses advance
// H -> M -> exit, and 10 s without a press exits too. Every increment is
// written straight to the RTC, so the clock is live while editing and the
// result survives resets (the BKP_DR1 flag skips the boot-time re-set).
typedef enum { SET_OFF, SET_HOURS, SET_MINUTES } SetMode;
static SetMode setMode = SET_OFF;
static uint32_t setModeActivity = 0;
#define SET_MODE_TIMEOUT_MS 10000

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
}

void loop() {
  updateMotor();

  RTC_TimeTypeDef sTime;
  if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
    return; // skip this pass; set mode must not write back a bad read
  }

  char timeString[8];
  snprintf(timeString, sizeof(timeString), "%02d:%02d", sTime.Hours, sTime.Minutes);

  ButtonEvent event = pollButton();
  uint32_t now = HAL_GetTick();

  if (setMode != SET_OFF) {
    if (event == BUTTON_SHORT) {
      if (setMode == SET_HOURS) {
        sTime.Hours = (sTime.Hours + 1) % 24;
      } else {
        sTime.Minutes = (sTime.Minutes + 1) % 60;
      }
      HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      setModeActivity = now;
    } else if (event == BUTTON_LONG) {
      setMode = (setMode == SET_HOURS) ? SET_MINUTES : SET_OFF;
      setModeActivity = now;
    } else if (now - setModeActivity >= SET_MODE_TIMEOUT_MS) {
      setMode = SET_OFF; // abandoned: the edited time is already in the RTC
    }
  } else {
    switch (event) {
    case BUTTON_SHORT:
      functionIndex = (functionIndex + 1) % FUNCTION_COUNT;
      break;
    case BUTTON_LONG:
      if (functionIndex == FUNCTION_CLOCK) {
        setMode = SET_HOURS;
        setModeActivity = now;
      } else {
        longFlashUntil = now + 1500; // stub until the other functions land
      }
      break;
    default:
      break;
    }
  }

  if (setMode == SET_HOURS) {
    char setString[8];
    snprintf(setString, sizeof(setString), "H %02u", sTime.Hours);
    showString(setString);
  } else if (setMode == SET_MINUTES) {
    char setString[8];
    snprintf(setString, sizeof(setString), "M %02u", sTime.Minutes);
    showString(setString);
  } else if (HAL_GetTick() < longFlashUntil) {
    showString("LONG");
  } else if (functionIndex == FUNCTION_CLOCK) {
    showString(timeString);
  } else {
    showString(functionNames[functionIndex]);
  }
}
