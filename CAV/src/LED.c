#include "LED.h"
#include <tk/tkernel.h>


UB led_matrix[5] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

void set_gpio(UW pin, UW value)
{
	INT port_addr;

    // COL4のみP1
	if(pin == COL4){
		if(value) {
			port_addr = GPIO(P1, OUTSET);
        }
		else {
			port_addr = GPIO(P1, OUTCLR);
        }
	} else {
		if(value) {
			port_addr = GPIO(P0, OUTSET);
        }
		else {
			port_addr = GPIO(P0, OUTCLR);
        }
	}
	out_w(port_addr, (1 << pin));
}

void set_led_col(UW column) {
    set_gpio(COL1, led_matrix[column] & 0b10000 ? 0 : 1);
    set_gpio(COL2, led_matrix[column] & 0b01000 ? 0 : 1);
    set_gpio(COL3, led_matrix[column] & 0b00100 ? 0 : 1);
    set_gpio(COL4, led_matrix[column] & 0b00010 ? 0 : 1);
    set_gpio(COL5, led_matrix[column] & 0b00001 ? 0 : 1);
}

// rowで指定した行のみを点灯
LOCAL void set_row_gpio(UW row)
{
    set_gpio(ROW1, row == 0 ? 1 : 0);
    set_gpio(ROW2, row == 1 ? 1 : 0);
    set_gpio(ROW3, row == 2 ? 1 : 0);
    set_gpio(ROW4, row == 3 ? 1 : 0);
    set_gpio(ROW5, row == 4 ? 1 : 0);
}

// LEDの表示行を切り替える物理タイマハンドラ
LOCAL W	led_disp_row = 5;		// 表示中のLEDの行の番号
LOCAL void led_switch_row_handler(void *exinf)
{
    // 1行ずつ点灯
    led_disp_row = (led_disp_row + 1) % 5;

	set_row_gpio(led_disp_row);
	set_led_col(led_disp_row);
}

// 物理タイマによるダイナミック点灯
T_DPTMR led_dynamic_physical_timer = {0, TA_HLNG, &led_switch_row_handler};	// ハンドラ定義情報
const INT led_physical_timer_clock_mhz = 16;	// 物理タイマのクロック(MHz単位)

const INT led_cycle_micros = 1000;		// ハンドラの起動周期(μs単位)、1000μs＝1ms
INT led_limit = led_cycle_micros * led_physical_timer_clock_mhz - 1;	// 物理タイマの上限値

void initialize_led(UB timer_number){
    // GPIOピンを出力に設定
    out_w(GPIO(P0, PIN_CNF(COL1)), 1);
    out_w(GPIO(P0, PIN_CNF(COL2)), 1);
    out_w(GPIO(P0, PIN_CNF(COL3)), 1);
    out_w(GPIO(P1, PIN_CNF(COL4)), 1);
    out_w(GPIO(P0, PIN_CNF(COL5)), 1);
    out_w(GPIO(P0, PIN_CNF(ROW1)), 1);
    out_w(GPIO(P0, PIN_CNF(ROW2)), 1);
    out_w(GPIO(P0, PIN_CNF(ROW3)), 1);
    out_w(GPIO(P0, PIN_CNF(ROW4)), 1);
    out_w(GPIO(P0, PIN_CNF(ROW5)), 1);

    // 画面をクリア
    clear_led();

    // 物理タイマーを起動
    DefinePhysicalTimerHandler(timer_number, &led_dynamic_physical_timer);
    StartPhysicalTimer(timer_number, led_limit, TA_CYC_PTMR);
}

void clear_led(){
    for (int i = 0; i < 5; i++) {
        led_matrix[i] = 0b00000;
    }
}

void turn_on_led(UW row, UW column){
    led_matrix[row] |= 1 << (4 - column);
}

