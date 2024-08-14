#ifndef ORDER_H_
#define ORDER_H_

#include <tk/typedef.h>

#define ORDER UB

#define MOVE_FORWARD 3
#define MOVE_BACKWARD 2
#define TURN_LEFT 1
#define TURN_RIGHT 0

#define ORDER_BIT_SHIFT 4
#define ORDER_MASK 0x0F

/*
 *  orderが前方かを判定
 */
BOOL is_forward(ORDER order);

/*
 *  orderが後方かを判定
 */
BOOL is_backward(ORDER order);

/*
 *  orderが左折かを判定
 */
BOOL is_left(ORDER order);

/*
 *  orderが右折かを判定
 */
BOOL is_right(ORDER order);

/*
 *  orderの持続時間を返す
 */
UB get_order_duration(ORDER order);

#endif /* ORDER_H_ */
