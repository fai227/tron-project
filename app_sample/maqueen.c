#include "maqueen.h"
#include <tk/tkernel.h>
#include <tm/tmonitor.h>  // tm_printfの宣言を含むヘッダファイル

// Maqueenの初期化
ER maqueen_init(void) {
	return iic_setup(TRUE);
}

// モーターを制御する関数
ER control_motor(B motor, UB direction, UB speed) {
    ER err;
    if (motor == LEFT_MOTOR) {  // 左モーター
        err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_DIRECTION, direction);
        if (err != E_OK) return err;
        err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_SPEED, speed);
        if (err != E_OK) return err;
    } else if(motor == RIGHT_MOTOR){  // 右モーター
        err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_DIRECTION, direction);
        if (err != E_OK) return err;
        err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_SPEED, speed);
        if (err != E_OK) return err;
    } else if(motor == BOTH_MOTOR){ //両方のモーター
    	err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_DIRECTION, direction);
    	if (err != E_OK) return err;
    	err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_DIRECTION, direction);
    	if (err != E_OK) return err;
    	err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_SPEED, speed);
    	if (err != E_OK) return err;
    	err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_SPEED, speed);
    	if (err != E_OK) return err;
    }
    return E_OK;
}


// 全モーターを停止する関数
ER stop_all_motor(void) {
    ER err;
    err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, LEFT_SPEED, 0);  // 左モーター停止
    if (err != E_OK) return err;
    err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, RIGHT_SPEED, 0);  // 右モーター停止
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
        led_reg = MAQUEEN_LED_RIGHT;  // 右LEDのレジスタアドレスを設定
    } else {
        return E_PAR;  // 無効な引数の場合はエラーを返す
    }

    // state が 0 か 1 以外の場合はエラーを返す
    if (state > 1) {
        return E_PAR;  // 無効なstateの場合はエラーを返す
    }

    // LEDをオンまたはオフにする
    err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, led_reg, state);
    if (err != E_OK) {
        return err;  // エラーが発生した場合、エラーコードを返す
    }

    return E_OK;  // 正常終了
}


ER turn_off_all_led(void){
	ER err;
	B led_reg;
	B led_reg2;
	led_reg = MAQUEEN_LED_LEFT;
	led_reg2 = MAQUEEN_LED_RIGHT;
	err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, led_reg, 0);
	if (err != E_OK) return err;
	err = iic_write(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, led_reg2, 0);
	if (err != E_OK) return err;
	    return E_OK;
}


/*// ラインセンサーの状態を読み取る関数01
B read_line_state(B line) {
    UB value = iic_read(1, MAQUEEN_ADDRESS,0x1D );
    UB value2 = value & (1<<line);
    return value2>>line;
}*/

B read_line_state(B line) {
	return (iic_read(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, MAQUEEN_LINE_SENSOR_29) >> line) & 0x01;
}
//iic_read(1, MAQUEEN_ADDRESS, 0x1D) で値を読み取る
//>> line で指定したライン番号のビットを最下位ビットに移動させる
//& 0x01 でその最下位ビットのみを抽出する

// ラインセンサーの値(0-256)を読み取る関数
UB read_line_value(B ch, B line) {
    return (UB)iic_read(EXTERNAL_I2C_CHANNEL, MAQUEEN_ADDRESS, MAQUEEN_LINE_SENSOR + 2*line);
}


