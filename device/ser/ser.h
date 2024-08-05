﻿/*
 *----------------------------------------------------------------------
 *    Device Driver for micro T-Kernel for μT-Kernel 3.00.05
 *
 *    Copyright (C) 2020-2021 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2021/11.
 *
 *----------------------------------------------------------------------
 */


/*
 *	dev_ser.h
 *	Serial communication device driver
 *	Driver local definition
 */

#ifndef	__DEV_SER_H__
#define	__DEV_SER_H__

#include "../include/dev_ser.h"
#include "../common/drvif/msdrvif.h"
#include "ser_cnf.h"

/*----------------------------------------------------------------------
 * Hardware dependent definition
 */
#ifdef CPU_TMPM367FDFG
#include "sysdepend/tx03_m367/ser_m367.h"
#endif		/* CPU_TMPM367FDFG */
#ifdef CPU_RX231
#include "sysdepend/rx231/ser_rx231.h"
#endif	/* CPU_RX231 */
#ifdef CPU_STM32L4
#include "sysdepend/stm32l4/ser_stm32l4.h"
#endif	/* CPU_STM32L4 */
#ifdef CPU_RZA2M
#include "sysdepend/rza2m/ser_rza2m.h"
#endif	/* CPU_RZA2M */
#ifdef CPU_NRF5
#include "sysdepend/nrf5/ser_nrf5.h"
#endif	/* CPU_NRF5 */


/*----------------------------------------------------------------------
 * Communication data buffer
 */
typedef struct {
	UW	top;
	UW	tail;
	ID	wait_tskid;
	UB	data[DEVCONF_SER_BUFFSIZE];
} T_SER_BUFF;

Inline	void buff_init(T_SER_BUFF *buff)
{
	buff->top = buff->tail = buff->wait_tskid = 0;
}

/*----------------------------------------------------------------------
 * Device driver Control block
 */
typedef struct {
	UW	unit;		/* Unit No. */
	UINT	omode;		/* Open mode */

	/* Attribute data */
	ID	evtmbfid;	/* MBF ID for event notification */
	UW	com_mode;	/* Communication mode */
	UW	com_speed;	/* Communication Speed */
	UW	com_error;	/* Communication Error */
	TMO	snd_tmo;	/* Send timeout */
	TMO	rcv_tmo;	/* Receive timeout */

	/* Intrrupt number */
	UINT	intno_rcv;
	UINT	intno_snd;

	/* Communication data buffer */
	T_SER_BUFF	snd_buff;
	T_SER_BUFF	rcv_buff;

} T_SER_DCB;

/*----------------------------------------------------------------------
 * Serial device driver low-level interface
 */

/* The following functions are defined in the system common part. */
IMPORT BOOL dev_ser_get_snddat(UW unit, UW *data);	/* Get send data */
IMPORT void dev_ser_notify_rcv(UW unit, UW data);	/* Notification of receipt */
IMPORT void dev_ser_notify_err(UW unit, UW err);	/* Communication error notification */

/* The following functions are defined in the system-dependent part. */

/* Low-level device control */
IMPORT ER dev_ser_llctl(UW, INT, UW);		/* Low-Level device control */
IMPORT ER dev_ser_llinit(T_SER_DCB*);		/* Low-level device initialization */

/* Low-level device control command */
typedef enum {
	LLD_SER_MODE,		/* Set Communication mode */
	LLD_SER_SPEED,		/* Set Communication Speed */
	LLD_SER_START,		/* Start communication */
	LLD_SER_STOP,		/* Stop Start communication */
	LLD_SER_SEND,		/* Send data */
	LLD_SER_BREAK,		/* Send Break */
	LLD_SER_INTCTL,		/* Interrupt control */
} T_LLD_SER_CMD;

#endif		/* __DEV_SER_H__ */
