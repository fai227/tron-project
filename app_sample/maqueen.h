#ifndef APP_SAMPLE_MAQUEEN_H_
#define APP_SAMPLE_MAQUEEN_H_

#include "tk/tkernel.h"

#define LINE_L2 0
#define LINE_L1 1
#define LINE_M 2
#define LINE_R1 3
#define LINE_R2 4

#define MOTOR_L 0
#define MOTOR_R 1

#define LIGHT_L 0
#define LIGHT_R 1

// その他必要なdefineはここ

ER maqueen_init();
ER control_motor(B motor, B direction, B speed);
ER control_motor_stop_all();
ER control_led(B led, B state);
B read_line_sensor_state(B line);
UB read_line_sensor_value(B line);
UW read_ultrasonic();

#endif /* APP_SAMPLE_MAQUEEN_H_ */
