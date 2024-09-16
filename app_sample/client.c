#include <tk/tkernel.h>

#include "client.h"
#include "order.h"
#include "list.h"
#include "radio.h"

IMPORT UB packet[32];

List* order_list_global;

ID transfer_task_id;
UINT receive_interrupt_number = INTNO(RADIO_BASE);

UINT request_departure_time_ms() {
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
        // 1秒待機
        tk_slp_tsk(1000);
        if(in_w(RADIO(EVENTS_END))) {
            out_w(RADIO(EVENTS_END), 0);
            break;
        }
    }   

    // 論理アドレスをパケットの1番目に設定
    out_b(RADIO(TXADDRESS), packet[0]);

    // ID表示
    tm_printf("ID: %d\n", packet[0]);

    // パケットの2番目をmsに変換して返す      
    return packet[1] * 1000;
}

void receive_interrupt_handler(UINT interrupt_number) {
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
    tm_printf("Received\n");

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

        // 次の支持を追加
        UB* order = (UB*)Kmalloc(sizeof(UB));
        *order = packet[i];
        list_append(order_list_global, order);
    }

    tm_printf("Order Appeded\n");
    tm_printf("Current Order Length: %d\n", list_length(order_list_global));
}

void transfer_task(INT stacd, void *exinf) {
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
    tm_printf("Waiting for Interrupt\n");
    while(TRUE) {
        // 無効化
        out_w(RADIO(EVENTS_DISABLED), 0);
        out_w(RADIO(TASKS_DISABLE), 1);
        tm_printf("Disabled\n");
        while(!in_w(RADIO(EVENTS_DISABLED)));
        out_w(RADIO(EVENTS_DISABLED), 0);

        // 送信準備待機
        out_w(RADIO(EVENTS_READY), 0);
        out_w(RADIO(TASKS_TXEN), 1);
        tm_printf("TX Ready\n");
        while(!in_w(RADIO(EVENTS_READY)));
        out_w(RADIO(EVENTS_READY), 0);

        // 送信
        out_w(RADIO(EVENTS_END), 0);
        out_w(RADIO(TASKS_START), 1);
        tm_printf("TX Start\n");
        while(!in_w(RADIO(EVENTS_END)));
        out_w(RADIO(EVENTS_END), 0);

        // 無効化
        out_w(RADIO(EVENTS_DISABLED), 0);
        out_w(RADIO(TASKS_DISABLE), 1);
        tm_printf("Disabled\n");
        while(!in_w(RADIO(EVENTS_DISABLED)));
        out_w(RADIO(EVENTS_DISABLED), 0);

        // 受信準備待機
        out_w(RADIO(EVENTS_READY), 0);
        out_w(RADIO(TASKS_RXEN), 1);
        tm_printf("RX Ready\n");
        while(!in_w(RADIO(EVENTS_READY)));
        out_w(RADIO(EVENTS_READY), 0);

        // 受信
        out_w(RADIO(EVENTS_END), 0);
        out_w(RADIO(TASKS_START), 1);
        EnableInt(receive_interrupt_number, 1);
        tm_printf("RX Start\n");
        tk_slp_tsk(1000);
        DisableInt(receive_interrupt_number);
        tm_printf("Could not receive, Trying again...\n");
    }   
}

void reserve_order(List* order_list, UB delay_until_departure) {
    order_list_global = order_list;

    // 移動先を決定

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

