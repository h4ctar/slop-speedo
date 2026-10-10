#ifndef __MOTOR_H__
#define __MOTOR_H__

void initMotor();
void setMotorTargetPosition(float position);
float getMotorCurrentPosition();
void waitForMotor();
void stepMotor();

#endif
