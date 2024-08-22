#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "list.h"
#include "maqueen.h"

// usermain 関数
EXPORT void usermain(void) {
    // maqueen の初期化
    maqueen_init();

    // センサーの状態を1秒ごとに表示
    while(1) {
        tm_printf("Sensor State: %d\n", read_line_state(MAQUEEN_LINE_SENSOR_M));
        tk_dly_tsk(1000);
    }
}
