#include <tk/tkernel.h>
#include <tm/tmonitor.h>

#include "radio.h"


/* 初期化処理 */
EXPORT void usermain(void)
{
	tm_printf("Hello, World!\n");
	while(TRUE);
}
