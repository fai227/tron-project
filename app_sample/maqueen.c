#include "maqueen.h"
#include <tk/tkernel.h>

// Maqueenの初期化
ER maqueen_init(void) {
    ER err;

    // I2C通信の初期化
    err = iicsetup(TRUE);
    if (err != E_OK) {
        tm_printf("Error: I2C setup failed with code %d\n", err);
        return err;
    }

    // Maqueenのバージョンを確認
    UB version = 0;
    int retry_count = 0;
    const int max_retries = 10;  // 最大リトライ回数

    while (version == 0 && retry_count < max_retries) {
        iic_write(2, MAQUEEN_ADDRESS, SENSOR, 0);  // チャンネル2を使用
        version = iic_read(2, MAQUEEN_ADDRESS, SENSOR);
        tk_dly_tsk(500);
        retry_count++;
    }

    if (version == 0) {
        tm_printf("Error: Maqueen version check failed after %d retries\n", retry_count);
        return E_OBJ;
    }

    tm_printf("Maqueen initialized successfully. Version: %d\n", version);
    return E_OK;
}


// モーターを制御する関数
ER control_motor(B motor, B direction, B speed) {
    if (motor == 0) {  // 左モーター
        iic_write(2, MAQUEEN_ADDRESS, LEFT_DIRECTION, direction);
        iic_write(2, MAQUEEN_ADDRESS, LEFT_SPEED, speed);
    } else {  // 右モーター
        iic_write(2, MAQUEEN_ADDRESS, RIGHT_DIRECTION, direction);
        iic_write(2, MAQUEEN_ADDRESS, RIGHT_SPEED, speed);
    }
    return E_OK;
}

// 全モーターを停止する関数
ER control_motor_stop_all(void) {
    iic_write(2, MAQUEEN_ADDRESS, LEFT_SPEED, 0);  // 左モーター停止
    iic_write(2, MAQUEEN_ADDRESS, RIGHT_SPEED, 0);  // 右モーター停止
    return E_OK;
}

// LEDを制御する関数
ER control_led(B led, B state) {
    if (led == 0) {
        iic_write(2, MAQUEEN_ADDRESS, LEFT_DIRECTION, state);  // 仮にLEDの制御として定義
    } else {
        iic_write(2, MAQUEEN_ADDRESS, RIGHT_DIRECTION, state);  // 仮にLEDの制御として定義
    }
    return E_OK;
}

// ラインセンサーの状態を読み取る関数
B read_line_sensor_state(B line) {
    UB value = iic_read(2, MAQUEEN_ADDRESS, SENSOR + line);
    return value ? 1 : 0;
}

// ラインセンサーの値を読み取る関数
UB read_line_sensor_value(B line) {
    return iic_read(2, MAQUEEN_ADDRESS, SENSOR + line);
}

// 超音波センサーの距離を読み取る関数
UW read_ultrasonic(void) {
    UB high_byte = iic_read(2, MAQUEEN_ADDRESS, SENSOR);  // 仮に超音波センサーとして定義
    UB low_byte = iic_read(2, MAQUEEN_ADDRESS, SENSOR + 1);
    return (high_byte << 8) | low_byte;
}
