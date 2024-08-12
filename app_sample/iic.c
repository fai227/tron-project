/*
 *	@(#)nrf5_iic.c 2024-04-18
 *
 *	nRF5 I2C ドライバー
 *	Copyright (C) 2024 by Personal Media Corporation
 *
 *	I2C を IIC と表記している。
 */


#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <tstdlib.h>
#include "iic.h"



//以下nrf5_iic.cより
LOCAL FastMLock	IICLock;	/* 排他制御用ロック */
LOCAL ID	IICFlgID;		/* 割込通知用イベントフラグ */

/*
 * IIC 制御情報
 */
typedef struct iiccb {
	UW	iob;		/* IIC レジスターアドレス */
	UH	*cmddat;	/* コマンド現在位置 */
	UH	*end;		/* コマンド終了位置 */
} IICCB;
LOCAL IICCB	iiccb[IICMAX];

/*
 * nRF5 TWI (I2C) 構成情報
 *	ピン割り当て
 *	P0.08	I2C_INT_SCL
 *	P0.16	I2C_INT_SDA
 *	P0.26	I2C_EXT_SCL
 *	P1.00	I2C_EXT_SDA
 */
LOCAL struct iic_conf {
	UW	iob;		/* I/O ベースアドレス */
	UW	psel_scl;	/* SCL ピン設定 */
	UW	psel_sda;	/* SDA ピン設定 */
	UW	freq;		/* クロック設定 */
} const iic_conf[IICMAX] = {
	{ 0x40003000, 0x08, 0x10, 0x06680000 },	/* TWI0 I2C_INT 400KHz */
	{ 0x40004000, 0x1a, 0x20, 0x06680000 }	/* TWI1 I2C_EXT 400KHz */
};

/*
 * 送受信開始
 *	*cb->cmddat が IIC_START または IIC_RESTART であること。
 *	戻値 0:正常 -1:異常
 */
LOCAL INT transfer_start( IICCB *cb )
{
	UW	d;

	if ( cb->end - cb->cmddat < 3 ) return -1;

	d = *cb->cmddat++ & 0xff;
	out_w(IIC(cb, IIC_ADDRESS), d >> 1);

	if ( (d & 1) == 0 ) {
		/* 送信開始 */
		out_w(IIC(cb, IIC_SHORTS), 0);
		out_w(IIC(cb, IIC_TASKS_STARTTX), 1);
		out_w(IIC(cb, IIC_TXD), *cb->cmddat & 0xff);
	} else {
		/* 受信開始 */
		if ( (*(cb->cmddat + 1) & 0xc000) == IIC_STOP ) {
			out_w(IIC(cb, IIC_SHORTS), 2); /* BB_STOP */
		} else {
			out_w(IIC(cb, IIC_SHORTS), 1); /* BB_SUSPEND */
		}
		out_w(IIC(cb, IIC_TASKS_STARTRX), 1);
	}

	return 0;
}

/*
 * コマンド実行
 *	戻値 0:継続 1:終了 -1:異常
 */
