#include "clock.h"

#include <stdio.h>

#include "rtc.h"
#include "screen.h"
#include "stm32f1xx_hal.h"

// Set mode: every increment is written straight to the RTC, so the clock is
// live while editing and the result survives resets (the BKP_DR1 flag skips
// the boot-time re-set).
typedef enum { SET_OFF, SET_HOURS, SET_MINUTES } SetMode;
static SetMode set_mode = SET_OFF;
static uint32_t set_mode_activity_ms = 0;
#define SET_MODE_TIMEOUT_MS 10000

static void clock_init() {
}

static bool clock_update(ButtonEvent event) {
  uint32_t now = HAL_GetTick();

  if (set_mode == SET_OFF) {
    if (event == BUTTON_SHORT) {
      return true; // cycle to the next function
    }
    if (event == BUTTON_LONG) {
      set_mode = SET_HOURS;
      set_mode_activity_ms = now;
    }
  } else {
    if (event == BUTTON_SHORT) {
      // Read-modify-write the field being edited.
      RTC_TimeTypeDef s_time;
      if (HAL_RTC_GetTime(&hrtc, &s_time, RTC_FORMAT_BIN) == HAL_OK) {
        if (set_mode == SET_HOURS) {
          s_time.Hours = (s_time.Hours + 1) % 24;
        } else {
          s_time.Minutes = (s_time.Minutes + 1) % 60;
        }
        HAL_RTC_SetTime(&hrtc, &s_time, RTC_FORMAT_BIN);
      }
      set_mode_activity_ms = now;
    } else if (event == BUTTON_LONG) {
      set_mode = (set_mode == SET_HOURS) ? SET_MINUTES : SET_OFF;
      set_mode_activity_ms = now;
    } else if (now - set_mode_activity_ms >= SET_MODE_TIMEOUT_MS) {
      set_mode = SET_OFF; // abandoned: the edited time is already in the RTC
    }
  }

  RTC_TimeTypeDef s_time;
  if (HAL_RTC_GetTime(&hrtc, &s_time, RTC_FORMAT_BIN) != HAL_OK) {
    return false; // skip this pass; set mode must not show a bad read
  }
  char str[8];
  if (set_mode == SET_HOURS) {
    snprintf(str, sizeof(str), "H %02u", s_time.Hours);
  } else if (set_mode == SET_MINUTES) {
    snprintf(str, sizeof(str), "M %02u", s_time.Minutes);
  } else {
    snprintf(str, sizeof(str), "%02d:%02d", s_time.Hours, s_time.Minutes);
  }
  screen_show_string(str);
  return false;
}

const Function clock_function = {
    .init = clock_init,
    .update = clock_update,
    .name = "CLOCK",
};
