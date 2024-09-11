#include <tk/tkernel.h>

#include "client.h"
#include "order.h"
#include "list.h"

UINT request_departure_time_ms() {
    // サーバーと通信して，結果を受け取るまで待つ
    tk_dly_tsk(1000);

    // 仮の値を返す 
    return 1000;
}

void reserve_order(List *order_list, UB delay_until_departure) {
    // サーバーと通信する，この関数は結果を受け取るまで待たない
    // 通信が完了すると，order_listに命令を追加する
    // 目的地はここで決定する

    // 一旦仮の値を返す
    /*
        前1秒
        前1秒
        前1秒
        前1秒
        左2秒
        前1秒
        前1秒
        前1秒
        前1秒
        左2秒: 次の旅行に向けてあらかじめ左に転回
    */

    list_append(order_list, MOVE_FORWARD << ORDER_BIT_SHIFT | 1);
    list_append(order_list, MOVE_FORWARD << ORDER_BIT_SHIFT | 1);
    list_append(order_list, MOVE_FORWARD << ORDER_BIT_SHIFT | 1);
    list_append(order_list, MOVE_FORWARD << ORDER_BIT_SHIFT | 1);
    list_append(order_list, TURN_LEFT    << ORDER_BIT_SHIFT | 2);
    list_append(order_list, MOVE_FORWARD << ORDER_BIT_SHIFT | 1);
    list_append(order_list, MOVE_FORWARD << ORDER_BIT_SHIFT | 1);
    list_append(order_list, MOVE_FORWARD << ORDER_BIT_SHIFT | 1);
    list_append(order_list, MOVE_FORWARD << ORDER_BIT_SHIFT | 1);
    list_append(order_list, TURN_LEFT    << ORDER_BIT_SHIFT | 2);    
}