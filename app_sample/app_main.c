#include "tk/tkernel.h"
#include <tm/tmonitor.h>
#include "maqueen.h"

/* usermain関数 */
EXPORT INT usermain(void)
{
    maqueen_init();
    while(1) {
        tm_printf("Sensor State: %d\n", read_line_state(M));
        tk_dly_tsk(1000);
    }

    return 0;
}

/*EXPORT INT usermain(void)
{
    maqueen_init();
    stop_all_motor();
    while(1) {
        control_motor(LEFT_MOTOR,FORWARD,50);
        tk_dly_tsk(1000);
    }

    return 0;
}*/

/*
EXPORT INT usermain(void)
{
    maqueen_init();
    control_motor_stop_all();
    stop_all_led();
    while(1) {
    	//control_led(MAQUEEN_LED_LEFT, 1);
        tk_dly_tsk(1000);
    }

    return 0;
}
*/
