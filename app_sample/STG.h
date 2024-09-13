#ifndef STG_H_
#define STG_H_

#include "order.h"

typedef UB Position;

#define POS(x, y) x << 4 | y
#define POS_X(pos) pos >> 4
#define POS_Y(pos) pos & 0x0F

#define GRID_SIZE 5
#define STG_BUFFER_LENGTH 100

/*
 * 時空間グリッドの初期化と立ち上げ
 */
void stg_start(void);

/*
 * 時空間グリッドの予約処理
 */
void stg_reserve(Order *orders, UB max_order_size, UB vehicle_id, UB delay_until_departure, Position start_position, Position end_position);

/*
 * 時空間グリッドの読み取り
 */
UB get_grid(UW time, Position position);

/*
 * 時空間グリッドへの書き込み
 */
void set_grid(UW time, Position position, UB value);

#endif /* STG_H_ */