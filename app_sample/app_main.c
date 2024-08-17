#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "maqueen.h"

/* usermain関数 */
EXPORT void usermain(void)
{
    maqueen_init();
    while(1) {
        tm_printf("Sensor State: %d\n", read_line_state(M));
        tk_dly_tsk(1000);
    }
    return 0;
}
