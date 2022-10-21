#include "pid.h"

float pid_update(pid_t* pid, float input) {
  pid->e = input - pid->last_out;
  
  pid->sum += pid->e;
  
  float out = pid->kp * pid->e + pid->ki * pid->sum;
  if (out > pid->max) {
    out = pid->max;
  } else if(out < pid->min) {
    out = pid->min;
  }
  
  pid->last_out = out;
  return input;
}