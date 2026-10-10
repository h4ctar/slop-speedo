#include "motor.h"
#include "main.h"
#include "tim.h"

#define MOT_PATTERN_4 0b00000111
#define MOT_PATTERN_3 0b00011100
#define MOT_PATTERN_2 0b00011100
#define MOT_PATTERN_1 0b00110001

#define STEP_PERIOD 1200

#define MAX_POSITION       16
#define STEPS_PER_POSITION 60.
#define MAX_STEPS          (STEPS_PER_POSITION * MAX_POSITION)

uint16_t lastStepTime = 10000;
uint16_t currentSteps = 0;
uint16_t targetSteps = 0;

void initMotor() {
  // Home the motor
  setMotorTargetPosition(MAX_POSITION);
  waitForMotor();
  setMotorTargetPosition(0);
  waitForMotor();
}

void setMotorTargetPosition(float position) {
  // Clamp the float before converting: a negative float to uint16_t is
  // undefined behaviour, and the old unsigned checks could never catch it.
  // !(position >= 0) also catches NaN, which fails every comparison.
  if (!(position >= 0)) {
    position = 0;
  }
  if (position > MAX_POSITION) {
    position = MAX_POSITION;
  }
  targetSteps = (uint16_t)(position * STEPS_PER_POSITION);
  if (targetSteps > MAX_STEPS) {
    targetSteps = MAX_STEPS;
  }
}

float getMotorCurrentPosition() {
  return currentSteps / STEPS_PER_POSITION;
}

void updateMotor() {
  // 16-bit math on purpose: TIM1's counter wraps every 65.5 ms, and unsigned
  // subtraction mod 2^16 gives the true elapsed ticks across any wrap.
  // Comparing in 32 bits made each wrap fire a step early.
  uint16_t currentTime = (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
  if ((uint16_t)(currentTime - lastStepTime) >= STEP_PERIOD) {
    int16_t delta = targetSteps - currentSteps;
    int8_t direction = (delta > 0) - (delta < 0);
    currentSteps += direction;
    stepMotor();
    lastStepTime = currentTime;
  }
}

void waitForMotor() {
  while (currentSteps != targetSteps) {
    updateMotor();
  }
}

void stepMotor() {
  uint8_t step = currentSteps % 6;
  HAL_GPIO_WritePin(M1_GPIO_Port, M1_Pin, (MOT_PATTERN_1 >> step) & 1);
  HAL_GPIO_WritePin(M2_GPIO_Port, M2_Pin, (MOT_PATTERN_2 >> step) & 1);
  HAL_GPIO_WritePin(M3_GPIO_Port, M3_Pin, (MOT_PATTERN_3 >> step) & 1);
  HAL_GPIO_WritePin(M4_GPIO_Port, M4_Pin, (MOT_PATTERN_4 >> step) & 1);
}