LOCAL INT transfer_cmddat( IICCB *cb )
{
	UW	cmd, d;

	cmd = *cb->cmddat;

	if ( (in_w(IIC(cb, IIC_EVENTS_ERROR)) & 1) != 0 ) {
		out_w(IIC(cb, IIC_EVENTS_ERROR), 0);
		goto err_stop;
	}

	if ( (in_w(IIC(cb, IIC_EVENTS_TXDSENT)) & 1) != 0 ) {
		out_w(IIC(cb, IIC_EVENTS_TXDSENT), 0);
		if ( (cmd & IIC_SEND) != 0 ) {
			/* データ送信完了 */
			if ( ++cb->cmddat >= cb->end ) goto err_stop;
			cmd = *cb->cmddat; /* 次へ */
		}

		if ( (cmd & IIC_SEND) != 0 ) {
			/* 次のデータを送信 */
			out_w(IIC(cb, IIC_TXD), cmd & 0xff);
		} else if ( (cmd & 0xc000) == IIC_RESTART ) {
			if ( transfer_start(cb) < 0 ) goto err_stop;
		} else if ( (cmd & 0xc000) == IIC_STOP ) {
			out_w(IIC(cb, IIC_TASKS_STOP), 1);
		}
	}

	if ( (in_w(IIC(cb, IIC_EVENTS_RXDREADY)) & 1) != 0 ) {
		out_w(IIC(cb, IIC_EVENTS_RXDREADY), 0);
		d = in_w(IIC(cb, IIC_RXD)) & 0xff;
		if ( (cmd & IIC_RECV) != 0 ) {
			/* 受信データを保存 */
			*cb->cmddat |= d;
			if ( ++cb->cmddat >= cb->end ) goto err_stop;
			cmd = *cb->cmddat; /* 次へ */
		}
	}

	if ( (in_w(IIC(cb, IIC_EVENTS_SUSPENDED)) & 1) != 0 ) {
		out_w(IIC(cb, IIC_EVENTS_SUSPENDED), 0);
		if ( (cmd & IIC_LASTDATA) != 0 ) {
			if ( cb->cmddat + 1 >= cb->end ) goto err_stop;
			if ( (*(cb->cmddat + 1) & 0xc000) == IIC_STOP ) {
				out_w(IIC(cb, IIC_SHORTS), 2); /* BB_STOP */
			}
		}
		if ( (cmd & 0xc000) == IIC_RESTART ) {
			if ( transfer_start(cb) < 0 ) goto err_stop;
		} else {
			/* 次のデータ受信開始 */
			out_w(IIC(cb, IIC_TASKS_RESUME), 1);
		}
	}

	if ( (in_w(IIC(cb, IIC_EVENTS_STOPPED)) & 1) != 0 ) {
		out_w(IIC(cb, IIC_EVENTS_STOPPED), 0);
		if ( (cmd & 0xc000) == IIC_STOP ) cb->cmddat++;
		return 1;
	}

	return 0;

  err_stop:
	out_w(IIC(cb, IIC_TASKS_STOP), 1);
	return -1;
}

/*
 * 割込ハンドラ
 */
LOCAL void iic_int_handler( UINT dintno )
{
	IICCB	*cb;
	UW	ch;

	for ( ch = 0; ch < IICMAX; ch++ ) {
		cb = &iiccb[ch];
		if ( DINTNO(IRQ(cb)) == dintno ) break;
	}
	if ( ch >= IICMAX ) return;

	if ( transfer_cmddat(cb) != 0 ) {
		out_w(IIC(cb, IIC_INTENCLR), 0xffffffff);

		/* 終了通知 */
		tk_set_flg(IICFlgID, 1 << ch);
	}
}

/*
 * IIC 送受信処理
 */
EXPORT ER iic_transfer( W ch, UH *cmddata, W words, W *xwords )
{
	IICCB	*cb;
	UINT	ptn;
	UW	n;
	ER	err;

	if ( ch < 0 || ch >= IICMAX ) { err = E_PAR; goto err_ret; }
	if ( words < 3 ) { err = E_PAR; goto err_ret; }

	err = MLock(&IICLock, ch);
	if ( err < E_OK ) goto err_ret;

	cb = &iiccb[ch];

	/* イベント／ステータス・クリア */
	out_w(IIC(cb, IIC_EVENTS_STOPPED),		0);
	out_w(IIC(cb, IIC_EVENTS_RXDREADY),		0);
	out_w(IIC(cb, IIC_EVENTS_TXDSENT),		0);
	out_w(IIC(cb, IIC_EVENTS_ERROR),		0);
	out_w(IIC(cb, IIC_EVENTS_BB),		0);
	out_w(IIC(cb, IIC_EVENTS_SUSPENDED),	0);
	out_w(IIC(cb, IIC_SHORTS),			0);
	out_w(IIC(cb, IIC_ERRORSRC),		0xffffffff);
	out_w(IIC(cb, IIC_INTENCLR),		0xffffffff);

	/* 制御情報設定 */
	cb->cmddat = cmddata;
	cb->end	   = cmddata + words;

	/* IIC 動作開始 */
	n = transfer_start(cb);
	if ( n == 0 ) {
		out_w(IIC(cb, IIC_INTENSET),
		      (1 << 1) |	/* STOPPED */
		      (1 << 2) |	/* RXDREADY */
		      (1 << 7) |	/* TXDSENT */
		      (1 << 9) |	/* ERROR */
		      (1 << 18));	/* SUSPENDED */

		/* 終了待ち */
		err = tk_wai_flg(IICFlgID, 1 << ch, TWF_ANDW|TWF_BITCLR,
				 &ptn, 1000 + words * 10);

		out_w(IIC(cb, IIC_INTENCLR), 0xffffffff);
		if ( err < E_OK ) {
			/* 強制終了 */
			out_w(IIC(cb, IIC_TASKS_STOP), 1);
		} else {
			n = in_w(IIC(cb, IIC_ERRORSRC)) & 7;
			if ( (n & 4) != 0 &&
			     (*cb->cmddat & IIC_SEND) != 0 ) {
				/* スレーブ側から停止した */
				cb->cmddat++;
			}
			if ( n != 0 )	err = E_IO | n;
			else		err = E_OK;
		}
	} else {
		err = E_PAR;
	}

	/* 転送が完了したワード数の記録 */
	if ( xwords != NULL ) *xwords = cb->cmddat - cmddata;

	MUnlock(&IICLock, ch);

	return err;

  err_ret:
	if ( xwords != NULL ) *xwords = 0;
	return err;
}

