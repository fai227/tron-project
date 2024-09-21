#include "STG.h"
#include "order.h"
#include "list.h"
#include "LED.h"

UB ***spatio_temporal_grid;
UW server_time;

// グリッド移動可能方向の定義
LOCAL UB grid_directions[GRID_SIZE][GRID_SIZE] = {
    {    DOWN_PATH | GRID_EMPTY,      LEFT_PATH | GRID_EMPTY,    LEFT_PATH | DOWN_PATH ,     LEFT_PATH | GRID_EMPTY,      LEFT_PATH | GRID_EMPTY      },
    {    DOWN_PATH | GRID_EMPTY,     GRID_EMPTY | GRID_EMPTY,   GRID_EMPTY | DOWN_PATH ,    GRID_EMPTY | GRID_EMPTY,        UP_PATH | GRID_EMPTY      },
    {    DOWN_PATH | RIGHT_PATH,     RIGHT_PATH | GRID_EMPTY,   RIGHT_PATH | DOWN_PATH ,    RIGHT_PATH | GRID_EMPTY,        UP_PATH | GRID_EMPTY      },
    {    DOWN_PATH | GRID_EMPTY,     GRID_EMPTY | GRID_EMPTY,   GRID_EMPTY | DOWN_PATH ,    GRID_EMPTY | GRID_EMPTY,        UP_PATH | GRID_EMPTY      },
    {   RIGHT_PATH | GRID_EMPTY,     RIGHT_PATH | GRID_EMPTY,   RIGHT_PATH | GRID_EMPTY,    RIGHT_PATH | GRID_EMPTY,        UP_PATH | GRID_EMPTY      }
};

// 探索用ノード構造体
typedef struct Node {
    struct Node *parent;
    Position position;
    UW h_departure_time; 
    UW g_distance; 
    BOOL has_moved;
    BOOL has_turned;
} Node;

LOCAL UB calculate_distance(Position start, Position target) {
    UB x1 = POS_X(start);
    UB y1 = POS_Y(start);
    UB x2 = POS_X(target);
    UB y2 = POS_Y(target);
    return (x1 > x2 ? x1 - x2 : x2 - x1) + (y1 > y2 ? y1 - y2 : y2 - y1);
}

LOCAL UW calculate_f(Node* node) {
    return node->h_departure_time + node->g_distance;
}

LOCAL UB get_moved_position(UB x, UB y, UB direction) {
    switch (direction) {
        case UP_PATH:
            return POS(x, y - 1);
        case DOWN_PATH:
            return POS(x, y + 1);
        case LEFT_PATH:
            return POS(x - 1, y);
        case RIGHT_PATH:    
            return POS(x + 1, y);
    }
}

LOCAL BOOL has_turned(UB next_x, UB next_y, Node* current_node) {
    // そこが初めての移動の場合は転回していないとする
    if(current_node->parent == NULL) {
        return FALSE;
    }

    UB previous_x = POS_X(current_node->parent->position);
    UB previous_y = POS_Y(current_node->parent->position);    

    // X・Y座標どちらも変化している場合は転回している
    if(previous_x != next_x && previous_y != next_y) {
        return TRUE;
    }

    return FALSE;
}

LOCAL BOOL check_grid(UW from, UW to, Position position, UB vehicle_id) {
    for(UW i = from; i <= to; i++) {
        UB grid = stg_get_grid(i, position);
        if(grid != GRID_EMPTY && grid != vehicle_id) {
            return FALSE;
        }
    }
    return TRUE;
}

