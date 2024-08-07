#ifndef APP_SAMPLE_MAQUEEN_H_
#define APP_SAMPLE_MAQUEEN_H_

#include <tk/typedef.h>
//include "tk/tkernel.h"

// I2C Address
#define MAQUEEN 0x10

// Register definitions
#define LEFT_DIRECTION 0x00
#define LEFT_SPEED 0x01
#define RIGHT_DIRECTION 0x02
#define RIGHT_SPEED 0x03
#define SENSOR 0x1D

// Line sensor values
#define LINE_L2 0
#define LINE_L1 1
#define LINE_M 2
#define LINE_R1 3
#define LINE_R2 4

// Motor definitions
#define MOTOR_L 0
#define MOTOR_R 1

// LED definitions
#define LIGHT_L 0
#define LIGHT_R 1

// Line sensor states
#define L2 16
#define L1 8
#define M 4
#define R1 2
#define R2 1

#define L2R2 17
#define L2MR2 21
#define L2MR1R2 23
#define L2L1MR2 25
#define L2L1MR1R2 31
#define L1M 12
#define MR1 6
#define L1MR1 13
#define MR2 5

#define MR1R2 7
#define L1MR2 13
#define L1MR1R2 15
#define L2M 20
#define L2MR1 22
#define L2L1M 28
#define L2L1MR1 30

// Direction definitions
#define FORWARD 0
#define BACKWARD 1

// Sensor delay
#define SENSOR_DELAY 100 // センサー読み取りの遅延時間（ミリ秒）

// Ultrasonic sensor pins
#define ULTRASONIC_TRIG_PIN 7  // トリガーピン（例）
#define ULTRASONIC_ECHO_PIN 8  // エコーピン（例）

// Function prototypes
ER maqueen_init(void);
ER control_motor(B motor, B direction, B speed);
ER control_motor_stop_all(void);
ER control_led(B led, B state);
B read_line_sensor_state(B line);
UB read_line_sensor_value(B line);
UW read_ultrasonic(void);

#endif /* APP_SAMPLE_MAQUEEN_H_ */
