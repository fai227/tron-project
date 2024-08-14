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
        err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, LEFT_DIRECTION, direction);
        if (err != E_OK) return err;
        err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, LEFT_SPEED, speed);
        if (err != E_OK) return err;
    } else if(motor == RIGHT_MOTOR){  // 右モーター
        err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, RIGHT_DIRECTION, direction);
        if (err != E_OK) return err;
        err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, RIGHT_SPEED, speed);
        if (err != E_OK) return err;
    } else if(motor == BOTH_MOTOR){ //両方のモーター
    	err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, LEFT_DIRECTION, direction);
    	if (err != E_OK) return err;
    	err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, RIGHT_DIRECTION, direction);
    	if (err != E_OK) return err;
    	err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, LEFT_SPEED, speed);
    	if (err != E_OK) return err;
    	err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, RIGHT_SPEED, speed);
    	if (err != E_OK) return err;
    }
    return E_OK;
}


// 全モーターを停止する関数
ER stop_all_motor(void) {
    ER err;
    err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, LEFT_SPEED, 0);  // 左モーター停止
    if (err != E_OK) return err;
    err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, RIGHT_SPEED, 0);  // 右モーター停止
    if (err != E_OK) return err;
    return E_OK;
}

// LEDを制御する関数
ER control_led(B led, BOOL state) {
    ER err;

    // led が左または右LEDでない場合はエラーを返す
    if(led != MAQUEEN_LED_LEFT && led != MAQUEEN_LED_RIGHT) {
        return E_PAR;
    }

    // state が 0 か 1 以外の場合はエラーを返す
    if (state > 1) {
        return E_PAR;
    }

    // LEDをオンまたはオフにする
    err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, led, state);
    if (err != E_OK) {
        return err;
    }

    return E_OK;
}


ER turn_off_all_led(void){
	ER err;
	err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, MAQUEEN_LED_LEFT, 0);
	if (err != E_OK) return err;
	err = iic_write(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, MAQUEEN_LED_RIGHT, 0);
	if (err != E_OK) return err;
    return E_OK;
}

BOOL read_line_state(B line) {
	return (iic_read(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, MAQUEEN_LINE_SENSOR_29) >> line) & 0x01;
}

// ラインセンサーの値(0-256)を読み取る関数
UB read_line_value(B ch, B line) {
    return (UB)iic_read(EXTERNAL_IIC_CHANNEL, MAQUEEN_IIC_ADDRESS, MAQUEEN_LINE_SENSOR + 2*line);
}


