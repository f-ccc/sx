/*
 * list.h - 双向链表数据结构
 * 功能：定义双向链表节点及操作接口
 */

#ifndef LIST_H
#define LIST_H

#include "record.h"

/* 双向链表节点 */
typedef struct ListNode {
    Record data;                /* 数据域 */
    struct ListNode *prev;      /* 前驱指针 */
    struct ListNode *next;      /* 后继指针 */
} ListNode;

/* 双向链表结构体 */
typedef struct {
    ListNode *head;             /* 头节点 */
    ListNode *tail;             /* 尾节点 */
    int size;                   /* 节点数量 */
} LinkedList;

/* 链表迭代器回调函数类型 */
typedef int (*ListComparator)(const Record *, const Record *);
typedef int (*ListPredicate)(const Record *, void *);

/* 基本操作 */
void list_init(LinkedList *list);
int list_insert(LinkedList *list, const Record *rec);
int list_delete(LinkedList *list, const char *student_id, const char *course_id);
int list_update(LinkedList *list, const char *student_id, const char *course_id, const Record *new_rec);
ListNode *list_find_by_id(LinkedList *list, const char *student_id, const char *course_id);
ListNode *list_find(LinkedList *list, ListPredicate pred, void *arg);
void list_traverse(LinkedList *list, void (*visit)(const Record *));
int list_size(const LinkedList *list);
int list_is_empty(const LinkedList *list);
void list_clear(LinkedList *list);
int list_sort(LinkedList *list, ListComparator cmp);

/* 批量操作 */
int list_delete_expired(LinkedList *list, int base_year, int base_month, int base_day, int *deleted_count);

#endif /* LIST_H */
