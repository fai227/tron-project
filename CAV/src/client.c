#include <tk/tkernel.h>
#include <tm/tmonitor.h>

#include "client.h"
#include "order.h"
#include "list.h"
#include "radio.h"
#include "STG.h"

#define CLIENT_DEBUG 0

#define RETRY_TIME 10

IMPORT UB packet[32];

List* order_list_global;

ID transfer_task_id;
LOCAL UINT receive_interrupt_number = INTNO(RADIO_BASE);

EXPORT UINT request_departure_time_ms() {
    // ラジオの初期化
    radio_setup();

    // 受信アドレスの設定
    out_w(RADIO(RXADDRESSES), 0b1);  // 0を有効化

    // 送信アドレスの設定
    out_w(RADIO(TXADDRESS), 0);  // 論理アドレス0を送信アドレスに設定

    // 受信が完了するまで繰り返し
    while(TRUE) {
        // 無効化設定
        out_w(RADIO(EVENTS_DISABLED), 0);
        out_w(RADIO(TASKS_DISABLE), 1);
        while(!in_w(RADIO(EVENTS_DISABLED)));
        out_w(RADIO(EVENTS_DISABLED), 0);

        // 送信準備待機
        out_w(RADIO(EVENTS_READY), 0);
        out_w(RADIO(TASKS_TXEN), 1);
        while(!in_w(RADIO(EVENTS_READY)));
        out_w(RADIO(EVENTS_READY), 0);

        // 送信
        out_w(RADIO(EVENTS_END), 0);
        out_w(RADIO(TASKS_START), 1);
        while(!in_w(RADIO(EVENTS_END)));
        out_w(RADIO(EVENTS_END), 0);

        // 無効化設定
        out_w(RADIO(EVENTS_DISABLED), 0);
        out_w(RADIO(TASKS_DISABLE), 1);
        while(!in_w(RADIO(EVENTS_DISABLED)));
        out_w(RADIO(EVENTS_DISABLED), 0);

        // 受信準備待機
        out_w(RADIO(EVENTS_READY), 0);
        out_w(RADIO(TASKS_RXEN), 1);
        while(!in_w(RADIO(EVENTS_READY)));
        out_w(RADIO(EVENTS_READY), 0);

        // 受信
        out_w(RADIO(EVENTS_END), 0);
        out_w(RADIO(TASKS_START), 1);
        // 0.1秒待機
        tk_slp_tsk(RETRY_TIME);
        if(in_w(RADIO(EVENTS_END))) {
            out_w(RADIO(EVENTS_END), 0);
            break;
        }
    }   

    // 論理アドレスをパケットの1番目に設定
    out_b(RADIO(TXADDRESS), packet[0]);

#if CLIENT_DEBUG
    // ID表示
    tm_printf("ID: %d\n", packet[0]);
#endif

    // パケットの2番目をmsに変換して返す      
    return packet[1] * 1000;
}

LOCAL void receive_interrupt_handler(UINT interrupt_number) {
    // EVENTS_ENDの立ち上がりチェック
    // tm_printf("INTERRUPTED\n");
    if(!in_w(RADIO(EVENTS_END))) {
        return;
    }
    // tm_printf("EVENTS_END\n");
    
    // RECEIVEかチェック
    if(in_w(RADIO(STATE)) != 2) { // RXIDLE
        // tm_printf("Current State: %d\n", in_w(RADIO(STATE)));
        return;
    }
    // tm_printf("RXIDLE\n");

    out_w(RADIO(EVENTS_END), 0);
    out_w(RADIO(TASKS_DISABLE), 1);

#if CLIENT_DEBUG
    tm_printf("Received\n");
#endif

    // 送信タスク終了
    tk_ter_tsk(transfer_task_id);

    // 割込無効化
    DisableInt(interrupt_number);

    // パケット読取
    for(UB i = 0; i < 32; i++) {
        // 指示がなくなったら終了
        if(packet[i] == 0) {
            break;
        }

        // リストの最後の指示と同じ場合は，その指示の継続時間を更新
        Order* last_order = (Order*)list_get_last(order_list_global);
        if(list_length(order_list_global) > 0 && *last_order == packet[i]) {
            *last_order = packet[i] + get_order_duration(packet[i]);

#if CLIENT_DEBUG
            tm_printf("Same Order\n");
#endif
            continue;
        }

        // 次の指示を追加
        UB* order = (UB*)Kmalloc(sizeof(UB));
        *order = packet[i];
        list_append(order_list_global, order);
    }    

#if CLIENT_DEBUG
    tm_printf("Order Appeded\n");
    tm_printf("Current Order Length: %d\n", list_length(order_list_global));

    print_packet();
#endif
}

