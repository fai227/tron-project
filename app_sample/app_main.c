#include "tk/tkernel.h"
#include <tm/tmonitor.h>

#include "maqueen.h"

/* usermain関数 */
EXPORT INT usermain(void)
{
	// T_RVER	rver;

	// tm_putstring((UB*)"Start User-main program.\n");

	// tk_ref_ver(&rver);		/* Get the OS Version. */

	// tm_printf((UB*)"Make Code: %04x  Product ID: %04x\n", rver.maker, rver.prid);
	// tm_printf((UB*)"Product Ver. %04x\nProduct Num. %04x %04x %04x %04x\n", 
	// 		rver.prver, rver.prno[0],rver.prno[1],rver.prno[2],rver.prno[3]);

	// return 0;

	maqueen_init();
	whlle(1) {
    	tm_printf("Sensor State: %d\n", read_line_sensor_state(LINE_M));
		tk_dly_tsk(1000);
	}

	return 0;
}