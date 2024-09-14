#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <sys/sysdepend/cpu/nrf5/sysdef.h>
#include "maqueen.h"
#include "list.h"
#include "client.h"
#include "order.h"

#define LIST_MIN 4 //指示四つ

volatile BOOL request_sent_flag = FALSE;
volatile UH last_request_list_count = 0;


void start_drive(){
    List* order_list=list_init();//経路を保存するリストの作成
    intialize_timer();//タイマの初期化
    maqueen_init();//maqueenの初期化
    INT departure_ms=request_departure_time_ms();//待機時間受け取り
  
    INT departure_second=departure_ms/1000;
    reserve_order(order_list,departure_second);//listをグローバル変数にするとともに、送信タスクを起動
    tk_slp_tsk(departure_ms);//侵入可能時間まで待機
    
    
    while(TRUE){
        if(list_length(order_list)<LIST_MIN){
            INT departure_delay= calculate_departure_delay(order_list);
            process_orders(order_list);
        }
        void *data=list_get(order_list,0);
        Order* order = (Order*)data;
        if(data !=NULL){
            follow_path(*order);
        }
        delete_first_element(order_list);

    
            
        }
        
    }


INT calculate_departure_delay(List *order_list) { //リストにある経路の所要時間を計算
    UB delay_until_departure = 0;
    Element *pointer = order_list->head;
    
    while (pointer != order_list->tail && pointer != NULL) {
        delay_until_departure += get_order_duration((Order*)pointer->data);
        pointer = pointer->next;
    }
    
    return delay_until_departure;
}

void process_orders(List *order_list) { //リクエスト後反映されるまでにリクエストを行わないようにする
    UH current_list_count = list_length(order_list);

    if (!request_sent_flag && current_list_count <= 4) {
        // リクエストを送信
        INT departure_delay = calculate_departure_delay(order_list);
        reserve_order(order_list, departure_delay);
        
        // フラグを設定し、リスト個数を保存
        request_sent_flag = TRUE;
        last_request_list_count = current_list_count;
    } else if (request_sent_flag) {
        // リスト個数が増加したかチェック
        if (current_list_count > last_request_list_count) {
            // リスト個数が増加したのでフラグをリセット
            request_sent_flag = FALSE;
        }
    }
}
// リストの先頭要素を削除する関数
void delete_first_element(List *list)
{
    if (list == NULL || list->head == NULL)
    {
        // リストが空または無効な場合、何もしない
        return;
    }

    Element *first = list->head;
    list->head = first->next;

    // リストの長さを減少
    list->length--;

    // リストが空になった場合、tail も NULL に設定
    if (list->head == NULL)
    {
        list->tail = NULL;
    }

    // 先頭要素のメモリを解放
    Kfree(first);

}
