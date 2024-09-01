#include <tk/tkernel.h>
#include <tm/tmonitor.h>
//#include <trace/trace.h>

#include "radio.h"

/* オブジェクト ID 番号 */
LOCAL ID tidA;           /* タスク A の ID */
LOCAL ID cycA;
LOCAL const UINT intnoA = INTNO(RADIO_BASE); /*  割込番号 */

// パケットデータ
volatile UB packet[32];
volatile UB data = 12;

// パケットの内容を表示する関数
void print_packet() {
	for (size_t i = 0; i < 32; i++) {
		tm_printf("%02X ", packet[i]);
	}
	tm_printf("\n");
}

//パケットの初期化
void init_packet(){
	for (size_t i = 0; i < 32; i++) {
		packet[i] = 0;
	}
}

/* 割込みハンドラ A */
LOCAL void inthdrA(UINT intno)
{

	tm_printf("Receive Ended\n");
	tm_printf("RXRU:%d", in_w(RADIO(TASKS_RXEN)));

	tm_printf("RAPE: %d %d %d %d\n",
			in_w(RADIO(EVENTS_READY)),
			in_w(RADIO(EVENTS_ADDRESS)),
			in_w(RADIO(EVENTS_PAYLOAD)),
			in_w(RADIO(EVENTS_END))
	);

	out_w(RADIO(EVENTS_END), 0);

	// 無効化設定
	out_w(RADIO(EVENTS_DISABLED), 0);
	out_w(RADIO(TASKS_DISABLE), 1);
	while(!in_w(RADIO(EVENTS_DISABLED)));
	tm_printf("Events Disabled\n");

	print_packet();

	out_w(RADIO(EVENTS_DISABLED), 0);
	out_w(RADIO(TASKS_DISABLE), 1);
	while(!in_w(RADIO(EVENTS_DISABLED)));
	tm_printf("Events Disabled\n");

	// パケットのポインター設定 (受信開始前)
	out_w(RADIO(PACKETPTR), (UW)packet);

	// 準備待ち
	out_w(RADIO(EVENTS_READY), 0);
	out_w(RADIO(TASKS_RXEN), 1);
	while(!in_w(RADIO(EVENTS_READY)));
	tm_printf("Receive Ready\n");

	out_w(RADIO(EVENTS_END), 0);
	out_w(RADIO(TASKS_START), 1);

	init_packet();
	//print_packet();

	tk_sta_cyc(cycA);               /* 周期ハンドラ A を動作開始 */
	//tk_wup_tsk(tidA); /* タスク A を起床する */
}

/* 周期ハンドラ A */
LOCAL void cychdrA(void *exinf)
{
	tk_wup_tsk(tidA); /* タスク A を起床する */

}

/* タスク A */
LOCAL void taskA(INT stacd, void *exinf)
{
	for (;;) {
		tm_printf("Waiting\n");
		/* 割込みハンドラから起床されるまで待つ */
		tk_slp_tsk(TMO_FEVR);
		DisableInt(intnoA);

		tm_printf("Good Morning\n");

		packet[0] = data;
		data += 1;

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
			//init_packet();
		}

		print_packet();
		tm_printf("Transmit Ended\n");
		out_w(RADIO(EVENTS_END), 0);
		init_packet();

		// 無効化設定
		out_w(RADIO(EVENTS_DISABLED), 0);
		out_w(RADIO(TASKS_DISABLE), 1);
		while(!in_w(RADIO(EVENTS_DISABLED)));

		// 準備待ち
		out_w(RADIO(EVENTS_READY), 0);
		out_w(RADIO(TASKS_RXEN), 1);
		while(!in_w(RADIO(EVENTS_READY)));
		tm_printf("Receive Ready\n");

		out_w(RADIO(EVENTS_END), 0);
		out_w(RADIO(TASKS_START), 1);

		/* GPIOTE 割込みを許可する */
		ClearInt(intnoA);
		EnableInt(intnoA, 6);

	}

	/* このタスクは終了しない */
}

/* ハードウェア初期化ルーチン */
LOCAL void init(void)
{
	out_w(RADIO(EVENTS_END), 0);
	// INTENSET
	out_w(RADIO(INTENSET), (1 << 3));

	/* GPIOTE 割込みを許可する */
	EnableInt(intnoA, 6);
}

void radio_common_setup() {
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
}

void server_radio_setup() {
    // 共通部分の設定
    radio_common_setup();

    // 論理アドレス0 (クライアント1)
   out_w(RADIO(BASE0), 0xDEADBEEF);
   out_w(RADIO(PREFIX0), 0xAA);

//   // 論理アドレス1 (クライアント2)
//   out_w(RADIO(BASE1), 0xCAFEBABE);
//   out_w(RADIO(PREFIX0), 0xAA);

   // 論理アドレス2 (クライアント3)
   out_w(RADIO(BASE1), 0xCAFEBABE);
   out_w(RADIO(PREFIX1), 0xCC);

    // 受信アドレスの設定
    out_w(RADIO(RXADDRESSES),63 );  // 全ての論理アドレスを有効にする

    out_w(RADIO(TXADDRESS), 4);
}

