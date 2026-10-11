#include "app.h"

#include "button.h"
#include "clock.h"
#include "function.h"
#include "motor.h"
#include "odo.h"
#include "settings.h"
#include "speed.h"
#include "tim.h"
#include "trip.h"
#include "volt.h"
#include "screen.h"
#include "stm32f1xx_hal.h"

// app.c only tracks which function is on screen: the MODE button cycles the
// registry below, and the selected function's update handles its own logic
// and rendering (see function.h). Everything that must run in every function
// (motor servicing, speed/needle) is called before the function update.
static const Function *const functions[] = {
    &speedFunction,
    &clockFunction,
    &voltFunction,
    &odoFunction,
    &tripFunction,
};
#define FUNCTION_COUNT (sizeof(functions) / sizeof(functions[0]))
static uint8_t functionIndex = 0;
static uint32_t nameFlashUntil = 0;
#define NAME_FLASH_MS 1000

void setup() {
  HAL_TIM_Base_Start(&htim1);
  initButton();
  initMotor();
  settings_init();
  for (uint8_t i = 0; i < FUNCTION_COUNT; i++) {
    functions[i]->init();
  }
  initScreen();
}

void loop() {
  updateMotor();
  speedTask();

  ButtonEvent event = pollButton();

  // While settling on a newly selected function, show its name. Cycling is
  // never blocked: a short press advances immediately, and a long press cuts
  // the flash short and is handed to the function.
  if ((int32_t)(HAL_GetTick() - nameFlashUntil) < 0) {
    if (event == BUTTON_SHORT) {
      functionIndex = (functionIndex + 1) % FUNCTION_COUNT;
      nameFlashUntil = HAL_GetTick() + NAME_FLASH_MS;
    } else if (event == BUTTON_LONG) {
      nameFlashUntil = HAL_GetTick(); // expire the flash; the long goes through
      functions[functionIndex]->update(event);
    } else {
      showString(functions[functionIndex]->name);
    }
    return;
  }

  if (functions[functionIndex]->update(event)) {
    functionIndex = (functionIndex + 1) % FUNCTION_COUNT;
    nameFlashUntil = HAL_GetTick() + NAME_FLASH_MS;
  }
}