LOCAL List* get_valid_moves(Node* current_node, Position target_position, UB vehicle_id) {
    UB x = POS_X(current_node->position);
    UB y = POS_Y(current_node->position);
    List* candidate_moves = list_init();

    // 上の移動行動
    if(grid_directions[y][x] & UP_PATH) {
        Node* up_node = (Node*)Kmalloc(sizeof(Node));
        Position moved_position = get_moved_position(x, y, UP_PATH);

        up_node->parent = current_node;
        up_node->position = moved_position;
        up_node->h_departure_time = current_node->h_departure_time + GRID_MOVE_TIME;
        up_node->g_distance = calculate_distance(moved_position, target_position);
        up_node->has_moved = TRUE;
        up_node->has_turned = FALSE;

        // 転回が入った場合は移動時間を加算
        if(has_turned(POS_X(moved_position), POS_Y(moved_position), current_node)) {
            up_node->h_departure_time += GRID_TURN_TIME;
            up_node->has_turned = TRUE;
        }

        list_append(candidate_moves, up_node);
        // tm_printf("Up Move at (%d, %d)\n", POS_X(moved_position), POS_Y(moved_position));
    }

    // 下の移動行動
    if(grid_directions[y][x] &  DOWN_PATH) {
        Node* down_node = (Node*)Kmalloc(sizeof(Node));
        Position moved_position = get_moved_position(x, y, DOWN_PATH);

        down_node->parent = current_node;
        down_node->position = moved_position;
        down_node->h_departure_time = current_node->h_departure_time + GRID_MOVE_TIME;
        down_node->g_distance = calculate_distance(moved_position, target_position);
        down_node->has_moved = TRUE;
        down_node->has_turned = FALSE;

        // 転回が入った場合は移動時間を加算
        if(has_turned(POS_X(moved_position), POS_Y(moved_position), current_node)) {
            down_node->h_departure_time += GRID_TURN_TIME;
            down_node->has_turned = TRUE;
        }

        list_append(candidate_moves, down_node);
        // tm_printf("Down Move at (%d, %d)\n", POS_X(moved_position), POS_Y(moved_position));
    }

    // 左の移動行動
    if(grid_directions[y][x] &  LEFT_PATH) {
        Node* left_node = (Node*)Kmalloc(sizeof(Node));
        Position moved_position = get_moved_position(x, y, LEFT_PATH);

        left_node->parent = current_node;
        left_node->position = moved_position;
        left_node->h_departure_time = current_node->h_departure_time + GRID_MOVE_TIME;
        left_node->g_distance = calculate_distance(moved_position, target_position);
        left_node->has_moved = TRUE;
        left_node->has_turned = FALSE;

        // 転回が入った場合は移動時間を加算
        if(has_turned(POS_X(moved_position), POS_Y(moved_position), current_node)) {
            left_node->h_departure_time += GRID_TURN_TIME;
            left_node->has_turned = TRUE;
        }

        list_append(candidate_moves, left_node);
        // tm_printf("Left Move at (%d, %d)\n", POS_X(moved_position), POS_Y(moved_position));
    }

    // 右の移動行動
    if(grid_directions[y][x] & RIGHT_PATH) {
        Node* right_node = (Node*)Kmalloc(sizeof(Node));
        Position moved_position = get_moved_position(x, y, RIGHT_PATH);

        right_node->parent = current_node;
        right_node->position = moved_position;
        right_node->h_departure_time = current_node->h_departure_time + GRID_MOVE_TIME;
        right_node->g_distance = calculate_distance(moved_position, target_position);
        right_node->has_moved = TRUE;
        right_node->has_turned = FALSE;

        // 転回が入った場合は移動時間を加算
        if(has_turned(POS_X(moved_position), POS_Y(moved_position), current_node)) {
            right_node->h_departure_time += GRID_TURN_TIME;
            right_node->has_turned = TRUE;
        }

        list_append(candidate_moves, right_node);
        // tm_printf("Right Move at (%d, %d)\n", POS_X(moved_position), POS_Y(moved_position));
    }

    // 待機行動（直進のみでは待機できない）
    if((x + y) % 2 == 0) {
        Node* wait_node = (Node*)Kmalloc(sizeof(Node));
        Position wait_position = current_node->position;
        wait_node->parent = current_node;
        wait_node->position = wait_position;
        wait_node->h_departure_time = current_node->h_departure_time + GRID_WAIT_TIME;
        wait_node->g_distance = calculate_distance(wait_position, target_position);
        wait_node->has_moved = FALSE;
        wait_node->has_turned = FALSE;
        list_append(candidate_moves, wait_node);
        // tm_printf("Wait at (%d, %d)\n", POS_X(wait_position), POS_Y(wait_position));
    }
    
    // tm_printf("Possible Move at (%d, %d): %d\n", POS_X(current_node->position), POS_Y(current_node->position), list_length(candidate_moves));

    // 予約不可能なものは除外
    List* valid_moves = list_init();
    while(list_length(candidate_moves) > 0) {
        Node* candidate = (Node*)list_shift(candidate_moves);
        // 開始位置と終了位置の両方で予約可能な場合のみ採用
        if(
            check_grid(current_node->h_departure_time, candidate->h_departure_time, current_node->position, vehicle_id)
            &&
            check_grid(current_node->h_departure_time, candidate->h_departure_time, candidate->position, vehicle_id)
        ) {
            list_append(valid_moves, candidate);
        } else {
            Kfree(candidate);
        }
    }

    list_delete(candidate_moves);   
    return valid_moves;
}

