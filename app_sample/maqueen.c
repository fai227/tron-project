#include "maqueen.h"
#include "iic.h"  // 必要なI2C通信のヘッダファイルをインクルード

// Function implementations

ER maqueen_init(void) {
    UB version = 0;  // バージョン情報の初期化
    while (version == 0) {  // バージョンが0の間ループ
        write_reg(MAQUEEN, VERSION_CNT_REGISTER, 0);
        read_reg(MAQUEEN, VERSION_CNT_REGISTER, &version);
        tk_dly_tsk(500);  // 500ミリ秒の遅延
    }
    return E_OK;
}

ER control_motor(B motor, B direction, B speed) {
    switch (motor) {  // モーターの種類で分岐
    case MOTOR_L:  // 左モーターの場合
        write_reg(MAQUEEN, LEFT_DIRECTION, direction);
        write_reg(MAQUEEN, LEFT_SPEED, speed);
        break;
    case MOTOR_R:  // 右モーターの場合
        write_reg(MAQUEEN, RIGHT_DIRECTION, direction);
        write_reg(MAQUEEN, RIGHT_SPEED, speed);
        break;
    default:
        return E_PAR;
    }
    return E_OK;
}

ER control_motor_stop_all(void) {
    control_motor(MOTOR_L, FORWARD, 0);
    control_motor(MOTOR_R, FORWARD, 0);
    return E_OK;
}

ER control_led(B led, B state) {
    switch (led) {  // LEDの種類で分岐
    case LIGHT_L:  // 左LEDの場合
        write_reg(MAQUEEN, LEFT_LED_REGISTER, state);
        break;
    case LIGHT_R:  // 右LEDの場合
        write_reg(MAQUEEN, RIGHT_LED_REGISTER, state);
        break;
    default:
        return E_PAR;
    }
    return E_OK;
}

B read_line_sensor_state(B line) {
    UB data = 0;  // データの初期化
    read_reg(MAQUEEN, SENSOR, &data);
    B state;  // ステートの変数
    switch (line) {  // センサーの種類で分岐
    case LINE_L1:  // 左1センサーの場合
        state = (data & 0x08) ? 1 : 0;  // ステートの判定
        break;
    case LINE_M:  // 中央センサーの場合
        state = (data & 0x04) ? 1 : 0;  // ステートの判定
        break;
    case LINE_R1:  // 右1センサーの場合
        state = (data & 0x02) ? 1 : 0;  // ステートの判定
        break;
    case LINE_L2:  // 左2センサーの場合
        state = (data & 0x10) ? 1 : 0;  // ステートの判定
        break;
    case LINE_R2:  // 右2センサーの場合
        state = (data & 0x01) ? 1 : 0;  // ステートの判定
        break;
    default:
        state = 0;
    }
    return state;  // ステートの返却
}

UB read_line_sensor_value(B line) {
    UB buffer[2];  // バッファの定義
    UB data = 0;  // データの初期化
    switch (line) {  // センサーの種類で分岐
    case LINE_R2:  // 右2センサーの場合
        read_reg(MAQUEEN, ADC0_REGISTER, buffer);
        break;
    case LINE_R1:  // 右1センサーの場合
        read_reg(MAQUEEN, ADC1_REGISTER, buffer);
        break;
    case LINE_M:  // 中央センサーの場合
        read_reg(MAQUEEN, ADC2_REGISTER, buffer);
        break;
    case LINE_L1:  // 左1センサーの場合
        read_reg(MAQUEEN, ADC3_REGISTER, buffer);
        break;
    case LINE_L2:  // 左2センサーの場合
        read_reg(MAQUEEN, ADC4_REGISTER, buffer);
        break;
    default:
        return 0;
    }
    data = (buffer[1] << 8) | buffer[0];  // データの合成
    return data;  // データの返却
}

UW read_ultrasonic(void) {
    // トリガーピンを出力モードに設定
    tk_set_pin_mode(ULTRASONIC_TRIG_PIN, OUTPUT);
    // エコーピンを入力モードに設定
    tk_set_pin_mode(ULTRASONIC_ECHO_PIN, INPUT);

    // トリガーピンをLOWに設定
    tk_set_pin_low(ULTRASONIC_TRIG_PIN);
    tk_dly_tsk(2);  // 2マイクロ秒待機

    // トリガーピンをHIGHに設定し、10マイクロ秒待機後、再度LOWに設定
    tk_set_pin_high(ULTRASONIC_TRIG_PIN);
    tk_dly_tsk(10);  // 10マイクロ秒待機
    tk_set_pin_low(ULTRASONIC_TRIG_PIN);

    // エコーピンからのパルス幅を計測
    UW duration = tk_pulse_in(ULTRASONIC_ECHO_PIN, HIGH, 500 * 58);
    // 距離に変換
    UW distance = duration / 59;
    // 距離が0以下の場合
    if (distance <= 0) {
        return 0;  // 0を返却
    }
    // 距離が500より大きい場合
    if (distance > 500) {
        return 500;  // 500を返却
    }
    return distance;  // 距離の返却
}
