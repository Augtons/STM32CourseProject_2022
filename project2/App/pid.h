#ifndef _PID_H_
#define _PID_H_

typedef struct pid_t {
  float kp;
  float ki;
  
  float last_out;
  float e;
  
  float sum;
  
  float max;
  float min;
} pid_t;


float pid_update(pid_t* pid, float input);

#endif