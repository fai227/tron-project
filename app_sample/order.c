#include "order.h"
#include <tk/typedef.h>

#include "order.h"

/*
orderは全部で8bit
上位4ビットで方向指示、下位4bitで継続時間
例)order = 0x82 の場合：バイナリで 1000 0010.
ここで方向の1000を取り出そうとすると、右に4bitシフトすればいい。
(order >> 4) を行うと、0000 1000 になり1000 = 0x8 つまり MOVE_FORWARD.
*/


/*
 *  orderが前方かを判定
 *  上位4ビットがMOVE_FORWARDと一致するかを確認
 */
EXPORT BOOL is_forward(Order order) {
    // orderの上位4ビットをチェック
    return ((order >> ORDER_BIT_SHIFT) == MOVE_FORWARD);
}

/*
 *  orderが後方かを判定
 *  上位4ビットがMOVE_BACKWARDと一致するかを確認
 */
EXPORT BOOL is_backward(Order order) {
    return ((order >> ORDER_BIT_SHIFT) == MOVE_BACKWARD);
}

/*
 *  orderが左折かを判定
 *  上位4ビットがTURN_LEFTと一致するかを確認
 */
EXPORT BOOL is_left(Order order) {
    return ((order >> ORDER_BIT_SHIFT) == TURN_LEFT);
}

/*
 *  orderが右折かを判定
 *  上位4ビットがTURN_RIGHTと一致するかを確認
 */
EXPORT BOOL is_right(Order order) {
    return ((order >> ORDER_BIT_SHIFT) == TURN_RIGHT);
}

/*
 *  orderの持続時間を返す
 *  下位4ビットから持続時間（秒）を取得
 */
/*
0x0F (バイナリ表記: 0000 1111) は、下位4ビットがすべて1上位4ビットがすべて0になっている
先ほどと同じようにもしorderが0x82のとき
order & 0x0F は 1000 0010 と 0000 1111 のビットごとのAND演算
結果は 0000 0010 となり、0x02 に相当
つまり2秒
*/

EXPORT UB get_order_duration(Order order) {
    return (order & ORDER_MASK);  // 下位4ビットを抽出して持続時間
}
