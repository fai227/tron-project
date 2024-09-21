#include <tk/tkernel.h>

#include "radio.h"
#include "STG.h"

IMPORT UB packet[32];


ID radio_task_id;
UINT radio_interrupt_number = INTNO(RADIO_BASE);

UB current_id = 0;

LOCAL void radio_interrupt_handler(UINT interrupt_number)
{
    // 割込無効化
    ClearInt(interrupt_number);

    // イベント判定
    if(in_w(RADIO(EVENTS_DISABLED))) {
        tm_printf("INTERRUPT DISABLED\n");
        
        // イベントクリア
        out_w(RADIO(EVENTS_DISABLED), 0);

        // RXEN送信
        out_w(RADIO(TASKS_RXEN), 1);        
    }
    else if(in_w(RADIO(EVENTS_END))) {
        tm_printf("INTERRUPT END\n");
        
        // イベントクリア
        out_w(RADIO(EVENTS_END), 0);

        // RX・TX判定
        if(in_w(RADIO(STATE)) == 2) {  // RXIDLE
            // TXEN送信
            out_w(RADIO(TASKS_TXEN), 1);
        }
        else if (in_w(RADIO(STATE)) == 10) {  // TXIDLE
            // DISABLED送信
            out_w(RADIO(TASKS_DISABLE), 1);
        }
    }
    else if(in_w(RADIO(EVENTS_READY))) {
        tm_printf("INTERRUPT READY\n");

        // イベントクリア
        out_w(RADIO(EVENTS_READY), 0);

        // RX・TX判定
        if(in_w(RADIO(STATE)) == 2) {  // RXIDLE
            tm_printf("Start Waiting for Receiving Data\n");
        }
        else if (in_w(RADIO(STATE)) == 10) {  // TXIDLE
            // 直前の通信の論理アドレスを取得
            UB address = in_b(RADIO(RXMATCH));

            // ID取得
            if(address == 0) {
                // 使用する論理アドレス
                UB vehicle_id = ++current_id;

                // 侵入可能時間
                UB possible_departure_time = stg_get_departure_time();

                // パケットに反映
                packet[0] = vehicle_id;
                packet[1] = possible_departure_time;
            }
            // グリッド予約
            else {
                UB vehicle_id = address;
                UH departure_time = packet[0];
                UB start_position = packet[1];
                UB goal_position = packet[2];

                // パケットに反映
                stg_reserve(packet, 32, vehicle_id, departure_time, start_position, goal_position);
                print_packet();
            }
        }

        // START送信
        out_w(RADIO(EVENTS_END), 0);
        out_w(RADIO(TASKS_START), 1);
    }

    // 割込有効化
    EnableInt(interrupt_number, 6);
}


EXPORT void start_server() {
    // 通信初期化
    radio_setup();

    // 受信アドレスの設定
    out_w(RADIO(RXADDRESSES), 0b11111111);  // 0~7までを有効化
    // 送信アドレスの設定
    out_w(RADIO(TXADDRESS), 0);  // 論理アドレス0を送信アドレスに設定

    // 割り込みハンドラー定義
    T_DINT radio_interruption_handler_information;
    radio_interruption_handler_information.intatr = TA_HLNG;
    radio_interruption_handler_information.inthdr = radio_interrupt_handler;
    tk_def_int(radio_interrupt_number, &radio_interruption_handler_information);

    // 割り込み許可
    out_w(RADIO(INTENSET), 
        (1 << 0) | // READY
        (1 << 3) | // END
        (1 << 4)   // DISABLED
    );
    EnableInt(radio_interrupt_number, 6);  // 右の数なんやっけ

    // イベントクリア
    out_w(RADIO(EVENTS_DISABLED), 0);
    out_w(RADIO(EVENTS_READY), 0);
    out_w(RADIO(EVENTS_END), 0);

    // 無効化設定
    out_w(RADIO(EVENTS_DISABLED), 0);
    out_w(RADIO(TASKS_DISABLE), 1);
}