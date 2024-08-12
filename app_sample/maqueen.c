#include "maqueen.h"
#include <tk/tkernel.h>
#include <tm/tmonitor.h>  // tm_printfの宣言を含むヘッダファイル

// Maqueenの初期化
ER maqueen_init(void) {
    ER err;
    // I2C通信の初期化
    err = iicsetup(TRUE);
    if (err != E_OK) {
        return err;
    }
    return E_OK;
}

// モーターを制御する関数
ER control_motor(B motor, B direction, B speed) {
    ER err;
    if (motor == LEFT_MOTOR) {  // 左モーター
        err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_DIRECTION,  speed < 0 ? BACKWARD : FORWARD);
        if (err != E_OK) return err;
        err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_SPEED, abs(speed));
        if (err != E_OK) return err;
    } else if(motor == RIGHT_MOTOR){  // 右モーター
        err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_DIRECTION, speed < 0 ? BACKWARD : FORWARD);
        tm_printf("direction set\n");
        if (err != E_OK) return err;
        err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_SPEED, abs(speed));
        tm_printf("speed set\n");
        if (err != E_OK) return err;
    } else if(motor == BOTH_MOTOR){ //両方のモーター
    	err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_DIRECTION, direction);
    	err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_DIRECTION,  speed < 0 ? BACKWARD : FORWARD);
    	if (err != E_OK) return err;
    	err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_SPEED, abs(speed));
    	err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_SPEED, abs(speed));
    	if (err != E_OK) return err;
    }
    return E_OK;
}


// 全モーターを停止する関数
ER stop_all_motor(void) {
    ER err;
    err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_SPEED, 0);  // 左モーター停止
    if (err != E_OK) return err;
    err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_SPEED, 0);  // 右モーター停止
    if (err != E_OK) return err;
    return E_OK;
}

// LEDを制御する関数
ER control_led(B led, B state) {
    ER err;
    B led_reg;

    // LEDの位置に基づいてレジスタアドレスを設定
    if (led == MAQUEEN_LED_LEFT) {
        led_reg = MAQUEEN_LED_LEFT;  // 左LEDのレジスタアドレスを設定
    } else if (led == MAQUEEN_LED_RIGHT) {
        led_reg = MAQUEEN_LED_LEFT;  // 右LEDのレジスタアドレスを設定
    } else {
        return E_PAR;  // 無効な引数の場合はエラーを返す
    }

    // LEDをオンまたはオフにする
    if (state == 1) {
        err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, led_reg, 1);  // LEDをオン
    } else if (state == 0) {
        err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, led_reg, 0);  // LEDをオフ
    } else {
        return E_PAR;  // 無効なstateの場合はエラーを返す
    }

    if (err != E_OK) {
        return err;  // エラーが発生した場合、エラーコードを返す
    }

    return E_OK;  // 正常終了
}

ER stop_all_led(void){
	ER err;
	B led_reg;
	B led_reg2;
	led_reg = MAQUEEN_LED_LEFT;
	led_reg2 = MAQUEEN_LED_RIGHT;
	err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, led_reg, 0);
	if (err != E_OK) return err;
	err = write_reg(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, led_reg2, 0);
	if (err != E_OK) return err;
	    return E_OK;
}


// ラインセンサーの状態を読み取る関数01
B read_line_state(B line) {
    UB value = read_reg(1, MAQUEEN_ADDRESS,0x1D );
    UB value2 = value & (1<<line);
    return value2>>line;

}

// ラインセンサーの値を読み取る関数
UB read_line_value(B line) {
    return read_reg(1, MAQUEEN_ADDRESS, SENSOR + 2*line);
}

