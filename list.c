/*
 * list.c - 双向链表实现
 * 功能：实现双向链表的各种操作
 */

#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 初始化链表 */
void list_init(LinkedList *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/* 创建新节点 */
static ListNode *list_create_node(const Record *rec)
{
    ListNode *node;
    
    node = (ListNode *)malloc(sizeof(ListNode));
    if (node == NULL) {
        return NULL;
    }
    
    node->data = *rec;
    node->prev = NULL;
    node->next = NULL;
    
    return node;
}

/* 插入记录（尾部插入） */
int list_insert(LinkedList *list, const Record *rec)
{
    ListNode *node;
    
    node = list_create_node(rec);
    if (node == NULL) {
        return ERR;
    }
    
    if (list->head == NULL) {
        /* 空链表 */
        list->head = node;
        list->tail = node;
    } else {
        /* 尾插 */
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    
    list->size++;
    return OK;
}

/* 删除记录（按学号+课程编号） */
int list_delete(LinkedList *list, const char *student_id, const char *course_id)
{
    ListNode *p;
    
    p = list->head;
    while (p != NULL) {
        if (strcmp(p->data.student_id, student_id) == 0 &&
            strcmp(p->data.course_id, course_id) == 0) {
            
            /* 找到节点，删除 */
            if (p->prev != NULL) {
                p->prev->next = p->next;
            } else {
                list->head = p->next;
            }
            
            if (p->next != NULL) {
                p->next->prev = p->prev;
            } else {
                list->tail = p->prev;
            }
            
            free(p);
            list->size--;
            return OK;
        }
        p = p->next;
    }
    
    return NOT_FOUND;
}

/* 更新记录 */
int list_update(LinkedList *list, const char *student_id, const char *course_id, 
                const Record *new_rec)
{
    ListNode *p;
    
    p = list_find_by_id(list, student_id, course_id);
    if (p == NULL) {
        return NOT_FOUND;
    }
    
    /* 保留学号和课程编号不变 */
    /* 更新其他字段 */
    strcpy(p->data.name, new_rec->name);
    strcpy(p->data.college, new_rec->college);
    strcpy(p->data.course_name, new_rec->course_name);
    p->data.credit = new_rec->credit;
    strcpy(p->data.semester, new_rec->semester);
    p->data.enroll_date = new_rec->enroll_date;
    p->data.score = new_rec->score;
    
    return OK;
}

/* 按学号+课程编号查找 */
ListNode *list_find_by_id(LinkedList *list, const char *student_id, const char *course_id)
{
    ListNode *p;
    
    p = list->head;
    while (p != NULL) {
        if (strcmp(p->data.student_id, student_id) == 0 &&
            strcmp(p->data.course_id, course_id) == 0) {
            return p;
        }
        p = p->next;
    }
    
    return NULL;
}

/* 按条件查找 */
ListNode *list_find(LinkedList *list, ListPredicate pred, void *arg)
{
    ListNode *p;
    
    p = list->head;
    while (p != NULL) {
        if (pred(&p->data, arg)) {
            return p;
        }
        p = p->next;
    }
    
    return NULL;
}

/* 遍历链表 */
void list_traverse(LinkedList *list, void (*visit)(const Record *))
{
    ListNode *p;
    
    p = list->head;
    while (p != NULL) {
        visit(&p->data);
        p = p->next;
    }
}

/* 获取链表大小 */
int list_size(const LinkedList *list)
{
    return list->size;
}

/* 判断链表是否为空 */
int list_is_empty(const LinkedList *list)
{
    return (list->head == NULL);
}

/* 清空链表 */
void list_clear(LinkedList *list)
{
    ListNode *p;
    ListNode *next;
    
    p = list->head;
    while (p != NULL) {
        next = p->next;
        free(p);
        p = next;
    }
    
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/* 链表排序（冒泡排序，适用于双向链表） */
int list_sort(LinkedList *list, ListComparator cmp)
{
    int swapped;
    ListNode *p;
    Record tmp;
    
    if (list->head == NULL || list->head->next == NULL) {
        return OK;  /* 空表或只有一个元素 */
    }
    
    if (cmp == NULL) {
        return ERR;  /* 比较器不能为空 */
    }
    
    do {
        swapped = 0;
        p = list->head;
        
        while (p->next != NULL) {
            if (cmp(&p->data, &p->next->data) > 0) {
                /* 交换数据 */
                tmp = p->data;
                p->data = p->next->data;
                p->next->data = tmp;
                swapped = 1;
            }
            p = p->next;
        }
    } while (swapped);
    
    return OK;
}

/* 删除过期记录 */
int list_delete_expired(LinkedList *list, int base_year, int base_month, int base_day, 
                        int *deleted_count)
{
    ListNode *p;
    ListNode *next;
    int count = 0;
    
    p = list->head;
    while (p != NULL) {
        next = p->next;
        
        if (is_expired(&p->data, base_year, base_month, base_day)) {
            /* 删除此节点 */
            if (p->prev != NULL) {
                p->prev->next = p->next;
            } else {
                list->head = p->next;
            }
            
            if (p->next != NULL) {
                p->next->prev = p->prev;
            } else {
                list->tail = p->prev;
            }
            
            free(p);
            list->size--;
            count++;
        }
        
        p = next;
    }
    
    *deleted_count = count;
    return OK;
}
