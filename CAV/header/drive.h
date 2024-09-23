#ifndef DRIVE_H_
#define DRIVE_H_

#include <tm/tmonitor.h>
#include <sys/sysdepend/cpu/nrf5/sysdef.h>
#include "maqueen.h"
#include "list.h"
#include "client.h"
#include "order.h"
#include "LED.h"

// マクロ定義
#define D_TARGET_INTERVAL 2000
#define D_FORWARD_SPEED 35
#define D_BACKWARD_SPEED 25
#define D_STOP 0

#define D_DEFAULT_DELAY_TIME 0
#define D_DETECTION_INTERVAL 10

#define D_DEBUG_PRINT 1

#define D_LIST_MINIMUM_NUMBER 4

// デバッグ用マクロ
#if D_DEBUG_PRINT
#define DEBUG_LOG(fmt, ...) tm_printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) ((void)0)
#endif

EXPORT void start_drive(UINT timer_number);

#endif /* DRIVE_H_ */