H index_of(Node *node, Node *list[], UH size) {
    for(UH i = 0; i < size; i++) {
        if(node->position == list[i]->position) {
            return i;
        }
    }
    return -1;
}

LOCAL Node* find_path(Position start_position, Position target_position, UW departure_time, UB vehicle_id) {
    // 初期設定
    Node *start = (Node*)Kmalloc(sizeof(Node));
    Node *open_list[GRID_SIZE * GRID_SIZE];
    Node *closed_list[GRID_SIZE * GRID_SIZE];
    UH open_size = 0;
    UH closed_size = 0;

    start->parent = NULL;
    start->position = start_position;
    start->h_departure_time = departure_time;
    start->g_distance = calculate_distance(start_position, target_position);
    open_list[open_size++] = start;

    // A*アルゴリズムにて経路探索
    while (open_size > 0) {
        // 最もF値が小さいノードを選択
        UH current_index = 0;
        for (UH i = 1; i < open_size; i++) {
            if (calculate_f(open_list[i]) < calculate_f(open_list[current_index])) {
                current_index = i;
            }
        }
        Node *current_node = open_list[current_index];

        // ゴールに到達した場合
        if (current_node->position == target_position) {
            return current_node;
        }

        // 現在のノードをオープンリストから削除し，クローズドリストに移動
        for (UH i = current_index; i < open_size - 1; i++) {
            open_list[i] = open_list[i + 1];
        }
        open_size--;
        closed_list[closed_size++] = current_node;

        // 移動可能な方向を取得
        List* valid_moves = get_valid_moves(current_node, target_position, vehicle_id);
        while(list_length(valid_moves) > 0) {
            Node* neighbor = (Node*)list_shift(valid_moves);

            // クローズドリストに含まれている場合（探索済み）はスキップ
            if(index_of(neighbor, closed_list, closed_size) >= 0) {
                Kfree(neighbor);
                continue;
            }

            H index = index_of(neighbor, open_list, open_size);
            // オープンリストに含まれていない場合
            if(index < 0) {
                // オープンリストに追加
                open_list[open_size++] = neighbor;
                neighbor->parent = current_node;
            }
            // オープンリストに含まれている場合
            else {
                // より良い経路の場合
                if(neighbor->g_distance < open_list[index]->g_distance) {
                    Kfree(open_list[index]);
                    open_list[index] = neighbor;
                    neighbor->parent = current_node;
                }
                // 別の経路がより良い場合はスキップ
                else {
                    Kfree(neighbor);
                }
            }
        }
    }

    // ゴールに到達できなかった場合
    tm_printf("\n---------- No path found!!! ----------\n");
    tm_printf("Vehicle : %d\n", vehicle_id);
    tm_printf("Start  : (%d, %d)\n", POS_X(start_position), POS_Y(start_position));
    tm_printf("Target : (%d, %d)\n", POS_X(target_position), POS_Y(target_position));
    tm_printf("--------------------------------------\n");
}


LOCAL void stg_handler() {
    // 古いデータを削除
    for(UH x = 0; x < GRID_SIZE; x++) {
        for(UH y = 0; y < GRID_SIZE; y++) {
            stg_set_grid(server_time, POS(x, y), GRID_EMPTY);
        }
    }

    // サーバー時間を進める
    server_time++;
    tm_printf("Server Time: %d\n", server_time);

    // グリッド状態を表示
    clear_led();
    for(UH y = 0; y < GRID_SIZE; y++) {
        for(UH x = 0; x < GRID_SIZE; x++) {
            if(stg_get_grid(server_time, POS(x, y)) != GRID_EMPTY) {
                turn_on_led(y, x);
            }
        }
    }
}

T_DPTMR stg_timer_handler = {
    0,
    TA_HLNG,
    &stg_handler
};
LOCAL const INT stg_physical_timer_clock_mhz = 16;	// 物理タイマのクロック(MHz単位)

