/*
 * タスクトレーサ : ヘッダーファイル
 *
 * Copyright (C) 2009-2017 by Personal Media Corporation
 */

#ifndef __TRACE_TRACE_H__
#define __TRACE_TRACE_H__

int usermain(void)
{
    static char __attribute__ ((aligned(4))) trace_mem[3088 + ((7 + 5) * 4 * 2000)];
    extern int trace_init(void *mem, int memsz, int maxtskid, void *entry);
#define usermain trace_umain
    extern int usermain();
    trace_init(trace_mem, sizeof(trace_mem),
#if defined(CFN_MAX_TSKID)
               CFN_MAX_TSKID,
#else
               32,
#endif
               usermain);
    return 0;
}

#endif
