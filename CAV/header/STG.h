#ifndef STG_H_
#define STG_H_

#include "order.h"
#include <tk/tkernel.h>

typedef UB Position;

#define POS(x, y) ((x << 4) | y)
#define POS_X(pos) (pos >> 4)
#define POS_Y(pos) (pos & 0x0F)

#define GRID_SIZE 5
#define STG_BUFFER_LENGTH 1000

#define STG_VERBOSE 1

#define GRID_EMPTY 0

#define UP_PATH 0b0001
#define DOWN_PATH 0b0010
#define LEFT_PATH 0b0100
#define RIGHT_PATH 0b1000

#define GRID_MOVE_TIME 2
#define GRID_WAIT_TIME 1
#define GRID_TURN_TIME 3

/*
 * 時空間グリッドの初期化と立ち上げ
 */
EXPORT void stg_start(UB timer_number);

/*
 * 時空間グリッドの予約処理
 */
EXPORT void stg_reserve(Order *orders, UB max_order_size, UB vehicle_id, UB delay_until_departure, Position start_position, Position target_position);

/*
 * (0, 0)に侵入可能時間を計算
 */
EXPORT UW stg_get_delay_until_departure(UB vehicle_id);

/*
 * 時空間グリッドの読み取り
 */
LOCAL UB stg_get_grid(UW time, Position position);

/*
 * 時空間グリッドへの書き込み
 */
LOCAL void stg_set_grid(UW time, Position position, UB value);

#endif /* STG_H_ */