/*
 * IIC ドライバ起動/終了
 */
EXPORT ER iic_setup( BOOL start )
{
#define	IICTag	"IIC_"

	ER	err;
	T_CFLG	cflg;
	T_DINT	dint;
	W	ch;
	IICCB	*cb;

	if ( !start ) {
		/* 終了処理 */
		err = E_OK;
		goto finish;
	}

	/* 排他制御用ロック生成 */
	err = CreateMLock(&IICLock, IICTag);
	if ( err < E_OK ) goto err_ret1;

	/* 割込通知用イベントフラグの作成 */
	SetOBJNAME(cflg.exinf, IICTag);
	cflg.flgatr  = TA_TFIFO | TA_WMUL;
	cflg.iflgptn = 0;
	err = tk_cre_flg(&cflg);
	if ( err < E_OK ) goto err_ret2;
	IICFlgID = err;

	for ( ch = 0; ch < IICMAX; ++ch ) {
		cb = &iiccb[ch];
		cb->iob = iic_conf[ch].iob;

		/* 割込ハンドラ登録 */
		dint.intatr = TA_HLNG;
		dint.inthdr = iic_int_handler;
		err = tk_def_int(DINTNO(IRQ(cb)), &dint);
		if ( err < E_OK ) goto err_ret3;

		/* TWI 初期設定 */
		out_w(IIC(cb, IIC_ENABLE),    5);
		out_w(IIC(cb, IIC_INTENCLR),  0xffffffff);
		out_w(IIC(cb, IIC_PSEL_SCL),  iic_conf[ch].psel_scl);
		out_w(IIC(cb, IIC_PSEL_SDA),  iic_conf[ch].psel_sda);
		out_w(IIC(cb, IIC_FREQUENCY), iic_conf[ch].freq);

		EnableInt(IRQ(cb), IRQ_LEVEL);
	}

	return E_OK;

  finish:
	/* 終了処理 */
	ch = IICMAX;
  err_ret3:
	while ( ch-- > 0 ) {
		cb = &iiccb[ch];
		MLock(&IICLock, ch);
		DisableInt(IRQ(cb));
		tk_def_int(DINTNO(IRQ(cb)), NULL);
	}
	tk_del_flg(IICFlgID);
  err_ret2:
	DeleteMLock(&IICLock);
  err_ret1:
	return err;
}

/*
 * レジスタ書き込み
 */
EXPORT ER iic_write(W ch, INT adr, INT reg, UB dat ) /*チャンネル追加*/
{
	UH	c[4];
	W	n;
	ER	err;

	c[0] = IIC_START | WR(adr);
	c[1] = IIC_SEND  | IIC_TOPDATA  | reg;
	c[2] = IIC_SEND  | IIC_LASTDATA | dat;
	c[3] = IIC_STOP;

	err = iic_transfer(ch, c, sizeof(c) / sizeof(UH), &n);

#if VERBOSE
	tm_printf("iic_write 0x%02x 0x%02x <- 0x%02x : n=%d err=%d\n",
		  adr, reg, dat, n, err);
#endif

	return err;
}

/*
 * レジスタ読み出し
 */
EXPORT INT iic_read(W ch, INT adr, INT reg ) /*チャンネル追加*/
{
	UH	c[5];
	UB	dat;
	W	n;
	ER	err;

	c[0] = IIC_START   | WR(adr);
	c[1] = IIC_SEND    | IIC_TOPDATA | IIC_LASTDATA | reg;
	c[2] = IIC_RESTART | RD(adr);
	c[3] = IIC_RECV    | IIC_TOPDATA | IIC_LASTDATA;
	c[4] = IIC_STOP;

	err = iic_transfer(ch, c, sizeof(c) / sizeof(UH), &n);

	dat = c[3] & 0xff;

#if VERBOSE
	tm_printf("iic_read 0x%02x 0x%02x -> 0x%02x : n=%d err=%d\n",
		  adr, reg, dat, n, err);
#endif

	return ( err < E_OK )? err: dat;
}


