#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <sys/sysdepend/cpu/nrf5/sysdef.h>
#include "maqueen.h"

// 各区間の目標走行時間（ミリ秒）
#define TARGET_INTERVAL 2000
#define FORWARD_SPEED 35
#define BACKWARD_SPEED 25
#define STOP 0

#define DEFAULT_DELAY_TIME 1000
#define DETECTION_INTERVAL 10



#define DEBUG_PRINT 0  // 1でデバッグ出力を有効化、0で無効化

#if DEBUG_PRINT
#define DEBUG_LOG(fmt, ...) tm_printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) ((void)0)
#endif

static UINT start_time = 0;

// タイマーの現在値を読み取る関数
UINT read_timer_value() {
    *((volatile UW*)(TIMER0_BASE + TIMER_TASKS_CAPTURE(0))) = 1;
    return *((volatile UW*)(TIMER0_BASE + TIMER_CC(0)));


}

void start_timer() {
    // 高周波クロックを開始
    out_w(CLOCK_TASKS_HFCLKSTART, 1);
    while (!(in_w(CLOCK_EVENTS_HFCLKSTARTED)));

    // タイマーの設定
    out_w(TIMER0_BASE + TIMER_TASKS_STOP, 1);  // タイマーを停止
    out_w(TIMER0_BASE + TIMER_TASKS_CLEAR, 1);  // タイマーをクリア
    out_w(TIMER0_BASE + TIMER_MODE, 0);
    // モード0: タイマーモード（イベント発生時にカウンタが増加）
    out_w(TIMER0_BASE + TIMER_BITMODE, 3);
    // ビットモード3: 32ビットタイマー（0から2^32-1までカウント可能）
    out_w(TIMER0_BASE + TIMER_PRESCALER, 4);
    // プリスケーラー値4: 16MHz / 2^4 = 1MHz
    // タイマーのティック間隔は1μs（1MHz）となる
    out_w(TIMER0_BASE + TIMER_CC(0), 0xFFFFFFFF);
    // キャプチャ/コンペア値を最大値に設定
    // これにより、タイマーは連続的に動作し、オーバーフローしない

    // タイマーを開始
    out_w(TIMER0_BASE + TIMER_TASKS_START, 1);
    start_time = read_timer_value();
}


// タイマーを停止し、経過時間（ミリ秒）を返す関数
UINT stop_timer() {
    UINT end_time = read_timer_value();
    UINT elapsed;
    if (end_time >= start_time) {
        elapsed = end_time - start_time;
    } else {
        elapsed = (0xFFFFFFFF - start_time) + end_time + 1;
    }
    return elapsed / 1000;  // μsからmsへ変換
}

// 交差点かどうかを判定する関数
BOOL is_intersection() {
    return read_line_state(MAQUEEN_LINE_SENSOR_L2) || read_line_state(MAQUEEN_LINE_SENSOR_R2);
}

// 交差点で停止する関数
void stop_at_intersection() {
    stop_all_motor();
    tk_slp_tsk(DEFAULT_DELAY_TIME);  // 1秒待機
}

// 残り時間を待機する関数
void wait_remaining_time(UINT elapsed_time) {
    if (elapsed_time < TARGET_INTERVAL) {
        stop_all_motor();
        UINT remaining_time = TARGET_INTERVAL - elapsed_time;
        DEBUG_LOG("Addtional waiting time: %d ms\n", remaining_time);
        tk_slp_tsk(remaining_time);
    }
}

// ライントラッキングを行う関数
void line_tracking() {
    start_timer();
    while (!is_intersection()) {
        BOOL right = read_line_state(MAQUEEN_LINE_SENSOR_R1);
        BOOL left = read_line_state(MAQUEEN_LINE_SENSOR_L1);
        BOOL middle = read_line_state(MAQUEEN_LINE_SENSOR_M);

        // センサーの状態に応じてモーターを制御
        if (middle) {
            control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);
        } else if (right) {
            control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, STOP);
        } else if (left) {
            control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, STOP);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);
        }

		tk_slp_tsk(DETECTION_INTERVAL);  // ms待機
    }
    UINT tracking_time = stop_timer();
    DEBUG_LOG("tracking completed (Required Time: %d ms)\n", tracking_time);
    wait_remaining_time(tracking_time);
}

// 右折する関数
void turn_right() {
    DEBUG_LOG("Start Right Turn\n");
    start_timer();
    int flag = 0;

    control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);
    control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_BACKWARD, BACKWARD_SPEED);

    while (flag != 2) {
        switch (flag) {
            case 0:
                // ラインを外れるまで回転
                if (!read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_R1) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_L1)) {
                    flag = 1;
                    DEBUG_LOG("flag is %d\n", flag);
                }
                break;
            case 1:
                // 新しいラインを検出するまで回転
                if (read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                    read_line_state(MAQUEEN_LINE_SENSOR_R1) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_L2)) {
                    flag = 2;
                    DEBUG_LOG("flag is %d\n", flag);
                }
                break;
        }
        tk_slp_tsk(DETECTION_INTERVAL);  // ms待機
    }
    UINT turn_time = stop_timer();
    DEBUG_LOG("Turn Right Completed (Required Time: %d ms)\n", turn_time);
    wait_remaining_time(turn_time);
}

// 左折関数
void turn_left() {
    DEBUG_LOG("Start Right Turn\n");
    start_timer();
    int flag = 0;

    control_motor(LEFT_MOTOR, MAQUEEN_MOVE_BACKWARD, BACKWARD_SPEED);
    control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);

    while (flag != 2) {
        switch (flag) {
            case 0:
                // ラインを外れるまで待機
                if (!read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_L1) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_R1)) {
                    flag = 1;
                    DEBUG_LOG("flag is %d\n", flag);
                }
                break;
            case 1:
                // 新しいラインを検出するまで待機
                if (read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                    read_line_state(MAQUEEN_LINE_SENSOR_L1) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_R2)) {
                    flag = 2;
                    DEBUG_LOG("flag is %d\n", flag);
                }
                break;
        }
        tk_slp_tsk(DETECTION_INTERVAL);  // 10ms待機
    }
    UINT turn_time = stop_timer();
    DEBUG_LOG("Turn Right Completed (Time required: %d ms)\n", turn_time);
    wait_remaining_time(turn_time);
}

// 指定された経路に従ってmaqueenを動かす関数
void follow_path(char *path) {
    int order_index = 0; //経路（ex.LRL）のうち何個目か

    while (path[order_index] != '\0') {
        line_tracking();
        stop_at_intersection();
        switch (path[order_index]) {
            case 'W':
                tk_slp_tsk(2000);  // 2秒待機
                order_index++;
                switch(path[order_index]){
                    case 'R':
                        turn_right();
                        break;
                    case 'L':
                        turn_left();
                        break;
                    case 'S':
                        control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);
                        tk_slp_tsk(DEFAULT_DELAY_TIME);  // 1秒直進
                        break;
                    default:
                        DEBUG_LOG("Invalid direction: %c\n", path[order_index]);
                        return;
                }
                break;
            case 'R':
                turn_right();
                break;
            case 'L':
                turn_left();
                break;
            case 'S':
                control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);
                tk_slp_tsk(500);  // 0.5秒直進
                break;
            default:
                DEBUG_LOG("Invalid direction: %c\n", path[order_index]);
                return;
        }
        order_index++;
    }
    line_tracking();
    stop_at_intersection();
}