LOCAL const INT stg_cycle_micros = 1000 * 1000;		// ハンドラの起動周期(μs単位)、1000*1000μs＝1s
INT stg_limit = stg_cycle_micros * stg_physical_timer_clock_mhz - 1;	// 物理タイマの上限値
EXPORT void stg_start(UB timer_number) {
    // 時空間グリッドの初期化
    server_time = 0;
    spatio_temporal_grid = (UB***)Kmalloc(sizeof(UB**) * GRID_SIZE);
    for(UH y = 0; y < GRID_SIZE; y++) {
        spatio_temporal_grid[y] = (UB**)Kmalloc(sizeof(UB*) * GRID_SIZE);
        for(UH x = 0; x < GRID_SIZE; x++) {
            spatio_temporal_grid[y][x] = (UB*)Kmalloc(sizeof(UB) * STG_BUFFER_LENGTH);
            for(UW i = 0; i < STG_BUFFER_LENGTH; i++) {
                spatio_temporal_grid[y][x][i] = GRID_EMPTY;
            }
        }
    }

    // 物理タイマーの起動
    DefinePhysicalTimerHandler(timer_number, &stg_timer_handler);
    StartPhysicalTimer(timer_number, stg_limit, TA_CYC_PTMR);
}

EXPORT void stg_reserve(Order *orders, UB max_order_size, UB vehicle_id, UB delay_until_departure, Position start_position, Position target_position) {
    UW departure_time = server_time + delay_until_departure;
    
    // 経路探索
    Node* path = find_path(start_position, target_position, departure_time, vehicle_id);

    // リストへの変換（ゴールから辿りリストの先頭に入れていく）
    List* path_list = list_init();
    while(path != NULL) {
        list_unshift(path_list, path);
        path = path->parent;
    }

    // 経路予約と指示への変換
    UH order_index = 0;
    Node* previous_node = (Node*)list_shift(path_list);
    while(list_length(path_list) > 0) {
        Node* next_node = (Node*)list_shift(path_list);
        
        // 時空間グリッド予約
        for(UW i = previous_node->h_departure_time; i <= next_node->h_departure_time; i++) {
            stg_set_grid(i, previous_node->position, vehicle_id);
            stg_set_grid(i, next_node->position, vehicle_id);
        }

        // 指示変換
        if(!next_node->has_moved) {  // 待機指示
            orders[order_index++] = GRID_WAIT_TIME;
        }
        else if(next_node->has_turned) {  // 転回指示
            // 中央の場合は右折
            if(next_node->position == POS(2,3)) {
                orders[order_index++] = (TURN_RIGHT << ORDER_BIT_SHIFT) | GRID_TURN_TIME;
                tm_printf("(%d, %d)\n", POS_X(next_node->position), POS_Y(next_node->position));
            }
            // それ以外は左折
            else {
                orders[order_index++] = (TURN_LEFT << ORDER_BIT_SHIFT) | GRID_TURN_TIME;
            }
            orders[order_index++] = (MOVE_FORWARD << ORDER_BIT_SHIFT) | GRID_MOVE_TIME;
        }
        else {  // 前進指示
            orders[order_index++] = (MOVE_FORWARD << ORDER_BIT_SHIFT) | GRID_MOVE_TIME;
        }

        Kfree(previous_node);
        previous_node = next_node;
    }
    // 最後に左折指示を追加
    orders[order_index++] = (TURN_LEFT << ORDER_BIT_SHIFT) | GRID_TURN_TIME;
    for(UW i = previous_node->h_departure_time; i <= previous_node->h_departure_time + 2; i++) {
        stg_set_grid(i, previous_node->position, vehicle_id);
    }
    Kfree(previous_node);

#if STG_VERBOSE
    // 予約内容を表示
    tm_printf("\n---------- Reservation ----------\n");
    tm_printf("Time   : %d\n", departure_time);
    tm_printf("Start  : (%d, %d)\n", POS_X(start_position), POS_Y(start_position));
    tm_printf("Target : (%d, %d)\n", POS_X(target_position), POS_Y(target_position));
    tm_printf("---------------------------------\n");
#endif
}

EXPORT UW stg_get_departure_time() {
    // (0, 0)に侵入可能時間を計算（1秒はずらす）
    UW delay_until_departure = 1;
    while(TRUE) {
        if(stg_get_grid(delay_until_departure + server_time, POS(0, 0)) == GRID_EMPTY) {
            return delay_until_departure;
        }
        delay_until_departure++;
    }
}

LOCAL UB stg_get_grid(UW time, Position Position) {
    return spatio_temporal_grid[POS_Y(Position)][POS_X(Position)][time % STG_BUFFER_LENGTH];
}

LOCAL void stg_set_grid(UW time, Position Position, UB value) {
    spatio_temporal_grid[POS_Y(Position)][POS_X(Position)][time % STG_BUFFER_LENGTH] = value;
}

