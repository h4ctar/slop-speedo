#include "motor.h"
#include "main.h"
#include "tim.h"

#define MOT_PATTERN_4 0b00000111
#define MOT_PATTERN_3 0b00011100
#define MOT_PATTERN_2 0b00011100
#define MOT_PATTERN_1 0b00110001

#define STEP_PERIOD_TICKS 1200

#define MAX_POSITION       16
#define STEPS_PER_POSITION 60.
#define MAX_STEPS          (STEPS_PER_POSITION * MAX_POSITION)

uint16_t last_step_time_ticks = 10000;
uint16_t current_steps = 0;
uint16_t target_steps = 0;

void motor_init() {
  // Home the motor
  motor_set_target_position(MAX_POSITION);
  motor_wait_target();
  motor_set_target_position(0);
  motor_wait_target();
}

void motor_set_target_position(float position) {
  // Clamp the float before converting: a negative float to uint16_t is
  // undefined behaviour, and the old unsigned checks could never catch it.
  // !(position >= 0) also catches NaN, which fails every comparison.
  if (!(position >= 0)) {
    position = 0;
  }
  if (position > MAX_POSITION) {
    position = MAX_POSITION;
  }
  target_steps = (uint16_t)(position * STEPS_PER_POSITION);
  if (target_steps > MAX_STEPS) {
    target_steps = MAX_STEPS;
  }
}

float motor_get_position() {
  return current_steps / STEPS_PER_POSITION;
}

void motor_update() {
  // 16-bit math on purpose: TIM1's counter wraps every 65.5 ms, and unsigned
  // subtraction mod 2^16 gives the true elapsed ticks across any wrap.
  // Comparing in 32 bits made each wrap fire a step early.
  //
  // loop() can be busy for tens of ms (the OLED redraw blocks on I2C), so
  // catch up on every step that is due, not just one. Gaps over 65.5 ms
  // under-measure and merely defer steps, never fire early.
  for (;;) {
    uint16_t now_ticks = (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
    if ((uint16_t)(now_ticks - last_step_time_ticks) < STEP_PERIOD_TICKS) {
      return;
    }
    int16_t delta = target_steps - current_steps;
    if (delta == 0) {
      last_step_time_ticks = now_ticks; // idle: anchor the schedule for the next command
      return;
    }
    int8_t direction = (delta > 0) - (delta < 0);
    current_steps += direction;
    motor_step();
    last_step_time_ticks += STEP_PERIOD_TICKS;
  }
}

void motor_wait_target() {
  while (current_steps != target_steps) {
    motor_update();
  }
}

void motor_step() {
  uint8_t step = current_steps % 6;
  HAL_GPIO_WritePin(M1_GPIO_Port, M1_Pin, (MOT_PATTERN_1 >> step) & 1);
  HAL_GPIO_WritePin(M2_GPIO_Port, M2_Pin, (MOT_PATTERN_2 >> step) & 1);
  HAL_GPIO_WritePin(M3_GPIO_Port, M3_Pin, (MOT_PATTERN_3 >> step) & 1);
  HAL_GPIO_WritePin(M4_GPIO_Port, M4_Pin, (MOT_PATTERN_4 >> step) & 1);
}
