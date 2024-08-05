﻿/*
 *----------------------------------------------------------------------
 *    Device Driver for micro T-Kernel for μT-Kernel 3.00.03
 *
 *    Copyright (C) 2020-2021 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2021/03/31.
 *
 *----------------------------------------------------------------------
 */


/*
 *	i2c_cnf.h 
 *	I2C device driver
 *		Device configuration file
 */
#ifndef	__DEV_I2C_CNF_H__
#define	__DEV_I2C_CNF_H__


#define	DEVCNF_I2C_DEVNAME	"iic"		// Device name ("iic")

#define	DEVCNF_I2C_MAX_SDATSZ	100		// Maximum transmission data size
#define	DEVCNF_I2C_MAX_RDATSZ	100		// Maximum received data size


/* Hardware dependent definition */
#ifdef CPU_TMPM367FDFG
#include "sysdepend/tx03_m367/i2c_cnf_m367.h"
#endif		/* CPU_TMPM367FDFG */

#ifdef CPU_STM32L4
#include "sysdepend/stm32l4/i2c_cnf_stm32l4.h"
#endif		/* CPU_STM32L4 */

#ifdef CPU_RX231
#include "sysdepend/rx231/i2c_cnf_rx231.h"
#endif		/* CPU_RX231 */

#endif		/* __DEVCNF_I2C_H__ */
