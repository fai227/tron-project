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

#define	IIC_TASKS_STARTRX		0x000	/* Start TWI receive sequence */
#define	IIC_TASKS_STARTTX		0x008	/* Start TWI transmit sequence */
#define	IIC_TASKS_STOP			0x014	/* Stop TWI transaction */
#define	IIC_TASKS_SUSPEND		0x01C	/* Suspend TWI transaction */
#define	IIC_TASKS_RESUME		0x020	/* Resume TWI transaction */
#define	IIC_EVENTS_STOPPED		0x104	/* TWI stopped */
#define	IIC_EVENTS_RXDREADY		0x108	/* TWI RXD byte received */
#define	IIC_EVENTS_TXDSENT		0x11C	/* TWI TXD byte sent */
#define	IIC_EVENTS_ERROR		0x124	/* TWI error */
#define	IIC_EVENTS_BB			0x138	/* TWI byte boundary */
#define	IIC_EVENTS_SUSPENDED	0x148	/* TWI entered the suspended state */
#define	IIC_SHORTS				0x200	/* Shortcuts */
#define	IIC_INTENSET			0x304	/* Enable interrupt */
#define	IIC_INTENCLR			0x308	/* Disable interrupt */
#define	IIC_ERRORSRC			0x4C4	/* Error source */
#define	IIC_ENABLE				0x500	/* Enable TWI */
#define	IIC_PSEL_SCL			0x508	/* Pin select for SCL */
#define	IIC_PSEL_SDA			0x50C	/* Pin select for SDA */
#define	IIC_RXD					0x518	/* RXD register */
#define	IIC_TXD					0x51C	/* TXD register */
#define	IIC_FREQUENCY			0x524	/* TWI frequency */
#define	IIC_ADDRESS				0x588	/* Address used in the TWI transfer */

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
IMPORT ER iic_transfer(W ch, UH *cmddat, W words, W *xwords);
IMPORT ER iic_setup(BOOL start);

/* iic_reg.cからの追加関数プロトタイプ */
IMPORT ER iic_write(W ch,INT adr, INT reg, UB dat);	/*引数にチャンネル追加*/
IMPORT INT iic_read(W ch,INT adr, INT reg);			/*引数にチャンネル追加*/


#ifdef __cplusplus
}
#endif

#endif /* __IIC_H__ */