void client1_radio_setup() {
    // 共通部分の設定
    radio_common_setup();

    // 論理アドレス0 (クライアント1)
    out_w(RADIO(BASE1), 0xCAFEBABE);
    out_w(RADIO(PREFIX0), 0xACABAA);
//    out_w(RADIO(BASE0), 0xDEADBEEF);
//      out_w(RADIO(PREFIX0), 0xAA);
    out_w(RADIO(TXADDRESS), 1); 	// 論理アドレス0に設定

    out_w(RADIO(RXADDRESSES),1 << 1);
}

void client2_radio_setup() {
    // 共通部分の設定
    radio_common_setup();

    // 論理アドレス1 (クライアント2)
    out_w(RADIO(BASE1), 0xCAFEBABE);
    out_w(RADIO(PREFIX0), 0xACABAA);
    out_w(RADIO(TXADDRESS), 2); 	// 論理アドレス4に設定

    out_w(RADIO(RXADDRESSES),1 << 2);
}

void client3_radio_setup() {
    // 共通部分の設定
    radio_common_setup();

    // 論理アドレス2 (クライアント3)
    out_w(RADIO(BASE1), 0xCAFEBABE);
    out_w(RADIO(PREFIX0), 0xACABAA);
//       out_w(RADIO(BASE1), 0xCAFEBABE);
//       out_w(RADIO(PREFIX1), 0xCC);
    out_w(RADIO(TXADDRESS), 3); 	// 論理アドレス5に設定

    out_w(RADIO(RXADDRESSES),1 << 3);
}

// 送信関数
void send() {
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
}

// 受信関数
void receive() {
    // 無効化設定
    out_w(RADIO(EVENTS_DISABLED), 0);
    out_w(RADIO(TASKS_DISABLE), 1);
    while(!in_w(RADIO(EVENTS_DISABLED)));
    tm_printf("Events Disabled\n");

    // パケットのポインター設定 (受信開始前)
    out_w(RADIO(PACKETPTR), (UW)packet);

    // 準備待ち
    out_w(RADIO(EVENTS_READY), 0);
    out_w(RADIO(TASKS_RXEN), 1);
    while(!in_w(RADIO(EVENTS_READY)));
    tm_printf("Receive Ready\n");

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

        if(in_w(RADIO(EVENTS_CTEPRESENT))) {
            tm_printf("Events CTEPresent\n");
            out_w(RADIO(EVENTS_CTEPRESENT), 0); // イベントクリア
        }

        if(in_w(RADIO(EVENTS_PHYEND))) {
            tm_printf("Events Phyend\n");
            out_w(RADIO(EVENTS_PHYEND), 0); // イベントクリア
        }
    }
    tm_printf("Receive Ended\n");

    // 無効化設定
    out_w(RADIO(EVENTS_DISABLED), 0);
    out_w(RADIO(TASKS_DISABLE), 1);
    while(!in_w(RADIO(EVENTS_DISABLED)));
    tm_printf("Events Disabled\n");
}

EXPORT INT usermain(void)
{
	/* Aボタン (P0.14), Bボタン (P0.23) から入力するように設定する */
    out_w(GPIO(P0, PIN_CNF(14)), 0);
    out_w(GPIO(P0, PIN_CNF(23)), 0);

	int sender=0;

	// Bが押されるまでカウントアップ
	tm_printf("A to count, B to start\n");
	while(TRUE) {
		UW x = in_w(GPIO(P0, IN));
		UW A = (x & (1 << 14)) == 0; /* Aボタン (P0.14) */
		UW B = (x & (1 << 23)) == 0; /* Bボタン (P0.23) */

		if (A) {
			sender++;
			tm_printf("Sender: %d\n", sender);
			tk_dly_tsk(1000);
		} else if (B) {
			tm_printf("Start\n");
			break;
		}
	}

	if(sender == 1){
		client1_radio_setup();
		for(int i=0; i<32; i++){
			packet[i] = 1;
		}
		send();
	}else if(sender == 2){
		client2_radio_setup();
		for(int i=0; i<32; i++){
			packet[i] = 2;
		}
		send();
	}else if(sender == 3){
		client3_radio_setup();
		for(int i=0; i<32; i++){
			packet[i] = 3;
		}
		send();
	}else if(sender == 4){
		client1_radio_setup();
		receive();
	}else if(sender == 5){
		client2_radio_setup();
		receive();
	}else if(sender == 6){
		client3_radio_setup();
		receive();
	}
	tm_printf("Packet: ");
	print_packet();
}