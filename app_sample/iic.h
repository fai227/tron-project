#ifndef IIC_H_
#define IIC_H_

#include <tk/typedef.h>

// I2C操作に関する定義
#define IIC_RESTART   (3 << 14)    // リスタートコンディションを送信
#define IIC_START     (2 << 14)    // スタートコンディションを送信
#define IIC_STOP      (1 << 14)    // ストップコンディションを送信
#define IIC_SEND      (1 << 13)    // データを送信
#define IIC_RECV      (1 << 12)    // データを受信
#define IIC_TOPDATA   (1 << 11)    // 送受信データの先頭
#define IIC_LASTDATA  (1 << 10)    // 送受信データの末端

// 書き込み/読み込み操作のマクロ
#define WR(adr)       ((adr) | 0x01)  // 書き込み操作
#define RD(adr)       ((adr) & ~0x01) // 読み取り操作

// 関数プロトタイプ
ER iicxfer(W ch, UH *cmd_dat, W words, W *xwords);
ER iicsetup(BOOL start);
ER iic_write(W ch, INT adr, INT reg, UB dat);
INT iic_read(W ch, INT adr, INT reg);
ER write_reg(W ch, INT adr, INT reg, UB dat);
INT read_reg(W ch, INT adr, INT reg);

#endif /* IIC_H_ */
