#ifndef LIST_H_
#define LIST_H_

#include <tk/tkernel.h>

/*
 * Element 構造体
 */
typedef struct Element
{
    void *data;
    struct Element *next;
} Element;

/*
 * List 構造体
 */
typedef struct List
{
    Element *head;
    Element *tail;
    UH length;
} List;

/*
 * List 初期化関数
 */
EXPORT List *list_init();

/*
 * リストの最初に要素を挿入
 */
EXPORT void list_unshift(List *list, void *data);

/*
 * リストの最後に要素を挿入
 */
EXPORT void list_append(List *list, void *data);

/*
 * 最初の要素を取得して削除
 */
EXPORT void *list_shift(List *list);

/*
 * 任意のインデックスの要素を取得する関数
 */
EXPORT void *list_get(List *list, UH index);

/*
 * リストを削除する関数
 */
EXPORT void list_delete(List *list);

/*
 * リストの長さを取得する関数
 */
EXPORT UH list_length(List *list);

/*
 * リストの最後の要素を取得
 */
EXPORT void *list_get_last(List *list);

#endif /* LIST_H_ */
