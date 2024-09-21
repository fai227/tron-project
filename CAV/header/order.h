#ifndef ORDER_H_
#define ORDER_H_

#include <tk/typedef.h>

typedef UB Order;

#define MOVE_FORWARD (1 << 3)
#define MOVE_BACKWARD (1 << 2)
#define TURN_LEFT (1 << 1)
#define TURN_RIGHT (1 << 0)

#define ORDER_BIT_SHIFT 4
#define ORDER_MASK 0x0F

/*
 *  orderが前方かを判定
 */
BOOL is_forward(Order order);

/*
 *  orderが後方かを判定
 */
BOOL is_backward(Order order);

/*
 *  orderが左折かを判定
 */
BOOL is_left(Order order);

/*
 *  orderが右折かを判定
 */
BOOL is_right(Order order);

/*
 *  orderの持続時間を返す
 */
UB get_order_duration(Order order);

#endif /* ORDER_H_ */
