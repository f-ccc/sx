/*
 * hash.c - 哈希表实现
 * 功能：实现哈希表的链地址法操作
 */

#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 哈希函数：使用学号和课程编号计算哈希值 */
unsigned int hash_function(const char *student_id, const char *course_id)
{
    unsigned int hash = 0;
    int i;
    
    /* 使用BKDR哈希算法 */
    for (i = 0; student_id[i] != '\0'; i++) {
        hash = hash * 131 + (unsigned char)student_id[i];
    }
    for (i = 0; course_id[i] != '\0'; i++) {
        hash = hash * 131 + (unsigned char)course_id[i];
    }
    
    return hash % HASH_TABLE_SIZE;
}

/* 初始化哈希表 */
void hash_init(HashTable *table)
{
    int i;
    
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    table->size = 0;
}

/* 创建哈希节点 */
static HashNode *hash_create_node(const Record *rec)
{
    HashNode *node;
    
    node = (HashNode *)malloc(sizeof(HashNode));
    if (node == NULL) {
        return NULL;
    }
    
    node->data = *rec;
    node->next = NULL;
    
    return node;
}

/* 插入记录 */
int hash_insert(HashTable *table, const Record *rec)
{
    unsigned int idx;
    HashNode *node;
    HashNode *p;
    
    idx = hash_function(rec->student_id, rec->course_id);
    
    /* 检查是否已存在 */
    p = table->buckets[idx];
    while (p != NULL) {
        if (strcmp(p->data.student_id, rec->student_id) == 0 &&
            strcmp(p->data.course_id, rec->course_id) == 0) {
            return DUPLICATE;
        }
        p = p->next;
    }
    
    /* 创建新节点，头插 */
    node = hash_create_node(rec);
    if (node == NULL) {
        return ERR;
    }
    
    node->next = table->buckets[idx];
    table->buckets[idx] = node;
    table->size++;
    
    return OK;
}

/* 删除记录 */
int hash_delete(HashTable *table, const char *student_id, const char *course_id)
{
    unsigned int idx;
    HashNode *p;
    HashNode *prev;
    
    idx = hash_function(student_id, course_id);
    
    p = table->buckets[idx];
    prev = NULL;
    
    while (p != NULL) {
        if (strcmp(p->data.student_id, student_id) == 0 &&
            strcmp(p->data.course_id, course_id) == 0) {
            
            if (prev == NULL) {
                table->buckets[idx] = p->next;
            } else {
                prev->next = p->next;
            }
            
            free(p);
            table->size--;
            return OK;
        }
        prev = p;
        p = p->next;
    }
    
    return NOT_FOUND;
}

/* 更新记录 */
int hash_update(HashTable *table, const char *student_id, const char *course_id, 
                const Record *new_rec)
{
    HashNode *node;
    
    node = hash_find(table, student_id, course_id);
    if (node == NULL) {
        return NOT_FOUND;
    }
    
    /* 保留学号和课程编号，更新其他字段 */
    strcpy(node->data.name, new_rec->name);
    strcpy(node->data.college, new_rec->college);
    strcpy(node->data.course_name, new_rec->course_name);
    node->data.credit = new_rec->credit;
    strcpy(node->data.semester, new_rec->semester);
    node->data.enroll_date = new_rec->enroll_date;
    node->data.score = new_rec->score;
    
    return OK;
}

/* 查找记录 */
HashNode *hash_find(HashTable *table, const char *student_id, const char *course_id)
{
    unsigned int idx;
    HashNode *p;
    
    idx = hash_function(student_id, course_id);
    
    p = table->buckets[idx];
    while (p != NULL) {
        if (strcmp(p->data.student_id, student_id) == 0 &&
            strcmp(p->data.course_id, course_id) == 0) {
            return p;
        }
        p = p->next;
    }
    
    return NULL;
}

/* 遍历哈希表 */
void hash_traverse(HashTable *table, void (*visit)(const Record *))
{
    int i;
    HashNode *p;
    
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        p = table->buckets[i];
        while (p != NULL) {
            visit(&p->data);
            p = p->next;
        }
    }
}

/* 获取哈希表大小 */
int hash_size(const HashTable *table)
{
    return table->size;
}

/* 判断哈希表是否为空 */
int hash_is_empty(const HashTable *table)
{
    return (table->size == 0);
}

/* 清空哈希表 */
void hash_clear(HashTable *table)
{
    int i;
    HashNode *p;
    HashNode *next;
    
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        p = table->buckets[i];
        while (p != NULL) {
            next = p->next;
            free(p);
            p = next;
        }
        table->buckets[i] = NULL;
    }
    
    table->size = 0;
}

/* 删除过期记录 */
int hash_delete_expired(HashTable *table, int base_year, int base_month, int base_day,
                        int *deleted_count)
{
    int i;
    HashNode *p;
    HashNode *prev;
    HashNode *next;
    int count = 0;
    
    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        p = table->buckets[i];
        prev = NULL;
        
        while (p != NULL) {
            next = p->next;
            
            if (is_expired(&p->data, base_year, base_month, base_day)) {
                /* 删除节点 */
                if (prev == NULL) {
                    table->buckets[i] = p->next;
                } else {
                    prev->next = p->next;
                }
                
                free(p);
                count++;
                table->size--;
            } else {
                prev = p;
            }
            
            p = next;
        }
    }
    
    *deleted_count = count;
    return OK;
}
