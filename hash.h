/*
 * hash.h - 哈希表数据结构
 * 功能：定义哈希表节点及操作接口
 * 说明：采用链地址法处理冲突
 */

#ifndef HASH_H
#define HASH_H

#include "record.h"

#define HASH_TABLE_SIZE 10007  /* 哈希表大小（质数） */

/* 哈希表节点（链地址法） */
typedef struct HashNode {
    Record data;                /* 数据域 */
    struct HashNode *next;      /* 链表指针 */
} HashNode;

/* 哈希表结构体 */
typedef struct {
    HashNode *buckets[HASH_TABLE_SIZE]; /* 桶数组 */
    int size;                   /* 记录数量 */
} HashTable;

/* 基本操作 */
void hash_init(HashTable *table);
int hash_insert(HashTable *table, const Record *rec);
int hash_delete(HashTable *table, const char *student_id, const char *course_id);
int hash_update(HashTable *table, const char *student_id, const char *course_id, const Record *new_rec);
HashNode *hash_find(HashTable *table, const char *student_id, const char *course_id);
void hash_traverse(HashTable *table, void (*visit)(const Record *));
int hash_size(const HashTable *table);
int hash_is_empty(const HashTable *table);
void hash_clear(HashTable *table);

/* 批量操作 */
int hash_delete_expired(HashTable *table, int base_year, int base_month, int base_day, int *deleted_count);

/* 哈希函数 */
unsigned int hash_function(const char *student_id, const char *course_id);

#endif /* HASH_H */
