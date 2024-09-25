#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "drive.h"
#include "server.h"
#include "client.h"
#include "list.h"
#include "LED.h"
#include "button.h"
#include "STG.h"
#include "list.h"
#include "maqueen.h"

#define LED_TIMER 1
#define STG_TIMER 2
#define DRIVE_TIMER 3

//関数のインポート
IMPORT void start_drive(UINT timer_number);


LOCAL void server_main() {
	// 時空間グリッドを物理タイマー1で起動
	stg_start(STG_TIMER);

	// サーバー起動
	start_server();
}

LOCAL void vehicle_main() {
	// 運転プログラムを起動
	start_drive(DRIVE_TIMER);
}

LOCAL void test_main() {
	UB test_number = 1;

	show_number(test_number);
	tm_printf("Test Number: %d\n", test_number);

	while(TRUE) {
		// Bが押されたら終了
		if(is_b_pressed()) {
			break;
		}

		// Aで数値をカウントアップ，Bで決定
		if(is_a_pressed()) {
			tk_slp_tsk(200);
			test_number++;
			show_number(test_number);
			tm_printf("Test Number: %d\n", test_number);
		} 
	}

	// テスト開始
	switch (test_number)
	{
		case 1: {
			// 時空間グリッド予約を行う
			List *order_list = list_init();
			maqueen_init();
			UINT delay = request_departure_time_ms();
			reserve_order(order_list, delay);
			while(list_length(order_list) > 0) {
				tm_printf("Order: %d\n", list_shift(order_list));
			}
			break;
		}

		case 2: {
			// maqueenのライントレーサーテスト
			maqueen_init();
			while(TRUE) {
				clear_led();

				UB line_state_l2 = read_line_state(MAQUEEN_LINE_SENSOR_L2);
				if(line_state_l2) {
					turn_on_led(2, 0);
				}

				UB line_state_l1 = read_line_state(MAQUEEN_LINE_SENSOR_L1);
				if(line_state_l1) {
					turn_on_led(2, 1);
				}


				UB line_state_m = read_line_state(MAQUEEN_LINE_SENSOR_M);
				if(line_state_m) {
					turn_on_led(2, 2);
				}

				UB line_state_r1 = read_line_state(MAQUEEN_LINE_SENSOR_R1);
				if(line_state_r1) {
					turn_on_led(2, 3);
				}

				UB line_state_r2 = read_line_state(MAQUEEN_LINE_SENSOR_R2);
				if(line_state_r2) {
					turn_on_led(2, 4);
				}

				tk_slp_tsk(100);
			}
			break;
		}

		case 3: {
			maqueen_init();

			control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, D_STOP);

			tk_slp_tsk(2000);

			stop_all_motor();

			break;
		}


	}
}

EXPORT INT usermain(void)
{
	// 表示用のLEDをで初期化
	initialize_led(LED_TIMER);

	// ロゴ表示
	show_logo();

	// ボタンの初期化
	button_setup();

	tm_printf("A:   Server\n");
	tm_printf("B:   Vehicle\n");
	tm_printf("A+B: Test\n");

	while(TRUE) {
		// どちらかが押されるまで待つ
		if(is_a_pressed() || is_b_pressed()) {
			tk_slp_tsk(200);

			BOOL a_pressed = is_a_pressed();
			BOOL b_pressed = is_b_pressed();	

			if(a_pressed && b_pressed) {
				tm_printf("Selected: Test\n");
				show_T();
				tk_slp_tsk(1000);

				test_main();
			} else if(a_pressed) {
				tm_printf("Selected: Server\n");
				show_S();
				tk_slp_tsk(1000);

				server_main();
			} else if(b_pressed) {
				tm_printf("Selected: Vehicle\n");
				show_V();
				tk_slp_tsk(1000);

				vehicle_main();
			} 
			//tk_slp_tsk(TMO_FEVR);
			DEBUG_LOG("finish process\n");
			return E_OK;

		}
	}
}
