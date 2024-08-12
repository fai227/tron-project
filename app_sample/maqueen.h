#ifndef MAQUEEN_H_
#define MAQUEEN_H_

#include "iic.h"

// MaqueenのI2Cアドレスおよびレジスタアドレス
#define MAQUEEN_ADDRESS 0x10
#define LEFT_DIRECTION  0x00
#define LEFT_SPEED      0x01
#define RIGHT_DIRECTION 0x02
#define RIGHT_SPEED     0x03
#define SENSOR          0x1E//30

// ライントレーサの定数
#define L2  0
#define L1  1
#define M   2
#define R1  3
#define R2  4

// 進行方向の定義
#define FORWARD     0
#define BACKWARD    1

//LEDライトの定義
#define MAQUEEN_LED_LEFT 0x0B
#define MAQUEEN_LED_RIGHT 0x0C

//左モータと右モータの定義
#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1
#define BOTH_MOTOR 2

//チャンネル設定
#define EXTERNAL_I2C_CHANNEL 1

// センサー読み取りの遅延時間（ミリ秒）
#define SENSOR_DELAY 100

// 関数プロトタイプ
ER maqueen_init(void);
ER control_motor(B motor, B direction, B speed);
ER stop_all_motor(void);
ER control_led(B led, B state);
ER stop_all_led(void);
B read_line_state(B line);
UB read_line_value(B line);


#endif /* MAQUEEN_H_ */
