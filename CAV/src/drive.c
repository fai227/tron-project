#include <tk/tkernel.h>
#include <tm/tmonitor.h>

#include "maqueen.h"
#include "list.h"
#include "client.h"
#include "order.h"
#include "STG.h"
#include "LED.h"

// 各区間の目標走行時間（ミリ秒）
#define D_TARGET_INTERVAL 2000
#define D_FORWARD_SPEED 60
#define D_FORWARD_SLOW_SPPED 20
#define D_BACKWARD_SPEED 25
#define D_STOP 0

#define D_DEFAULT_DELAY_TIME 0
#define D_DETECTION_INTERVAL_TURN 10
#define D_DETECTION_INTERVAL_TRACK 0

//#define D_DRIVE_TIMER TIMER2_BASE

#define D_DEBUG_PRINT 1 // 1でデバッグ出力を有効化、0で無効化

#if D_DEBUG_PRINT
#define DEBUG_LOG(fmt, ...) tm_printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) ((void)0)
#endif



#define D_LIST_MINIMUM_NUMBER 4 //リストがこの数を下回ったら、リクエスト送信

BOOL d_request_sent_flag = FALSE;
UH d_last_request_list_count = 0;

// 物理タイマーのクロック(MHz単位)
const INT d_timer_clock_mhz = 16;

// 物理タイマーの上限値
INT d_timer_limit = 16 * 1000 * 1000 * 10;

LOCAL INT calculate_departure_delay_s(List *order_list) { //リストにある経路の所要時間を計算
    UB delay_until_departure_s = 0;
    Element *pointer = order_list->head;

    while (pointer != order_list->tail && pointer != NULL) {
        delay_until_departure_s += get_order_duration(*(Order*)pointer->data);
        pointer = pointer->next;
    }

    return delay_until_departure_s;
}

LOCAL void process_orders(List *order_list) { //リクエスト後反映されるまでにリクエストを行わないようにする
    UH current_list_count = list_length(order_list);

    if (!d_request_sent_flag) {
        // リクエストを送信
        DEBUG_LOG("呼びまーす\n");
        INT departure_delay_s = calculate_departure_delay_s(order_list);
        reserve_order(order_list, departure_delay_s);

        // フラグを設定し、リスト個数を保存
        d_request_sent_flag = TRUE;
        d_last_request_list_count = current_list_count;
    } else if (d_request_sent_flag) {
        // リスト個数が増加したかチェック
        // DEBUG_LOG("%d\n",current_list_count);
        // DEBUG_LOG("%d\n",d_last_request_list_count);
        // if (current_list_count > d_last_request_list_count) {
        //     // リスト個数が増加したのでフラグをリセット
        //     d_request_sent_flag = FALSE;
        // }
    }
}

// 交差点かどうかを判定する関数
LOCAL BOOL is_intersection() {
    return read_line_state(MAQUEEN_LINE_SENSOR_L2) || read_line_state(MAQUEEN_LINE_SENSOR_R2);
}

// ライントラッキングを行う関数
LOCAL void line_tracking(INT timer_number, UINT duration_s) {
    INT flag = 0;  // 0: 初期状態または交差点上, 1: 交差点を離れた後
    UW current_time_count=0;
     while (TRUE) {
        BOOL right = read_line_state(MAQUEEN_LINE_SENSOR_R1);
        BOOL left = read_line_state(MAQUEEN_LINE_SENSOR_L1);
        BOOL middle = read_line_state(MAQUEEN_LINE_SENSOR_M);
        BOOL on_intersection = is_intersection();

        // 現在の時間を取得
        GetPhysicalTimerCount(timer_number, &current_time_count);
        UW elapsed_time = current_time_count / (d_timer_clock_mhz * 1000);  // ミリ秒単位に変換

        // 指定された時間を超えた場合、トラッキングを終了
        if (elapsed_time >= duration_s * 1000) {
            DEBUG_LOG("execute next order\n");
            control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, D_STOP);
            return;
        }

        // 交差点を離れた瞬間にフラグを1にする
        if (!on_intersection && flag == 0) {
            flag = 1;
            DEBUG_LOG("Left intersection, flag set to 1\n");
        }

        // フラグが1かつ交差点を検知したら、トラッキングを終了
        if (flag == 1 && on_intersection) {
            DEBUG_LOG("Reached next intersection, stopping tracking\n");
            control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, D_STOP);
            return;
        }

        // ライン追跡のロジック（常に実行）
        if (middle) {
            control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
        } else if (right) {
            control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SLOW_SPPED);
        } else if (left) {
            control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SLOW_SPPED);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
        }

        tk_slp_tsk(D_DETECTION_INTERVAL_TRACK);  // ms待機
    }
}

