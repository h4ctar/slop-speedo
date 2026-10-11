#include "clock.h"

#include <stdio.h>

#include "rtc.h"
#include "screen.h"
#include "stm32f1xx_hal.h"

// Set mode: every increment is written straight to the RTC, so the clock is
// live while editing and the result survives resets (the BKP_DR1 flag skips
// the boot-time re-set).
typedef enum { SET_OFF, SET_HOURS, SET_MINUTES } SetMode;
static SetMode setMode = SET_OFF;
static uint32_t setModeActivity = 0;
#define SET_MODE_TIMEOUT_MS 10000

static void initClock() {
}

static bool updateClock(ButtonEvent event) {
  uint32_t now = HAL_GetTick();

  if (setMode == SET_OFF) {
    if (event == BUTTON_SHORT) {
      return true; // cycle to the next function
    }
    if (event == BUTTON_LONG) {
      setMode = SET_HOURS;
      setModeActivity = now;
    }
  } else {
    if (event == BUTTON_SHORT) {
      // Read-modify-write the field being edited.
      RTC_TimeTypeDef sTime;
      if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK) {
        if (setMode == SET_HOURS) {
          sTime.Hours = (sTime.Hours + 1) % 24;
        } else {
          sTime.Minutes = (sTime.Minutes + 1) % 60;
        }
        HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      }
      setModeActivity = now;
    } else if (event == BUTTON_LONG) {
      setMode = (setMode == SET_HOURS) ? SET_MINUTES : SET_OFF;
      setModeActivity = now;
    } else if (now - setModeActivity >= SET_MODE_TIMEOUT_MS) {
      setMode = SET_OFF; // abandoned: the edited time is already in the RTC
    }
  }

  RTC_TimeTypeDef sTime;
  if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
    return false; // skip this pass; set mode must not show a bad read
  }
  char str[8];
  if (setMode == SET_HOURS) {
    snprintf(str, sizeof(str), "H %02u", sTime.Hours);
  } else if (setMode == SET_MINUTES) {
    snprintf(str, sizeof(str), "M %02u", sTime.Minutes);
  } else {
    snprintf(str, sizeof(str), "%02d:%02d", sTime.Hours, sTime.Minutes);
  }
  showString(str);
  return false;
}

const Function clockFunction = {
    .init = initClock,
    .update = updateClock,
    .name = "CLOCK",
};
