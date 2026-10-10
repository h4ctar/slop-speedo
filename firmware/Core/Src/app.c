#include <screen.h>
#include "app.h"
#include "motor.h"
#include "tim.h"
#include "rtc.h"
#include <stdio.h>

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
  RTC_TimeTypeDef sTime;
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

  char timeString[8];
  snprintf(timeString, sizeof(timeString), "%02d:%02d", sTime.Hours, sTime.Minutes);
  clearScreen();
  displayString(timeString);
  updateScreen();
}
