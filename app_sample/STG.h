#ifndef STG_H_
#define STG_H_

#include "order.h"

typedef UB Position;

#define POS(x, y) ((x << 4) | y)
#define POS_X(pos) (pos >> 4)
#define POS_Y(pos) (pos & 0x0F)

#define GRID_SIZE 5
#define STG_BUFFER_LENGTH 100

//LOCAL UB calculate_distance(Position start, Position target)
//LOCAL UW calculate_f(Node* node)
//LOCAl UB get_moved_position(UB x, UB y, UB direction)
//LOCAL BOOL has_turned(UB next_x, UB next_y, Node* current_node)
//LOCAL BOOL check_grid(UW from, UW to, Position position, UB vehicle_id)
//EXPORT void reserve_grid(UW from, UW to, Position position, UB vehicle_id) 
//LOCAL List* get_valid_moves(Node* current_node, Position target_position, UB vehicle_id)
//LOCAL Node* find_path(Position start_position, Position target_position, UW departure_time, UB vehicle_id)
//LOCAL void stg_handler() 


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