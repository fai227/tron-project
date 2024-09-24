/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.01
 *
 *    Copyright (C) 2006-2020 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2020/05/29.
 *
 *----------------------------------------------------------------------
 */

/*
 *	misc_calls.c
 *	Other System Calls
 */

#include "kernel.h"
#include "check.h"


#ifdef USE_FUNC_TK_REF_SYS
/*
 * Refer system state
 */
SYSCALL ER tk_ref_sys( T_RSYS *pk_rsys )
#if USE_DBGSPT
;
IMPORT FP knl_hook_enterfn;
IMPORT FP knl_hook_leavefn;
LOCAL ER l_tk_ref_sys( T_RSYS *pk_rsys );
ER tk_ref_sys( T_RSYS *pk_rsys )
{
	CONST W fncd = 0x80520100;
	void *exinf;
	if (knl_hook_enterfn != NULL) {
		UW calinf;
		asm("mov %0, lr" : "=r" (calinf));
		exinf = ((void*(*)())knl_hook_enterfn)( fncd, calinf, pk_rsys );
	}
	ER er = l_tk_ref_sys( pk_rsys );
	if (knl_hook_leavefn != NULL) {
		knl_hook_leavefn( fncd, er, exinf );
	}
	return er;
}
LOCAL ER l_tk_ref_sys( T_RSYS *pk_rsys )
#endif /* USE_DBGSPT */
{
	BOOL	b_qtsk;

	if ( in_indp() ) {
		pk_rsys->sysstat = TSS_INDP;
	} else {
		BEGIN_DISABLE_INTERRUPT;
		b_qtsk = in_qtsk();	
		END_DISABLE_INTERRUPT;
		
		if ( b_qtsk ) {
			pk_rsys->sysstat = TSS_QTSK;
		} else {
			pk_rsys->sysstat = TSS_TSK;
		}
		if ( in_loc() ) {
			pk_rsys->sysstat |= TSS_DINT;
		}
		if ( in_ddsp() ) {
			pk_rsys->sysstat |= TSS_DDSP;
		}
	}
	pk_rsys->runtskid = ( knl_ctxtsk != NULL )? knl_ctxtsk->tskid: 0;
	pk_rsys->schedtskid = ( knl_schedtsk != NULL )? knl_schedtsk->tskid: 0;

	return E_OK;
}
#endif /* USE_FUNC_TK_REF_SYS */

#ifdef USE_FUNC_TK_REF_VER
/*
 * Refer version information
 *	If there is no kernel version information,
 *	set 0 in each information. (Do NOT cause errors.)
 */
SYSCALL ER tk_ref_ver( T_RVER *pk_rver )
#if USE_DBGSPT
;
IMPORT FP knl_hook_enterfn;
IMPORT FP knl_hook_leavefn;
LOCAL ER l_tk_ref_ver( T_RVER *pk_rver );
ER tk_ref_ver( T_RVER *pk_rver )
{
	CONST W fncd = 0x80510100;
	void *exinf;
	if (knl_hook_enterfn != NULL) {
		UW calinf;
		asm("mov %0, lr" : "=r" (calinf));
		exinf = ((void*(*)())knl_hook_enterfn)( fncd, calinf, pk_rver );
	}
	ER er = l_tk_ref_ver( pk_rver );
	if (knl_hook_leavefn != NULL) {
		knl_hook_leavefn( fncd, er, exinf );
	}
	return er;
}
LOCAL ER l_tk_ref_ver( T_RVER *pk_rver )
#endif /* USE_DBGSPT */
{
	pk_rver->maker = (UH)VER_MAKER;	/* OS manufacturer */
	pk_rver->prid  = (UH)VER_PRID;	/* OS identification number */
	pk_rver->spver = (UH)VER_SPVER;	/* Specification version */
	pk_rver->prver = (UH)VER_PRVER;	/* OS product version */
	pk_rver->prno[0] = (UH)VER_PRNO1;	/* Product number */
	pk_rver->prno[1] = (UH)VER_PRNO2;	/* Product number */
	pk_rver->prno[2] = (UH)VER_PRNO3;	/* Product number */
	pk_rver->prno[3] = (UH)VER_PRNO4;	/* Product number */

	return E_OK;
}
#endif /* USE_FUNC_TK_REF_VER */

/* ------------------------------------------------------------------------ */
/*
 *	Debugger support function
 */
#if USE_DBGSPT

#ifdef USE_FUNC_TD_REF_SYS
/*
 * Refer system state
 */
SYSCALL ER td_ref_sys( TD_RSYS *pk_rsys )
{
	BOOL	b_qtsk;

	if ( in_indp() ) {
		pk_rsys->sysstat = TSS_INDP;
	} else {
		BEGIN_DISABLE_INTERRUPT;
		b_qtsk = in_qtsk();	
		END_DISABLE_INTERRUPT;
		
		if ( b_qtsk ) {
			pk_rsys->sysstat = TSS_QTSK;
		} else {
			pk_rsys->sysstat = TSS_TSK;
		}
		if ( in_loc() ) {
			pk_rsys->sysstat |= TSS_DINT;
		}
		if ( in_ddsp() ) {
			pk_rsys->sysstat |= TSS_DDSP;
		}
	}
	pk_rsys->runtskid = ( knl_ctxtsk != NULL )? knl_ctxtsk->tskid: 0;
	pk_rsys->schedtskid = ( knl_schedtsk != NULL )? knl_schedtsk->tskid: 0;

	return E_OK;
}
#endif /* USE_FUNC_TD_REF_SYS */

EXPORT FP knl_hook_enterfn;
EXPORT FP knl_hook_leavefn;

EXPORT FP knl_hook_execfn;
EXPORT FP knl_hook_stopfn;

EXPORT FP knl_hook_ienterfn;
EXPORT FP knl_hook_ileavefn;

#ifdef USE_FUNC_TD_HOK_SVC
SYSCALL ER td_hok_svc( CONST TD_HSVC *hsvc )
{
	BEGIN_DISABLE_INTERRUPT;
	if (hsvc != NULL) {
		knl_hook_enterfn = hsvc -> enter;
		knl_hook_leavefn = hsvc -> leave;
	} else {
		knl_hook_enterfn = NULL;
		knl_hook_leavefn = NULL;
	}
	END_DISABLE_INTERRUPT;

	return E_OK;
}
#endif /* USE_FUNC_TD_HOK_SVC */

#ifdef USE_FUNC_TD_HOK_DSP
SYSCALL ER td_hok_dsp( CONST TD_HDSP *hdsp )
{
	BEGIN_DISABLE_INTERRUPT;
	if (hdsp != NULL) {
		knl_hook_execfn = hdsp -> exec;
		knl_hook_stopfn = hdsp -> stop;
	} else {
		knl_hook_execfn = NULL;
		knl_hook_stopfn = NULL;
	}
	END_DISABLE_INTERRUPT;

	return E_OK;
}
#endif /* USE_FUNC_TD_HOK_DSP */

#ifdef USE_FUNC_TD_HOK_INT
SYSCALL ER td_hok_int( CONST TD_HINT *hint )
{
	BEGIN_DISABLE_INTERRUPT;
	if (hint != NULL) {
		knl_hook_ienterfn = hint -> enter;
		knl_hook_ileavefn = hint -> leave;
	} else {
		knl_hook_ienterfn = NULL;
		knl_hook_ileavefn = NULL;
	}
	END_DISABLE_INTERRUPT;

	return E_OK;
}
#endif /* USE_FUNC_TD_HOK_INT */

#endif /* USE_DBGSPT */
