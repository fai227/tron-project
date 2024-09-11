#include <tk/tkernel.h>

#include "radio.h"

UB packet[32];

ID radio_task_id;
UINT radio_interrupt_number = INTNO(RADIO_BASE);

UB current_id = 0;

void print_packet() {
	for (size_t i = 0; i < 32; i++) {
		tm_printf("%02X ", packet[i]);
	}
	tm_printf("\n");
}

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
                UB possible_departure_time = 0;
                tm_printf("To Do: Check Grid and Get Possible Departure Time\n");

                // パケットに反映
                packet[0] = vehicle_id;
                packet[1] = possible_departure_time;
            }
            // グリッド予約
            else {
                UB vehicle_id = address;
                UH departure_time = packet[0] << 8 | packet[1];
                UB start_position = packet[2];
                UB goal_position = packet[3];

                // パケットに反映
                tm_printf("To Do: Check Grid and Reserve\n");
                // 直接パケットに指示を反映すればよい
            }
        }

        // START送信
        out_w(RADIO(EVENTS_END), 0);
        out_w(RADIO(TASKS_START), 1);
    }

    // 割込有効化
    EnableInt(interrupt_number, 6);
}

void radio_setup() {
    // ハードウェアの初期化
    out_w(RADIO(EVENTS_ADDRESS), 0);
    out_w(0x40000000 + 0x578, 1);

    // 高周波数クロックを有効化
    out_w(CLOCK_EVENTS_HFCLKSTARTED, 0);
    out_w(CLOCK_TASKS_HFCLKSTART, 1);
    while(!in_w(CLOCK_EVENTS_HFCLKSTARTED));
    tm_printf("High Frequency Clock Enabled\n");

    // 出力パワー設定
    out_w(RADIO(TXPOWER), 8); // 最高出力

    // 周波数設定
    out_w(RADIO(FREQUENCY), 40);

    // モード設定（1 Mbps Nordic proprietary radio mode）
    out_w(RADIO(MODE), 1);
    out_w(RADIO(MODECNF0), 1); // 立ち上げの高速化

    // パケットの設定 (PCNF0)
    out_w(RADIO(PCNF0),
         (8 << 0) |   // LENGTHフィールドの長さ (0ビット)
         (0 << 8) |   // S0フィールドの長さ (0バイト)
         (0 << 16));  // S1フィールドの長さ (0バイト)

    // パケットの設定 (PCNF1)
    out_w(RADIO(PCNF1),
         (32 << 0) |  // 最大ペイロード長 (32バイト)
         (32 << 8) |  // 静的ペイロード長 (0バイト)
         (4 << 16) |  // ベースアドレス長 (4バイト)
         (1 << 24) |  // リトルエンディアン
         (1 << 25));  // データホワイトニングの有効化

    // CRC（誤りチェック）設定
    out_w(RADIO(CRCCNF), 1); // CRCを有効に設定
    out_w(RADIO(CRCINIT), 0xFFFF);
    out_w(RADIO(CRCPOLY), 0x11021);

    // データホワイトニング（XORで01をばらけさせる）
    out_w(RADIO(DATAWHITEIV), 0x18);

    // パケットのポインター
    out_w(RADIO(PACKETPTR), (UW)packet);

    // ショートカット
    out_w(RADIO(SHORTS), 0);

    // アドレス設定
   out_w(RADIO(BASE0), 0xBA5E0);
   out_w(RADIO(BASE1), 0xBA5E1);
   out_w(RADIO(PREFIX0),
    0x5C << 0   | // Server to Car, Car to Server for Getting ID
    0xC1 << 8   | // From Vehicle 1 
    0xC2 << 16  | // From Vehicle 2
    0xC3 << 24    // From Vehicle 3
   );
   out_w(RADIO(PREFIX1), 
    0xC4 << 0   | // From Vehicle 4
    0xC5 << 8   | // From Vehicle 5
    0xC6 << 16  | // From Vehicle 6
    0xC7 << 24    // From Vehicle 7
   );  
}

void start_server() {
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

void test_send() {
    // 通信初期化
    radio_setup();

    // 受信アドレスの設定
    out_w(RADIO(RXADDRESSES), 0b00000001);  // 0を有効化

    // 送信アドレスの設定
    out_w(RADIO(TXADDRESS), 3);  // 論理アドレス0を送信アドレスに設定

    for(int i = 0; i < 32; i++) {
        packet[i] = i;
    }

    // 無効化設定
    out_w(RADIO(EVENTS_DISABLED), 0);
    out_w(RADIO(TASKS_DISABLE), 1);
    while(!in_w(RADIO(EVENTS_DISABLED)));

    // 送信準備待ち
    out_w(RADIO(EVENTS_READY), 0);
    out_w(RADIO(TASKS_TXEN), 1);
    while(!in_w(RADIO(EVENTS_READY)));
    tm_printf("Transmit Ready\n");

    // 送信完了待ち
    out_w(RADIO(EVENTS_END), 0);
    out_w(RADIO(TASKS_START), 1);
    while(!in_w(RADIO(EVENTS_END))) {
        if(in_w(RADIO(EVENTS_ADDRESS))) {
            tm_printf("Events Address\n");
            out_w(RADIO(EVENTS_ADDRESS), 0); // イベントクリア
        }

        if(in_w(RADIO(EVENTS_PAYLOAD))) {
            tm_printf("Events Payload\n");
            out_w(RADIO(EVENTS_PAYLOAD), 0); // イベントクリア
        }

        if(in_w(RADIO(EVENTS_PHYEND))) {
            tm_printf("Events Phyend\n");
            out_w(RADIO(EVENTS_PHYEND), 0); // イベントクリア
        }
    }
    tm_printf("Transmit Ended\n");

    // 無効化設定
    out_w(RADIO(EVENTS_DISABLED), 0);
    out_w(RADIO(TASKS_DISABLE), 1);
    while(!in_w(RADIO(EVENTS_DISABLED)));

    print_packet();
}