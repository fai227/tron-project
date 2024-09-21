#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <sys/sysdepend/cpu/nrf5/sysdef.h>
#include "maqueen.h"
#include "list.h"
#include "client.h"
#include "order.h"
#include "LED.h"

// 各区間の目標走行時間（ミリ秒）
#define D_TARGET_INTERVAL 2000
#define D_FORWARD_SPEED 35
#define D_BACKWARD_SPEED 25
#define D_STOP 0

#define D_DEFAULT_DELAY_TIME 0
#define D_DETECTION_INTERVAL 10

//#define D_DRIVE_TIMER TIMER2_BASE

#define D_DEBUG_PRINT 0  // 1でデバッグ出力を有効化、0で無効化

#if D_DEBUG_PRINT
#define DEBUG_LOG(fmt, ...) tm_printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) ((void)0)
#endif

UINT start_time = 0;

#define D_LIST_MINIMUM_NUMBER 4 //リストがこの数を下回ったら、リクエスト送信

BOOL d_request_sent_flag = FALSE;
UH d_last_request_list_count = 0;

// 物理タイマーのクロック(MHz単位)
const INT d_timer_clock_mhz = 16;

// ハンドラの起動周期(μs単位)、1000μs＝1ms
const INT d_timer_cycle_micros = 1000;

// 物理タイマーの上限値
INT d_timer_limit = d_timer_cycle_micros * d_timer_clock_mhz - 1;

// 経過時間をカウントする変数
UW d_elapsed_count_s = 0;
void d_time_measurement_handler(void *exinf)
{
    d_elapsed_count_s++;
}
T_DPTMR d_time_measurement_timer = {0, TA_HLNG, &d_time_measurement_handler};

void initialize_timer(UINT timer_number)
{
    DefinePhysicalTimerHandler(timer_number, &d_time_measurement_timer);
}

void start_timer(UINT timer_number)
{

    // 経過時間をリセット
    d_elapsed_count_s = 0;

    // 物理タイマーを起動
    StartPhysicalTimer(timer_number, d_timer_limit, TA_CYC_PTMR);
}

UW stop_timer(UINT timer_number)
{
    StopPhysicalTimer(timer_number);
    return d_elapsed_count_s;
}

// 交差点かどうかを判定する関数
BOOL is_intersection() {
    return read_line_state(MAQUEEN_LINE_SENSOR_L2) || read_line_state(MAQUEEN_LINE_SENSOR_R2);
}

// ライントラッキングを行う関数
void line_tracking() {
    //start_timer();
    while (!is_intersection()) {
        BOOL right = read_line_state(MAQUEEN_LINE_SENSOR_R1);
        BOOL left = read_line_state(MAQUEEN_LINE_SENSOR_L1);
        BOOL middle = read_line_state(MAQUEEN_LINE_SENSOR_M);

        // センサーの状態に応じてモーターを制御
        if (middle) {
            control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
        } else if (right) {
            control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, D_STOP);
        } else if (left) {
            control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, D_STOP);
            control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, D_FORWARD_SPEED);
        }

		tk_slp_tsk(D_DETECTION_INTERVAL);  // ms待機
    }
}

// 右折する関数
void turn_right() {
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

         tk_slp_tsk(D_DETECTION_INTERVAL);  // ms待機
    }
  
}

// 左折関数
void turn_left() {
    DEBUG_LOG("Start Right Turn\n");

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
                DEBUG_LOG("New line found, stopping rotation\n");
                break;  // ループを抜けて回転を終了
            }
        }
        tk_slp_tsk(D_DETECTION_INTERVAL);  // 10ms待機
    }
}

void follow_path(Order order,INT timer_number) {
    UINT duration_s = get_order_duration(order);
    UINT actual_duration_ms = 0;

    start_timer(timer_number);

    if(is_forward(order)){
        clear_led();
        show_strait();
        line_tracking();
    }
    else if(is_left(order)){
        clear_led();
        show_left();
        turn_left();
    }
    else if(is_right(order)){
        clear_led();
        show_right();
        turn_right();
    }

    actual_duration_ms = stop_timer(timer_number);
    
    // 指定された時間まで待機
    if (actual_duration_ms< duration_s * 1000) {  // durationは秒単位
        DEBUG_LOG("(Required Time: %d ms)\n", actual_duration_ms);
        DEBUG_LOG("Addtional waiting time: %d ms\n", duration_s * 1000 - actual_duration_ms);
        
        stop_all_motor();
        tk_slp_tsk(duration_s * 1000 - actual_duration_ms);
    }
}

void start_drive(UINT timer_number) {
    List* order_list=list_init();//経路を保存するリストの作成

    initialize_timer(timer_number);//タイマの初期化
    maqueen_init();//maqueenの初期化

    UINT departure_ms=request_departure_time_ms();//待機時間受け取り
    tm_printf("Departure Time: %d\n", departure_ms);

    UINT departure_s = departure_ms / 1000;
    reserve_order(order_list,departure_s);//listをグローバル変数にするとともに、送信タスクを起動

    // 進入用の指示を追加
    UB* order = (UB*)Kmalloc(sizeof(UB));
    *order = 0b10000010;  // 2秒前進
    list_unshift(order_list, order);

    tk_slp_tsk(departure_ms);//侵入可能時間まで待機


    while(TRUE){
        if(list_length(order_list)<D_LIST_MINIMUM_NUMBER){
            INT departure_delay_s= calculate_departure_delay_s(order_list);
            process_orders(order_list);
        }

        void *data=list_get(order_list,0);
        Order* order = (Order*)data;
        if(data !=NULL){
            follow_path(*order,timer_number);
        }
        list_shift(order_list);
    }
}

INT calculate_departure_delay_s(List *order_list) { //リストにある経路の所要時間を計算
    UB delay_until_departure_s = 0;
    Element *pointer = order_list->head;

    while (pointer != order_list->tail && pointer != NULL) {
        delay_until_departure_s += get_order_duration((Order*)pointer->data);
        pointer = pointer->next;
    }

    return delay_until_departure_s;
}

void process_orders(List *order_list) { //リクエスト後反映されるまでにリクエストを行わないようにする
    UH current_list_count = list_length(order_list);

    if (!d_request_sent_flag && current_list_count <= 4) {
        // リクエストを送信
        INT departure_delay_s = calculate_departure_delay_s(order_list);
        reserve_order(order_list, departure_delay_s);

        // フラグを設定し、リスト個数を保存
        d_request_sent_flag = TRUE;
        d_last_request_list_count = current_list_count;
    } else if (d_request_sent_flag) {
        // リスト個数が増加したかチェック
        if (current_list_count > d_last_request_list_count) {
            // リスト個数が増加したのでフラグをリセット
            d_request_sent_flag = FALSE;
        }
    }
}
