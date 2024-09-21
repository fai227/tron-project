#ifndef STG_H_
#define STG_H_

#include "order.h"
#include <tk/tkernel.h>

typedef UB Position;

#define POS(x, y) ((x << 4) | y)
#define POS_X(pos) (pos >> 4)
#define POS_Y(pos) (pos & 0x0F)

#define GRID_SIZE 5
#define STG_BUFFER_LENGTH 100

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
EXPORT UW stg_get_departure_time();

/*
 * 時空間グリッドの読み取り
 */
LOCAL UB stg_get_grid(UW time, Position position);

/*
 * 時空間グリッドへの書き込み
 */
LOCAL void stg_set_grid(UW time, Position position, UB value);

#endif /* STG_H_ */