void show_number(UB number) {
    switch (number)
    {
        case 0:
            led_matrix[0] = 0b01110;
            led_matrix[1] = 0b01010;
            led_matrix[2] = 0b01010;
            led_matrix[3] = 0b01010;
            led_matrix[4] = 0b01110;
            return;

        case 1:
            led_matrix[0] = 0b00100;
            led_matrix[1] = 0b01100;
            led_matrix[2] = 0b00100;
            led_matrix[3] = 0b00100;
            led_matrix[4] = 0b01110;
            return;

        case 2:
            led_matrix[0] = 0b01110;
            led_matrix[1] = 0b00010;
            led_matrix[2] = 0b01110;
            led_matrix[3] = 0b01000;
            led_matrix[4] = 0b01110;
            return;

        case 3:
            led_matrix[0] = 0b01110;
            led_matrix[1] = 0b00010;
            led_matrix[2] = 0b01110;
            led_matrix[3] = 0b00010;
            led_matrix[4] = 0b01110;
            return;

        case 4:
            led_matrix[0] = 0b01010;
            led_matrix[1] = 0b01010;
            led_matrix[2] = 0b01110;
            led_matrix[3] = 0b00010;
            led_matrix[4] = 0b00010;
            return;

        case 5:
            led_matrix[0] = 0b01110;
            led_matrix[1] = 0b01000;
            led_matrix[2] = 0b01110;
            led_matrix[3] = 0b00010;
            led_matrix[4] = 0b01110;
            return;

        case 6:
            led_matrix[0] = 0b01110;
            led_matrix[1] = 0b01000;
            led_matrix[2] = 0b01110;
            led_matrix[3] = 0b01010;
            led_matrix[4] = 0b01110;
            return;

        case 7:
            led_matrix[0] = 0b01110;
            led_matrix[1] = 0b01010;
            led_matrix[2] = 0b00010;
            led_matrix[3] = 0b00010;
            led_matrix[4] = 0b00010;
            return;

        case 8:
            led_matrix[0] = 0b01110;
            led_matrix[1] = 0b01010;
            led_matrix[2] = 0b01110;
            led_matrix[3] = 0b01010;
            led_matrix[4] = 0b01110;
            return;

        case 9:
            led_matrix[0] = 0b01110;
            led_matrix[1] = 0b01010;
            led_matrix[2] = 0b01110;
            led_matrix[3] = 0b00010;
            led_matrix[4] = 0b01110;
            return;

        default:
            show_cross();
            return;
    }
}

void show_circle(){
    led_matrix[0] = 0b01110;
    led_matrix[1] = 0b10001;
    led_matrix[2] = 0b10001;
    led_matrix[3] = 0b10001;
    led_matrix[4] = 0b01110;
}

void show_cross() {
    led_matrix[0] = 0b10001;
    led_matrix[1] = 0b01010;
    led_matrix[2] = 0b00100;
    led_matrix[3] = 0b01010;
    led_matrix[4] = 0b10001;
}

void show_S() {
    led_matrix[0] = 0b00110;
    led_matrix[1] = 0b01000;
    led_matrix[2] = 0b01110;
    led_matrix[3] = 0b00010;
    led_matrix[4] = 0b01100;
}

void show_V() {
    led_matrix[0] = 0b10001;
    led_matrix[1] = 0b10001;
    led_matrix[2] = 0b01010;
    led_matrix[3] = 0b01010;
    led_matrix[4] = 0b00100;
}

void show_T() {
    led_matrix[0] = 0b01110;
    led_matrix[1] = 0b00100;
    led_matrix[2] = 0b00100;
    led_matrix[3] = 0b00100;
    led_matrix[4] = 0b00100;
}

void show_logo() {
    // N
    led_matrix[0] = 0b10001;
    led_matrix[1] = 0b11001;
    led_matrix[2] = 0b10101;
    led_matrix[3] = 0b10011;
    led_matrix[4] = 0b10001;
    tk_slp_tsk(400);

    // I
    led_matrix[0] = 0b01110;
    led_matrix[1] = 0b00100;
    led_matrix[2] = 0b00100;
    led_matrix[3] = 0b00100;
    led_matrix[4] = 0b01110;
    tk_slp_tsk(400);

    // S
    show_S();
    tk_slp_tsk(400);
    
    // L
    led_matrix[0] = 0b10000;
    led_matrix[1] = 0b10000;
    led_matrix[2] = 0b10000;
    led_matrix[3] = 0b10000;
    led_matrix[4] = 0b11111;
    tk_slp_tsk(400);

    // A
    led_matrix[0] = 0b00100;
    led_matrix[1] = 0b01010;
    led_matrix[2] = 0b10001;
    led_matrix[3] = 0b11111;
    led_matrix[4] = 0b10001;
    tk_slp_tsk(400);

    // B
    led_matrix[0] = 0b11110;
    led_matrix[1] = 0b10001;
    led_matrix[2] = 0b11110;
    led_matrix[3] = 0b10001;
    led_matrix[4] = 0b11110;
    tk_slp_tsk(400);

    clear_led();
}

void show_strait() {
    led_matrix[0] = 0b00100;
    led_matrix[1] = 0b01110;
    led_matrix[2] = 0b00100;
    led_matrix[3] = 0b00100;
    led_matrix[4] = 0b00100;
}

void show_left() {
    led_matrix[0] = 0b01000;
    led_matrix[1] = 0b11110;
    led_matrix[2] = 0b01010;
    led_matrix[3] = 0b00010;
    led_matrix[4] = 0b00010;
}

void show_right() {
    led_matrix[0] = 0b00010;
    led_matrix[1] = 0b01111;
    led_matrix[2] = 0b01010;
    led_matrix[3] = 0b01000;
    led_matrix[4] = 0b01000;
}

void show_stop() {
    show_cross();
}