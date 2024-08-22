#include <tk/tkernel.h>

#include "list.h"

List *list_init()
{
    List *list = (List *)Kmalloc(sizeof(List));
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    return list;
}

void list_unshift(List *list, void *data)
{
    Element *new_element = (Element *)Kmalloc(sizeof(Element));
    new_element->data = data;
    new_element->next = list->head;
    list->head = new_element;

    // 要素がない場合はtailにも追加
    if(list->tail == NULL) {
        list->tail = new_element;
    }  

    list->length++;    
}

void list_append(List *list, void *data) {
    Element* new_element = (Element*)Kmalloc(sizeof(Element));
    new_element->data = data;
    new_element->next = NULL;

    // 要素がない場合はheadにも追加
    if(list->head == NULL) {
        list->head = new_element;
    } else {
        list->tail->next = new_element;
    }

    list->tail = new_element;
    list->length++;
}

void *list_shift(List *list)
{
    // リストが空の場合
    if (list->head == NULL) {
        return NULL;
    }

    Element *tmp = list->head;
    void *data = tmp->data;

    list->head = list->head->next;
    // リストが空になった場合
    if (list->head == NULL) {
        list->tail = NULL;
    }

    Kfree(tmp);
    list->length--;
    return data;
}

void *list_get(List *list, UH index)
{
    Element *current = list->head;
    for (UH i = 0; i < index && current != NULL; i++)
    {
        current = current->next;
    }

    if (current == NULL)
    {
        return NULL;
    }

    return current->data;
}

void list_delete(List *list)
{
    Element *current = list->head;
    while (current != NULL)
    {
        Element *tmp = current;
        current = current->next;
        Kfree(tmp);
    }

    Kfree(list);
}

UH list_length(List *list)
{
    return list->length;
}
