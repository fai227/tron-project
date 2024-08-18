#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "list.h"
#include "maqueen.h"

// usermain 関数
EXPORT void usermain(void) {
    // リストの初期化
    List *list = init_list();

    // list_unshiftのテスト（リストの先頭に追加）
    int data1 = 10;
    int data2 = 20;
    int data3 = 30;
    list_unshift(list, &data1);
    list_unshift(list, &data2);
    list_unshift(list, &data3);

    //ここでリストの中身は[30 20 10]のはず

    // list_appendのテスト（リストの末尾に追加）
    int data4 = 40;
    list_append(list, &data4);

    //ここでリストの中身は[30 20 10 40]のはず

    // list_getのテスト（リストから要素を取得）
    int *retrieved_data;
    retrieved_data = (int *)list_get(list, 0);
    tm_printf("Index 0のデータ: %d\n", *retrieved_data);  // 30が表示されるはず

    retrieved_data = (int *)list_get(list, 1);
    tm_printf("Index 1のデータ: %d\n", *retrieved_data);  // 20が表示されるはず

    retrieved_data = (int *)list_get(list, 2);
    tm_printf("Index 2のデータ: %d\n", *retrieved_data);  // 10が表示されるはず

    retrieved_data = (int *)list_get(list, 3);
    tm_printf("Index 3のデータ: %d\n", *retrieved_data);  // 40が表示されるはず

    // list_shiftのテスト（リストの先頭から要素を削除）
    retrieved_data = (int *)list_shift(list);
    tm_printf("Shiftされたデータ: %d\n", *retrieved_data);  // 30が表示されるはず

    // list_lengthのテスト（リストの長さを取得）
    UH length = list_length(list);
    tm_printf("Shift後のリストの長さ: %d\n", length);  // 3が表示されるはず

    // メモリの解放
    list_delete(list);

    // maqueen の初期化
    maqueen_init();

    // センサーの状態を1秒ごとに表示
    while(1) {
        tm_printf("Sensor State: %d\n", read_line_state(MAQUEEN_LINE_SENSOR_M));
        tk_dly_tsk(1000);
    }
}
