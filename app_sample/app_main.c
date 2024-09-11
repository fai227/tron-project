#include <tk/tkernel.h>
#include <tm/tmonitor.h>

#include "server.h"

EXPORT INT usermain(void)
{
	// Aボタン (P0.14), Bボタン (P0.23) から入力するように設定
    out_w(GPIO(P0, PIN_CNF(14)), 0);
    out_w(GPIO(P0, PIN_CNF(23)), 0);

	tm_printf("A: Server\n");
	tm_printf("B: Vehicle\n");

	// AかBが押されるまで待機
	while(TRUE) {
		UW gpio_in = in_w(GPIO(P0, IN));
		BOOL a_pressed = (gpio_in & (1 << 14)) == 0;
		BOOL b_pressed = (gpio_in & (1 << 23)) == 0;

		if(a_pressed || b_pressed) {
			if(a_pressed) {
				tm_printf("Selected: Server\n");
				start_server();
				tk_slp_tsk(TMO_FEVR);
			} else {
				tm_printf("Selected: Vehicle\n");
				test_send(); // ここをdrive.cのメインループに変更
				tk_slp_tsk(TMO_FEVR);
			}
		}
	}

}