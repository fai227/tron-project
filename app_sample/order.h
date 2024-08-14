#ifndef ORDER_H_
#define ORDER_H_

#include <tk/typedef.h>

#define MOVE_FORWARD 3
#define MOVE_BACKWARD 2
#define TURN_LEFT 1
#define TURN_RIGHT 0

#define ORDER_BIT_SHIFT 4
#define ORDER_MASK 0x0F

/*
 *  orderが前方かを判定
 */
BOOL is_forward(UB order);

/*
 *  orderが後方かを判定
 */
BOOL is_backward(UB order);

/*
 *  orderが左折かを判定
 */
BOOL is_left(UB order);

/*
 *  orderが右折かを判定
 */
BOOL is_right(UB order);

/*
 *  orderの持続時間を返す
 */
UB get_order_duration(UB order);

#endif /* ORDER_H_ */
