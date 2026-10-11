#ifndef __MOTOR_H__
#define __MOTOR_H__

void motor_init();
void motor_set_target_position(float position);
float motor_get_position();
void motor_update();
void motor_wait_target();
void motor_step();

#endif
