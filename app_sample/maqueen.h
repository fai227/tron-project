#ifndef MAQUEEN_H_
#define MAQUEEN_H_

#include "iic.h"

// MaqueenのI2Cアドレスおよびレジスタアドレス
#define MAQUEEN_ADDRESS 0x10
#define LEFT_DIRECTION  0x00
#define LEFT_SPEED      0x01
#define RIGHT_DIRECTION 0x02
#define RIGHT_SPEED     0x03
#define MAQUEEN_LINE_SENSOR  0x1E//30
#define MAQUEEN_LINE_SENSOR_29 0x1D

// ライントレーサの定数
#define MAQUEEN_LINE_SENSOR_L2  0
#define MAQUEEN_LINE_SENSOR_L1  1
#define MAQUEEN_LINE_SENSOR_M   2
#define MAQUEEN_LINE_SENSOR_R1  3
#define MAQUEEN_LINE_SENSOR_R2  4

// 進行方向の定義
#define MAQUEEN_MOVE_FORWARD     0
#define MAQUEEN_MOVE_BACKWARD    1

//LEDライトの定義
#define MAQUEEN_LED_LEFT 0x0B
#define MAQUEEN_LED_RIGHT 0x0C

//左モータと右モータの定義
#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1
#define BOTH_MOTOR 2

//チャンネル設定
#define EXTERNAL_I2C_CHANNEL 1



// 関数プロトタイプ
ER maqueen_init(void);
ER control_motor(B motor, UB direction, UB speed);
ER stop_all_motor(void);
ER control_led(B led, B state);
ER turn_off_all_led(void);
B read_line_state(B line);
UB read_line_value(B ch, B line);


#endif /* MAQUEEN_H_ */
