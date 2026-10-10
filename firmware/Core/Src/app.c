#include <screen.h>
#include "app.h"
#include "motor.h"
#include "tim.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>

// What is currently on the OLED: loop() redraws only when the value changes,
// which keeps loop() fast (good for step timing) and stops constant I2C
// traffic. Starts empty so the first loop() paints the screen.
static char displayedString[8];

void setup() {
  HAL_TIM_Base_Start(&htim1);
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
  if (strcmp(timeString, displayedString) != 0) {
    clearScreen();
    displayString(timeString);
    // Cache only after a successful flush, so a failed I2C write retries on
    // the next loop() instead of leaving a stale screen for a minute.
    if (updateScreen() == HAL_OK) {
      strcpy(displayedString, timeString);
    }
  }
}
