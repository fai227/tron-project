/*
 *  I2C (IIC) 入出力インタフェース定義
 *  Copyright (C) 2024 by Personal Media Corporation
 */

#ifndef __IIC_H__
#define __IIC_H__

#include <tk/typedef.h>

#ifdef __cplusplus
extern "C" {
#endif

//以下元もとiic.cにあったもの
#define	VERBOSE		0	/* デバッグ用メッセージ 1:あり 0:なし */
// #define VERBOSE	1	/* デバッグ用メッセージ 1:あり 0:なし */

#define	I2C_CH		0	/* I2C チャンネル (micro:bitのI2C_INT用) */

#define	WR(da)		(((da) << 1) | 0)
#define	RD(da)		(((da) << 1) | 1)


#define	IICMAX		2					/* 対応チャンネル数 */


#define	IRQ(cb)		INTNO((cb)->iob)	/* I2C 割込番号 */
#define	IRQ_LEVEL	3					/* I2C 割込優先レベル */


/*
 * nRF5 TWI (I2C) レジスタ
 *	レジスタ幅はすべて 32 ビット
 */
#define	IIC(cb, reg)	( (cb)->iob + (reg) )

#define	TASKS_STARTRX		0x000	/* Start TWI receive sequence */
#define	TASKS_STARTTX		0x008	/* Start TWI transmit sequence */
#define	TASKS_STOP			0x014	/* Stop TWI transaction */
#define	TASKS_SUSPEND		0x01C	/* Suspend TWI transaction */
#define	TASKS_RESUME		0x020	/* Resume TWI transaction */
#define	EVENTS_STOPPED		0x104	/* TWI stopped */
#define	EVENTS_RXDREADY		0x108	/* TWI RXD byte received */
#define	EVENTS_TXDSENT		0x11C	/* TWI TXD byte sent */
#define	EVENTS_ERROR		0x124	/* TWI error */
#define	EVENTS_BB			0x138	/* TWI byte boundary */
#define	EVENTS_SUSPENDED	0x148	/* TWI entered the suspended state */
#define	SHORTS				0x200	/* Shortcuts */
#define	INTENSET			0x304	/* Enable interrupt */
#define	INTENCLR			0x308	/* Disable interrupt */
#define	ERRORSRC			0x4C4	/* Error source */
#define	ENABLE				0x500	/* Enable TWI */
#define	PSEL_SCL			0x508	/* Pin select for SCL */
#define	PSEL_SDA			0x50C	/* Pin select for SDA */
#define	RXD					0x518	/* RXD register */
#define	TXD					0x51C	/* TXD register */
#define	FREQUENCY			0x524	/* TWI frequency */
#define	ADDRESS				0x588	/* Address used in the TWI transfer */

//ここまでがもともとiic.cにあったもの


/* IIC 操作マクロ */
#define IIC_RESTART    (3 << 14)  /* リスタートコンディションを送信 */
#define IIC_START      (2 << 14)  /* スタートコンディションを送信 */
#define IIC_STOP       (1 << 14)  /* ストップコンディションを送信 */
#define IIC_SEND       (1 << 13)  /* データを送信 */
#define IIC_RECV       (1 << 12)  /* データを受信 */
#define IIC_TOPDATA    (1 << 11)  /* 送受信データの先頭 */
#define IIC_LASTDATA   (1 << 10)  /* 送受信データの末端 */

/*nrf5_iic.cからの追加関数プロトタイプ*/
IMPORT ER iicxfer(W ch, UH *cmddat, W words, W *xwords);
IMPORT ER iicsetup(BOOL start);

/* iic_reg.cからの追加関数プロトタイプ */
IMPORT ER write_reg(W ch,INT adr, INT reg, UB dat);	/*引数にチャンネル追加*/
IMPORT INT read_reg(W ch,INT adr, INT reg);			/*引数にチャンネル追加*/


#ifdef __cplusplus
}
#endif

#endif /* __IIC_H__ */