LOCAL void transfer_task(INT stacd, void *exinf) {
    // 割込ハンドラー定義
    T_DINT receive_interrupt_information;
    receive_interrupt_information.intatr = TA_HLNG;
    receive_interrupt_information.inthdr = receive_interrupt_handler;
    tk_def_int(receive_interrupt_number, &receive_interrupt_information);
    
    // 受信割込設定
    out_w(RADIO(INTENSET), 
        (1 << 3)   // END
    );
    
    // 割込でタスクが終了するまで繰り返し
#if CLIENT_DEBUG
    tm_printf("Waiting for Interrupt\n");
#endif

    while(TRUE) {
        // 無効化
        out_w(RADIO(EVENTS_DISABLED), 0);
        out_w(RADIO(TASKS_DISABLE), 1);
#if CLIENT_DEBUG
        tm_printf("Disabled\n");
#endif
        while(!in_w(RADIO(EVENTS_DISABLED)));
        out_w(RADIO(EVENTS_DISABLED), 0);

        // 送信準備待機
        out_w(RADIO(EVENTS_READY), 0);
        out_w(RADIO(TASKS_TXEN), 1);
#if CLIENT_DEBUG
        tm_printf("TX Ready\n");
#endif
        while(!in_w(RADIO(EVENTS_READY)));
        out_w(RADIO(EVENTS_READY), 0);

        // 送信
        out_w(RADIO(EVENTS_END), 0);
        out_w(RADIO(TASKS_START), 1);
#if CLIENT_DEBUG
        tm_printf("TX Start\n");
#endif
        while(!in_w(RADIO(EVENTS_END)));
        out_w(RADIO(EVENTS_END), 0);

        // 無効化
        out_w(RADIO(EVENTS_DISABLED), 0);
        out_w(RADIO(TASKS_DISABLE), 1);
#if CLIENT_DEBUG
        tm_printf("Disabled\n");
#endif
        while(!in_w(RADIO(EVENTS_DISABLED)));
        out_w(RADIO(EVENTS_DISABLED), 0);

        // 受信準備待機
        out_w(RADIO(EVENTS_READY), 0);
        out_w(RADIO(TASKS_RXEN), 1);
#if CLIENT_DEBUG
        tm_printf("RX Ready\n");
#endif
        while(!in_w(RADIO(EVENTS_READY)));
        out_w(RADIO(EVENTS_READY), 0);

        // 受信
        out_w(RADIO(EVENTS_END), 0);
        out_w(RADIO(TASKS_START), 1);
        EnableInt(receive_interrupt_number, 1);
#if CLIENT_DEBUG
        tm_printf("RX Start\n");
#endif
        // 0.1秒待機
        tk_slp_tsk(RETRY_TIME);
        DisableInt(receive_interrupt_number);
#if CLIENT_DEBUG
        tm_printf("Could not receive, Trying again...\n");
#endif
    }   
}

Position previous_target = POS(0, 0);
Position current_target = POS(0, 0);
EXPORT void reserve_order(List* order_list, UB delay_until_departure) {
    order_list_global = order_list;

    // 移動先を決定
    if(current_target == POS(4, 0)) {
        previous_target = current_target;
        current_target = POS(0, 4);
    }
    else if(current_target == POS(0, 4)) {
        previous_target = current_target;
        current_target = POS(4, 4);
    }
    else {
        previous_target = current_target;
        current_target = POS(4, 0);
    }

    // パケットに反映
    packet[0] = delay_until_departure;
    packet[1] = previous_target;
    packet[2] = current_target;

    // 送信タスク起動
    T_CTSK transfer_task_information;
    transfer_task_information.exinf = (void*)('t' | 'x' << 8 | 'T' << 16);  // txT
    transfer_task_information.tskatr = TA_HLNG;
    transfer_task_information.task = transfer_task;
    transfer_task_information.itskpri = 2;  // 起動優先度
    transfer_task_information.stksz = 1024;  // スタックサイズ
    
    transfer_task_id = tk_cre_tsk(&transfer_task_information);
    tk_sta_tsk(transfer_task_id, 0);
}

