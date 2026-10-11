#include "clock.h"

#include <stdio.h>

#include "rtc.h"
#include "screen.h"
#include "stm32f1xx_hal.h"

// Set mode: every increment is written straight to the RTC, so the clock is
// live while editing and the result survives resets (the BKP_DR1 flag skips
// the boot-time re-set). The field being edited blinks; the blink phase
// restarts on every press so the increment is visible immediately.
typedef enum { SET_OFF, SET_HOURS, SET_MINUTES } SetMode;
static SetMode set_mode = SET_OFF;
static uint32_t set_mode_activity_ms = 0;
#define SET_MODE_TIMEOUT_MS 10000
#define BLINK_HALF_MS 400

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
  // In set mode the field being edited blinks (phase from the last press, so
  // an increment is always visible for BLINK_HALF_MS right after it).
  bool hide_field = set_mode != SET_OFF &&
                    (now - set_mode_activity_ms) / BLINK_HALF_MS % 2 == 1;
  char str[8];
  if (hide_field && set_mode == SET_HOURS) {
    snprintf(str, sizeof(str), "  :%02u", s_time.Minutes);
  } else if (hide_field && set_mode == SET_MINUTES) {
    snprintf(str, sizeof(str), "%02u:  ", s_time.Hours);
  } else {
    snprintf(str, sizeof(str), "%02d:%02d", s_time.Hours, s_time.Minutes);
  }
  screen_show_value(str, NULL);
  return false;
}

const Function clock_function = {
    .init = clock_init,
    .update = clock_update,
    .name = "CLOCK",
};
