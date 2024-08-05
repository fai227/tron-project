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
 *	ser_cnf.h 
 *	Serial communication device driver
 *		Device configuration file
 */
#ifndef	__DEV_SER_CNF_H__
#define	__DEV_SER_CNF_H__

#define	DEVCNF_SER_DEVNAME	"ser"		// Device name ("ser")

#define	DEVCONF_SER_BUFFSIZE	50		//  Communication data buffer size

/* Default value for attribute data */
#define	DEVCNF_SER_SPEED	115200		// Communication speed (baud rate)
#define DEVCNF_SER_MODE		(DEV_SER_MODE_CTSEN | DEV_SER_MODE_RTSEN | \
				DEV_SER_MODE_8BIT | DEV_SER_MODE_1STOP | DEV_SER_MODE_PNON)
						// Mode: Hard flow control enable,
						// 	data 8bit, stop 1bit, no parity
#define	DEVCNF_SER_SND_TMO	TMO_FEVR	// Send timeout 
#define DEVCNF_SER_RCV_TMO	TMO_FEVR	// Receive timeout 

/*----------------------------------------------------------------------
 * Hardware dependent definition
 */
#ifdef CPU_TMPM367FDFG
#include "sysdepend/tx03_m367/ser_cnf_m367.h"
#endif		/* CPU_TMPM367FDFG */
#ifdef CPU_RX231
#include "sysdepend/rx231/ser_cnf_rx231.h"
#endif		/* CPU_RX231 */
#ifdef CPU_STM32L4
#include "sysdepend/stm32l4/ser_cnf_stm32l4.h"
#endif		/* CPU_STM32L4 */
#ifdef CPU_RZA2M
#include "sysdepend/rza2m/ser_cnf_rza2m.h"
#endif		/* CPU_RZA2M */
#ifdef CPU_NRF5
#include "sysdepend/nrf5/ser_cnf_nrf5.h"
#endif		/* CPU_NRF5 */


#endif		/* __DEV_SER_CNF_H__ */
