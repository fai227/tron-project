#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "maqueen.h"


#define TARGET_INTERVAL 2000  // 目標間隔（ミリ秒）

// クロックとタイマーのレジスタ定義
#define CLOCK_BASE      0x40000000
#define CLOCK_TASKS_HFCLKSTART (CLOCK_BASE + 0x000)
#define CLOCK_EVENTS_HFCLKSTARTED (CLOCK_BASE + 0x100)

#define TIMER0_BASE     0x40008000
#define TIMER_TASKS_START (0x000)
#define TIMER_TASKS_STOP  (0x004)
#define TIMER_TASKS_CLEAR (0x00C)
#define TIMER_TASKS_CAPTURE(n) (0x040 + (n)*4)
#define TIMER_MODE      (0x504)
#define TIMER_BITMODE   (0x508)
#define TIMER_PRESCALER (0x510)
#define TIMER_CC(n)     (0x540 + (n)*4)

static UINT start_time = 0;

UINT read_timer_value() {
    *((volatile UW*)(TIMER0_BASE + TIMER_TASKS_CAPTURE(0))) = 1;
    return *((volatile UW*)(TIMER0_BASE + TIMER_CC(0)));
}

void start_timer() {
    *((volatile UW*)CLOCK_TASKS_HFCLKSTART) = 1;
    while (!(*((volatile UW*)CLOCK_EVENTS_HFCLKSTARTED)));

    *((volatile UW*)(TIMER0_BASE + TIMER_TASKS_STOP)) = 1;
    *((volatile UW*)(TIMER0_BASE + TIMER_TASKS_CLEAR)) = 1;
    *((volatile UW*)(TIMER0_BASE + TIMER_MODE)) = 0;
    *((volatile UW*)(TIMER0_BASE + TIMER_BITMODE)) = 3;
    *((volatile UW*)(TIMER0_BASE + TIMER_PRESCALER)) = 4;
    *((volatile UW*)(TIMER0_BASE + TIMER_CC(0))) = 0xFFFFFFFF;

    *((volatile UW*)(TIMER0_BASE + TIMER_TASKS_START)) = 1;

    start_time = read_timer_value();
}

UINT stop_timer() {
    UINT end_time = read_timer_value();
    UINT elapsed;
    if (end_time >= start_time) {
        elapsed = end_time - start_time;
    } else {
        elapsed = (0xFFFFFFFF - start_time) + end_time + 1;
    }
    return elapsed / 1000;
}

BOOL is_intersection() {
    return read_line_state(MAQUEEN_LINE_SENSOR_L2) || read_line_state(MAQUEEN_LINE_SENSOR_R2);
}

void stop_at_intersection() {
    stop_all_motor();
    tk_dly_tsk(1000);
}

void wait_remaining_time(UINT elapsed_time) {
    if (elapsed_time < TARGET_INTERVAL) {
        stop_all_motor();
        UINT remaining_time = TARGET_INTERVAL - elapsed_time;
        tm_printf("追加待機時間: %d ms\n", remaining_time);
        tk_dly_tsk(remaining_time);
    }
}

void line_tracking() {
    start_timer();
    while (!is_intersection()) {
        BOOL right = read_line_state(MAQUEEN_LINE_SENSOR_R1);
        BOOL left = read_line_state(MAQUEEN_LINE_SENSOR_L1);
        BOOL middle = read_line_state(MAQUEEN_LINE_SENSOR_M);

        if (middle) {
            control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, 35);
        } else if (right) {
            control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, 35);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, 0);
        } else if (left) {
            control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, 0);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, 35);
        }

        tk_dly_tsk(10);
    }
    UINT tracking_time = stop_timer();
    tm_printf("ライントラッキング完了 (所要時間: %d ms)\n", tracking_time);
    wait_remaining_time(tracking_time);
}

void turn_right() {
    tm_printf("右折開始\n");
    start_timer();
    int flag = 0;

    control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, 35);
    control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_BACKWARD, 25);

    while (flag != 2) {
        switch (flag) {
            case 0:
                if (!read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_R1) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_L1)) {
                    flag = 1;
                    tm_printf("右折中flagは%d\n", flag);
                }
                break;
            case 1:
                if (read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                    read_line_state(MAQUEEN_LINE_SENSOR_R1) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_L2)) {
                    flag = 2;
                    tm_printf("右折中flagは%d\n", flag);
                }
                break;
        }
        tk_dly_tsk(10);
    }
    UINT turn_time = stop_timer();
    tm_printf("右折完了 (所要時間: %d ms)\n", turn_time);
    wait_remaining_time(turn_time);
}

void turn_left() {
    tm_printf("左折開始\n");
    start_timer();
    int flag = 0;

    control_motor(LEFT_MOTOR, MAQUEEN_MOVE_BACKWARD, 25);
    control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, 35);

    while (flag != 2) {
        switch (flag) {
            case 0:
                if (!read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_L1) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_R1)) {
                    flag = 1;
                    tm_printf("左折中flagは%d\n", flag);
                }
                break;
            case 1:
                if (read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                    read_line_state(MAQUEEN_LINE_SENSOR_L1) &&
                    !read_line_state(MAQUEEN_LINE_SENSOR_R2)) {
                    flag = 2;
                    tm_printf("左折中flagは%d\n", flag);
                }
                break;
        }
        tk_dly_tsk(10);
    }
    UINT turn_time = stop_timer();
    tm_printf("左折完了 (所要時間: %d ms)\n", turn_time);
    wait_remaining_time(turn_time);
}

void follow_path(char *path) {
    int i = 0;

    while (path[i] != '\0') {
        line_tracking();
        stop_at_intersection();
        switch (path[i]) {
            case 'W':
                tk_dly_tsk(2000);
                i++;
                switch(path[i]){
                    case 'R':
                        turn_right();
                        break;
                    case 'L':
                        turn_left();
                        break;
                    case 'S':
                        control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, 30);
                        tk_dly_tsk(500);
                        break;
                    default:
                        tm_printf("Invalid direction: %c\n", path[i]);
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
                control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, 30);
                tk_dly_tsk(500);
                break;
            default:
                tm_printf("Invalid direction: %c\n", path[i]);
                return;
        }
        i++;
    }
    line_tracking();
    stop_at_intersection();
}
