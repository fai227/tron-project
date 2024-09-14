#include <tk/tkernel.h>
#include <tm/tmonitor.h>

#include "server.h"
#include "client.h"
#include "list.h"

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

				// 出発時刻を取得し表示
				UINT delay_until_departure = request_departure_time_ms();
				tm_printf("Departure Time: %d\n", delay_until_departure);


				List* order_list = list_init();
				while(TRUE) {
					// Bが押されるまで待機
					while((in_w(GPIO(P0, IN)) & (1 << 23)) != 0)  {
						tk_slp_tsk(1);
					}

					// 予約しリストの長さを表示
					reserve_order(order_list, delay_until_departure);
					tk_slp_tsk(1000);					
					tm_printf("Order List Length: %d\n", list_length(order_list));
				}
				tk_slp_tsk(TMO_FEVR);
			}
		}
	}

}