// 右折する関数
LOCAL void turn_right() {
    DEBUG_LOG("Start Right Turn\n");

    control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
    control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_BACKWARD, D_BACKWARD_SPEED);

    BOOL complete_firststep = FALSE;//右折開始後、L1MR1がラインを離れたらTrue
    while (TRUE) {
        if (!complete_firststep) {
            // ステップ1: ラインを外れるまで回転
            if (!read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                !read_line_state(MAQUEEN_LINE_SENSOR_R1) &&
                !read_line_state(MAQUEEN_LINE_SENSOR_L1)) {
                complete_firststep  = TRUE;
                DEBUG_LOG("Left the line\n");
            }
        } else {
            // ステップ2: 新しいラインを検出するまで回転し続ける
            if (read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                read_line_state(MAQUEEN_LINE_SENSOR_R1) &&
                !read_line_state(MAQUEEN_LINE_SENSOR_L2)) {
                DEBUG_LOG("New line found, stopping rotation\n");
                break;  // ループを抜けて回転を終了
            }
        }

         tk_slp_tsk(D_DETECTION_INTERVAL_TURN);  // ms待機
    }
  
}

// 左折関数
LOCAL void turn_left() {
    DEBUG_LOG("Start Left Turn\n");

    control_motor(LEFT_MOTOR, MAQUEEN_MOVE_BACKWARD, D_BACKWARD_SPEED);
    control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
  
    BOOL complete_firststep = FALSE;//右折開始後、L1MR1がラインを離れたらTrue
    while (TRUE) {
        if (!complete_firststep) {
            // ステップ1: ラインを外れるまで回転
            if (!read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                !read_line_state(MAQUEEN_LINE_SENSOR_L1) &&
                !read_line_state(MAQUEEN_LINE_SENSOR_R1)) {
                complete_firststep  = TRUE;
                DEBUG_LOG("Left the line\n");
            }
        } else {
            // ステップ2: 新しいラインを検出するまで回転し続ける
            if (read_line_state(MAQUEEN_LINE_SENSOR_M) &&
                read_line_state(MAQUEEN_LINE_SENSOR_L1) &&
                !read_line_state(MAQUEEN_LINE_SENSOR_R2)) {
                //DEBUG_LOG("New line found, stopping rotation\n");
                break;  // ループを抜けて回転を終了
            }
        }
        tk_slp_tsk(D_DETECTION_INTERVAL_TURN);  // 10ms待機
    }
    DEBUG_LOG("New line found, stopping rotation\n");
}

LOCAL void follow_path(Order order,INT timer_number) {
    UINT duration_s = get_order_duration(order);
    UW actual_duration_count=0;
    UW actual_duration_ms = 0;

    StartPhysicalTimer(timer_number, d_timer_limit, TA_CYC_PTMR);
    if(is_forward(order)){
        clear_led();
        show_strait();
        line_tracking(timer_number, duration_s);
    }
    else if(is_left(order)){
        clear_led();
        show_right();  // 前から見て左になるように逆向きを表示
        turn_left();
    }
    else if(is_right(order)){
        clear_led();
        show_left(); // 前から見て右になるように逆向きを表示
        turn_right();
    }

    GetPhysicalTimerCount(timer_number, &actual_duration_count);
    actual_duration_ms = actual_duration_count /(d_timer_clock_mhz*1000);
    actual_duration_ms += (actual_duration_ms >> 4);
    
    // 指定された時間まで待機
    if (actual_duration_ms< duration_s * 1000) {  // durationは秒単位
        DEBUG_LOG("(Required Time: %d ms)\n", actual_duration_ms);
        DEBUG_LOG("Addtional waiting time: %d ms\n", duration_s * 1000 - actual_duration_ms);
        
        stop_all_motor();
        tk_slp_tsk(duration_s * 1000 - actual_duration_ms);
    }
}

EXPORT void start_drive(UINT timer_number) {
    List* order_list = list_init();//経路を保存するリストの作成

    maqueen_init();//maqueenの初期化
    stop_all_motor();

    UINT departure_ms = request_departure_time_ms();//待機時間受け取り
    tm_printf("Departure Time: %d\n", departure_ms);

    UINT departure_s = departure_ms / 1000;
    reserve_order(order_list,departure_s);//listをグローバル変数にするとともに、送信タスクを起動

    tk_slp_tsk(departure_ms);//侵入可能時間まで待機

    // 進入用の指示を追加
    UB* order = (UB*)Kmalloc(sizeof(UB));
    *order = (MOVE_FORWARD << ORDER_BIT_SHIFT) | GRID_MOVE_TIME;  // 2秒前進
    

    list_unshift(order_list, order);

    //tk_slp_tsk(departure_ms);//侵入可能時間まで待機

    //タスクトレーサーが動作するかの確認
    for(UW i=0;i<10;i++){
        if (list_length(order_list) > d_last_request_list_count) {
            // リスト個数が増加したのでフラグをリセット
            d_request_sent_flag = FALSE;
        }
        if(list_length(order_list)<D_LIST_MINIMUM_NUMBER){
            process_orders(order_list);
        }

        void *data=list_get(order_list,0);
        Order* order = (Order*)data;
        if(data !=NULL){
            follow_path(*order,timer_number);
        }
        list_shift(order_list);
    }
    // while(TRUE){
    //     if (list_length(order_list) > d_last_request_list_count) {
    //         // リスト個数が増加したのでフラグをリセット
    //         d_request_sent_flag = FALSE;
    //     }
    //     if(list_length(order_list)<D_LIST_MINIMUM_NUMBER){
    //         process_orders(order_list);
    //     }

    //     void *data=list_get(order_list,0);
    //     Order* order = (Order*)data;
    //     if(data !=NULL){
    //         follow_path(*order,timer_number);
    //     }
    //     list_shift(order_list);
    // }
}
