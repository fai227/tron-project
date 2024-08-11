
#include "tk/tkernel.h"
#include <tm/tmonitor.h>

#include "maqueen.h"

/* usermain関数 */

EXPORT INT usermain(void)
{
    tm_printf("Starting usermain...\n");

    // Maqueenの初期化
    ER err = maqueen_init();
    if (err != E_OK) {
        tm_printf("Maqueen initialization failed with error: %d\n", err);
        return err;
    }

    tm_printf("Maqueen initialized successfully.\n");

    while (1) {
        tm_printf("Attempting to read sensor state...\n");

        // センサーの状態を読み取る
        B sensor_state = read_line_sensor_state(M);

        if (sensor_state >= 0) {
            tm_printf("Sensor state: %d\n", sensor_state);
        } else {
            tm_printf("Failed to read sensor state.\n");
        }

        // 適切な遅延時間を設定
        tk_dly_tsk(SENSOR_DELAY);
    }

    return 0;
}


