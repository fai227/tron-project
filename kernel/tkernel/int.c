/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2006-2019 by Ken Sakamura.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
 *
 *----------------------------------------------------------------------
 */

/*
 *	int.c
 *	Interrupt Control
 */

#include "kernel.h"
#include "check.h"

/* ------------------------------------------------------------------------ */
/*
 * Interrupt handler definition
 */
SYSCALL ER tk_def_int( UINT intno, CONST T_DINT *pk_dint )
#if USE_DBGSPT
;
IMPORT FP knl_hook_enterfn;
IMPORT FP knl_hook_leavefn;
LOCAL ER l_tk_def_int( UINT intno, CONST T_DINT *pk_dint );
ER tk_def_int( UINT intno, CONST T_DINT *pk_dint )
{
	CONST W fncd = 0x80370200;
	void *exinf;
	if (knl_hook_enterfn != NULL) {
		UW calinf;
		asm("mov %0, lr" : "=r" (calinf));
		exinf = ((void*(*)())knl_hook_enterfn)( fncd, calinf, intno, pk_dint );
	}
	ER er = l_tk_def_int( intno, pk_dint );
	if (knl_hook_leavefn != NULL) {
		knl_hook_leavefn( fncd, er, exinf );
	}
	return er;
}
LOCAL ER l_tk_def_int( UINT intno, CONST T_DINT *pk_dint )
#endif /* USE_DBGSPT */
{
#if USE_STATIC_IVT
	return E_NOSPT;
#else
	ATR	intatr;
	FP	inthdr;
	ER	ercd;

	CHECK_PAR(intno < N_INTVEC);
	if(pk_dint != NULL) {
		CHECK_RSATR(pk_dint->intatr, TA_HLNG|TA_ASM);
		intatr	= pk_dint->intatr;
		inthdr	= pk_dint->inthdr;
	} else {
		intatr	= 0;
		inthdr	= NULL;
	}

	BEGIN_CRITICAL_SECTION;
	ercd = knl_define_inthdr(intno, intatr, inthdr);
	END_CRITICAL_SECTION;

	return ercd;
#endif
}

/* ------------------------------------------------------------------------ */
/*
 * return Interrupt handler
 */
SYSCALL void tk_ret_int( void )
#if USE_DBGSPT
;
IMPORT FP knl_hook_enterfn;
IMPORT FP knl_hook_leavefn;
LOCAL void l_tk_ret_int( void );
void tk_ret_int( void )
{
	CONST W fncd = 0x80380000;
	void *exinf;
	if (knl_hook_enterfn != NULL) {
		UW calinf;
		asm("mov %0, lr" : "=r" (calinf));
		exinf = ((void*(*)())knl_hook_enterfn)( fncd, calinf );
	}
	l_tk_ret_int(  );
}
LOCAL void l_tk_ret_int( void )
#endif /* USE_DBGSPT */
{
	knl_return_inthdr();
	return;
}

