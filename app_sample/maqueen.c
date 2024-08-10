#include "maqueen.h"
#include "iic.h"

void I2C_init(void) {
    // I2C通信の初期化を行うiicsetup
    iic_setup(TRUE);
}

ER maqueen_init(void) {
    // Maqueenの初期化処理
    // 例えば、センサーの初期化やデバイスの確認を行います
    return E_OK;
}



ER control_motor(B motor, B direction, B speed) {
    INT reg_direction, reg_speed;
    if (motor == MOTOR_L) {
        reg_direction = LEFT_DIRECTION;
        reg_speed = LEFT_SPEED;
    } else {
        reg_direction = RIGHT_DIRECTION;
        reg_speed = RIGHT_SPEED;
    }
    write_reg(I2C_CH, MAQUEEN, reg_direction, direction);
    write_reg(I2C_CH, MAQUEEN, reg_speed, speed);
    return E_OK;
}

ER control_motor_stop_all(void) {
    // 両方のモーターを停止
    write_reg(I2C_CH, MAQUEEN, LEFT_SPEED, 0);
    write_reg(I2C_CH, MAQUEEN, RIGHT_SPEED, 0);
    return E_OK;
}

ER control_led(B led, B state) {
    // LEDの制御
    INT reg = (led == LED_L) ? RGB_L : RGB_R;
    return write_reg(I2C_CH, MAQUEEN, reg, state);
}

B read_line_sensor_state(B line) {
    // ラインセンサーの状態を読み取る
    UB value = read_reg(I2C_CH, MAQUEEN, SENSOR + line);
    return (value > 0) ? 1 : 0;
}

UB read_line_sensor_value(B line) {
    // ラインセンサーの値を読み取る
    return read_reg(I2C_CH, MAQUEEN, SENSOR + line);
}

UW read_ultrasonic(void) {
    // 超音波センサーの読み取り処理
    // 実装の具体例として、以下のように書けますが、実際のセンサーの動作に応じた処理が必要です
    UB high_byte = read_reg(I2C_CH, MAQUEEN, ULTRASONIC_TRIG_PIN);
    UB low_byte = read_reg(I2C_CH, MAQUEEN, ULTRASONIC_ECHO_PIN);
    return (UW)((high_byte << 8) | low_byte);
}
