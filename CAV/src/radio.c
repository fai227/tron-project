#include <tk/tkernel.h>

#include "radio.h"

EXPORT UB packet[32];

void print_packet() {
	for (size_t i = 0; i < 32; i++) {
		tm_printf("%02X ", packet[i]);
	}
	tm_printf("\n");
}

void radio_setup() {
    // ハードウェアの初期化
    out_w(RADIO(EVENTS_ADDRESS), 0);
    out_w(0x40000000 + 0x578, 1);

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
