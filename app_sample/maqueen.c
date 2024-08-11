#include "maqueen.h"
#include <tk/tkernel.h>

// Maqueenの初期化
ER maqueen_init(void) {
    tm_printf("Starting maqueen_init...\n");

    ER err;

    // I2C通信の初期化
    err = iicsetup(TRUE);
    if (err != E_OK) {
        tm_printf("I2C setup failed with error: %d\n", err);
        return err;
    }
    tm_printf("I2C setup successful.\n");

    UB version = 0;
    int retry_count = 0;
    const int max_retries = 10;

    while (version == 0 && retry_count < max_retries) {
        tm_printf("Attempting to write to sensor...\n");
        err = write_reg(1, MAQUEEN_ADDRESS, SENSOR, 0);  // チャンネル1を使用
        if (err != E_OK) {
            tm_printf("Failed to write to sensor. Error: %d\n", err);
            return err;
        }
        tm_printf("Successfully wrote to sensor.\n");

        version = read_reg(1, MAQUEEN_ADDRESS, SENSOR);
        if (version == 0) {
            tm_printf("Read sensor version failed. Version: %d (Retry: %d)\n", version, retry_count);
        } else {
            tm_printf("Read sensor version: %d (Retry: %d)\n", version, retry_count);
        }

        tk_dly_tsk(500);
        retry_count++;
    }

    if (version == 0) {
        tm_printf("Maqueen version check failed after %d retries\n", retry_count);
        return E_OBJ;
    }

    tm_printf("Maqueen initialized successfully. Version: %d\n", version);
    return E_OK;
}


// モーターを制御する関数
ER control_motor(B motor, B direction, B speed) {
    ER err;
    if (motor == 0) {  // 左モーター
        err = iic_write(1, MAQUEEN_ADDRESS, LEFT_DIRECTION, direction);
        if (err != E_OK) return err;
        err = iic_write(1, MAQUEEN_ADDRESS, LEFT_SPEED, speed);
        if (err != E_OK) return err;
    } else {  // 右モーター
        err = iic_write(1, MAQUEEN_ADDRESS, RIGHT_DIRECTION, direction);
        if (err != E_OK) return err;
        err = iic_write(1, MAQUEEN_ADDRESS, RIGHT_SPEED, speed);
        if (err != E_OK) return err;
    }
    return E_OK;
}

// 全モーターを停止する関数
ER control_motor_stop_all(void) {
    ER err;
    err = iic_write(1, MAQUEEN_ADDRESS, LEFT_SPEED, 0);  // 左モーター停止
    if (err != E_OK) return err;
    err = iic_write(1, MAQUEEN_ADDRESS, RIGHT_SPEED, 0);  // 右モーター停止
    if (err != E_OK) return err;
    return E_OK;
}

// LEDを制御する関数
ER control_led(B led, B state) {
    // 処理を行わずに0を返す
    return 0;
}

// ラインセンサーの状態を読み取る関数
B read_line_sensor_state(B line) {
    UB value = iic_read(1, MAQUEEN_ADDRESS, SENSOR + line);
    return value ? 1 : 0;
}

// ラインセンサーの値を読み取る関数
UB read_line_sensor_value(B line) {
    return iic_read(1, MAQUEEN_ADDRESS, SENSOR + line);
}

// 超音波センサーの距離を読み取る関数
UW read_ultrasonic(void) {
    // 処理を行わずに0を返す
    return 0;
}
