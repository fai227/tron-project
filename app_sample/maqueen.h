#ifndef APP_SAMPLE_MAQUEEN_H_
#define APP_SAMPLE_MAQUEEN_H_

#include <tk/typedef.h>

#ifndef E_OK
#define E_OK 0  // 必要に応じて定義
#endif

// I2C Address
#define MAQUEEN 0x10
#define I2C_CH 1  // 適切なチャンネル番号に置き換えてください

// Register definitions
#define LEFT_DIRECTION 0x02
#define LEFT_SPEED 0x03
#define RIGHT_DIRECTION 0x00
#define RIGHT_SPEED 0x01
#define SENSOR 0x1D
#define RGB_L 0x0B
#define RGB_R 0x0C

// LED definitions
#define LED_L 0
#define LED_R 1

// Motor and direction definitions
#define MOTOR_L 0  // 左モーター
#define MOTOR_R 1  // 右モーター
#define FORWARD 0  // 前進
#define BACKWARD 1  // 後退

// Ultrasonic sensor pins
#define ULTRASONIC_TRIG_PIN 7  // トリガーピン
#define ULTRASONIC_ECHO_PIN 8  // エコーピン

// Function prototypes
void I2C_init(void);
ER maqueen_init(void);
ER control_motor(B motor, B direction, B speed);
ER control_motor_stop_all(void);
ER control_led(B led, B state);
B read_line_sensor_state(B line);
UB read_line_sensor_value(B line);
UW read_ultrasonic(void);

// Write and read register functions
ER write_reg(W ch, INT adr, INT reg, UB dat);
UB read_reg(W ch, INT adr, INT reg);

#endif /* APP_SAMPLE_MAQUEEN_H_ */
