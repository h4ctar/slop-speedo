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
static uint8_t function_index = 0;
static uint32_t name_flash_until_ms = 0;
#define NAME_FLASH_MS 1000

void app_setup() {
  HAL_TIM_Base_Start(&htim1);
  button_init();
  motor_init();
  settings_init();
  for (uint8_t i = 0; i < FUNCTION_COUNT; i++) {
    functions[i]->init();
  }
  screen_init();
}

void app_loop() {
  motor_update();
  speed_task();

  ButtonEvent event = button_poll();

  // While settling on a newly selected function, show its name. Cycling is
  // never blocked: a short press advances immediately, and a long press cuts
  // the flash short and is handed to the function.
  if ((int32_t)(HAL_GetTick() - name_flash_until_ms) < 0) {
    if (event == BUTTON_SHORT) {
      function_index = (function_index + 1) % FUNCTION_COUNT;
      name_flash_until_ms = HAL_GetTick() + NAME_FLASH_MS;
    } else if (event == BUTTON_LONG) {
      name_flash_until_ms = HAL_GetTick(); // expire the flash; the long goes through
      functions[function_index]->update(event);
    } else {
      screen_show_string(functions[function_index]->name);
    }
    return;
  }

  if (functions[function_index]->update(event)) {
    function_index = (function_index + 1) % FUNCTION_COUNT;
    name_flash_until_ms = HAL_GetTick() + NAME_FLASH_MS;
  }
}
