#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <sys/sysdepend/cpu/nrf5/sysdef.h>
#include "maqueen.h"
#include "list.h"
#include "client.h"
#include "order.h"

// 各区間の目標走行時間（ミリ秒）
#define TARGET_INTERVAL 2000
#define FORWARD_SPEED 35
#define BACKWARD_SPEED 25
#define STOP 0

#define DEFAULT_DELAY_TIME 0
#define DETECTION_INTERVAL 10

#define DRIVE_TIMER TIMER0_BASE

#define DEBUG_PRINT 0  // 1でデバッグ出力を有効化、0で無効化

#if DEBUG_PRINT
#define DEBUG_LOG(fmt, ...) tm_printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_LOG(fmt, ...) ((void)0)
#endif

static UINT start_time = 0;

#define LIST_MIN 4 //指示四つ文
volatile BOOL request_sent_flag = FALSE;
volatile UH last_request_list_count = 0;

// タイマーの現在値を読み取る関数
// UINT read_timer_value() {
//     *((volatile UW*)(DRIVE_TIMER + TIMER_TASKS_CAPTURE(0))) = 1;
//     return *((volatile UW*)(DRIVE_TIMER + TIMER_CC(0)));


// }
UINT read_timer_value() {
    // タイマーのキャプチャタスクをトリガー
    out_w(DRIVE_TIMER + TIMER_TASKS_CAPTURE(0), 1);
    
    // キャプチャされた値を読み取り
    return (UINT)in_w(DRIVE_TIMER + TIMER_CC(0));
}
void intialize_timer(){
        // 高周波クロックを開始
    out_w(CLOCK_TASKS_HFCLKSTART, 1);
    while (!(in_w(CLOCK_EVENTS_HFCLKSTARTED)));

    // タイマーの設定
    out_w(DRIVE_TIMER + TIMER_TASKS_STOP, 1);  // タイマーを停止
    out_w(DRIVE_TIMER + TIMER_TASKS_CLEAR, 1);  // タイマーをクリア
    out_w(DRIVE_TIMER + TIMER_MODE, 0);
    // モード0: タイマーモード（イベント発生時にカウンタが増加）
    out_w(DRIVE_TIMER + TIMER_BITMODE, 3);
    // ビットモード3: 32ビットタイマー（0から2^32-1までカウント可能）
    out_w(DRIVE_TIMER + TIMER_PRESCALER, 4);
    // プリスケーラー値4: 16MHz / 2^4 = 1MHz
    // タイマーのティック間隔は1μs（1MHz）となる
}


void start_timer() {
    out_w(DRIVE_TIMER + TIMER_TASKS_CLEAR, 1); 
    out_w(DRIVE_TIMER + TIMER_CC(0), 0xFFFFFFFF);
    // キャプチャ/コンペア値を最大値に設定
    // これにより、タイマーは連続的に動作し、オーバーフローしない

    // タイマーを開始
    out_w(DRIVE_TIMER + TIMER_TASKS_START, 1);
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
    //start_timer();
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


 }

// 右折する関数
void turn_right() {
    DEBUG_LOG("Start Right Turn\n");
    //start_timer();
    INT flag = 0;

    control_motor(LEFT_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);
    control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_BACKWARD, BACKWARD_SPEED);



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

         tk_slp_tsk(DETECTION_INTERVAL);  // ms待機
    }
  
}

// 左折関数
void turn_left() {
    DEBUG_LOG("Start Right Turn\n");
    //start_timer();
    INT flag = 0;

    control_motor(LEFT_MOTOR, MAQUEEN_MOVE_BACKWARD, BACKWARD_SPEED);
    control_motor(RIGHT_MOTOR, MAQUEEN_MOVE_FORWARD, FORWARD_SPEED);

  
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
        tk_slp_tsk(DETECTION_INTERVAL);  // 10ms待機
    }
}

void follow_path(Order order) {

    UINT duration = order & ORDER_MASK;
    UINT actual_duration = 0;

    start_timer();

    if(is_forward(order)){
        line_tracking();
    }
    else if(is_left(order)){
        turn_left();
    }
    else if(is_right(order)){
        turn_right();
    }

    actual_duration = stop_timer();
    
    // 指定された時間まで待機
    if (actual_duration < duration * 1000) {  // durationは秒単位
    
    DEBUG_LOG("(Required Time: %d ms)\n", actual_duration);
    DEBUG_LOG("Addtional waiting time: %d ms\n", duration * 1000 - actual_duration);
    control_motor(BOTH_MOTOR, MAQUEEN_MOVE_FORWARD, STOP);

    tk_slp_tsk(duration * 1000 - actual_duration);
}
        
}



void start_drive(){
    List* order_list=list_init();//経路を保存するリストの作成
    intialize_timer();//タイマの初期化
    maqueen_init();//maqueenの初期化
    UINT departure_ms=request_departure_time_ms();//待機時間受け取り
    tm_printf("Departure Time: %d\n",departure_ms );

    UINT departure_second=departure_ms/1000;
    reserve_order(order_list,departure_second);//listをグローバル変数にするとともに、送信タスクを起動
    tk_slp_tsk(departure_ms);//侵入可能時間まで待機


    while(TRUE){
        if(list_length(order_list)<LIST_MIN){
            INT departure_delay= calculate_departure_delay(order_list);
            process_orders(order_list);

        }
        void *data=list_get(order_list,0);
        Order* order = (Order*)data;
        if(data !=NULL){
            follow_path(*order);
        }
        delete_first_element(order_list);



        }

    }


INT calculate_departure_delay(List *order_list) { //リストにある経路の所要時間を計算
    UB delay_until_departure = 0;
    Element *pointer = order_list->head;

    while (pointer != order_list->tail && pointer != NULL) {
        delay_until_departure += get_order_duration((Order*)pointer->data);
        pointer = pointer->next;
    }

    return delay_until_departure;
}

void process_orders(List *order_list) { //リクエスト後反映されるまでにリクエストを行わないようにする
    UH current_list_count = list_length(order_list);

    if (!request_sent_flag && current_list_count <= 4) {
        // リクエストを送信
        INT departure_delay = calculate_departure_delay(order_list);
        reserve_order(order_list, departure_delay);

        // フラグを設定し、リスト個数を保存
        request_sent_flag = TRUE;
        last_request_list_count = current_list_count;
    } else if (request_sent_flag) {
        // リスト個数が増加したかチェック
        if (current_list_count > last_request_list_count) {
            // リスト個数が増加したのでフラグをリセット
            request_sent_flag = FALSE;
        }
    }
}
// リストの先頭要素を削除する関数
void delete_first_element(List *list)
{
    if (list == NULL || list->head == NULL)
    {
        // リストが空または無効な場合、何もしない
        return;
    }

    Element *first = list->head;
    list->head = first->next;

    // リストの長さを減少
    list->length--;

    // リストが空になった場合、tail も NULL に設定
    if (list->head == NULL)
    {
        list->tail = NULL;
    }

    // 先頭要素のメモリを解放
    Kfree(first);

}



