/*
 * sort.c - 排序与筛选实现
 * 功能：实现多条件筛选、多关键字排序
 */

#include "sort.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== 结果集操作 ========== */

void result_set_init(ResultSet *rs, int capacity)
{
    rs->records = (Record *)malloc(sizeof(Record) * capacity);
    if (rs->records != NULL) {
        rs->count = 0;
        rs->capacity = capacity;
    } else {
        rs->capacity = 0;
        rs->count = 0;
    }
}

int result_set_add(ResultSet *rs, const Record *rec)
{
    if (rs->count >= rs->capacity) {
        /* 扩容 */
        Record *new_records;
        int new_capacity = rs->capacity * 2;
        
        new_records = (Record *)realloc(rs->records, sizeof(Record) * new_capacity);
        if (new_records == NULL) {
            return ERR;
        }
        rs->records = new_records;
        rs->capacity = new_capacity;
    }
    
    rs->records[rs->count] = *rec;
    rs->count++;
    return OK;
}

void result_set_clear(ResultSet *rs)
{
    rs->count = 0;
}

void result_set_free(ResultSet *rs)
{
    if (rs->records != NULL) {
        free(rs->records);
        rs->records = NULL;
    }
    rs->count = 0;
    rs->capacity = 0;
}

/* ========== 字符串匹配 ========== */

/* 简单模糊匹配（判断substr是否为str的子串） */
static int fuzzy_match(const char *str, const char *substr)
{
    int i, j;
    
    if (substr[0] == '\0') {
        return 1;  /* 空字符串匹配所有 */
    }
    
    for (i = 0; str[i] != '\0'; i++) {
        int match = 1;
        for (j = 0; substr[j] != '\0'; j++) {
            if (str[i + j] == '\0' || str[i + j] != substr[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            return 1;
        }
    }
    
    return 0;
}

/* ========== 筛选 ========== */

/* 检查一条记录是否满足所有筛选条件 */
static int record_matches_all(const Record *rec, const FilterCondition *conditions, 
                              int cond_count)
{
    int i;
    
    for (i = 0; i < cond_count; i++) {
        switch (conditions[i].field) {
            case FILTER_COURSE_NAME:
                if (conditions[i].is_fuzzy) {
                    if (!fuzzy_match(rec->course_name, conditions[i].str_value)) {
                        return 0;
                    }
                } else {
                    if (strcmp(rec->course_name, conditions[i].str_value) != 0) {
                        return 0;
                    }
                }
                break;
                
            case FILTER_SEMESTER:
                if (strcmp(rec->semester, conditions[i].str_value) != 0) {
                    return 0;
                }
                break;
                
            case FILTER_SCORE_RANGE:
                if (rec->score < conditions[i].int_min || 
                    rec->score > conditions[i].int_max) {
                    return 0;
                }
                break;
                
            case FILTER_COLLEGE:
                if (conditions[i].is_fuzzy) {
                    if (!fuzzy_match(rec->college, conditions[i].str_value)) {
                        return 0;
                    }
                } else {
                    if (strcmp(rec->college, conditions[i].str_value) != 0) {
                        return 0;
                    }
                }
                break;
                
            case FILTER_STUDENT_ID:
                if (!fuzzy_match(rec->student_id, conditions[i].str_value)) {
                    return 0;
                }
                break;
                
            case FILTER_COURSE_ID:
                if (!fuzzy_match(rec->course_id, conditions[i].str_value)) {
                    return 0;
                }
                break;
        }
    }
    
    return 1;
}

/* 筛选记录 */
int filter_records(const Record *records, int count,
                   const FilterCondition *conditions, int cond_count,
                   ResultSet *result)
{
    int i;
    
    result_set_init(result, count > 0 ? count : 10);
    if (result->records == NULL) {
        return ERR;
    }
    
    for (i = 0; i < count; i++) {
        if (record_matches_all(&records[i], conditions, cond_count)) {
            if (result_set_add(result, &records[i]) != OK) {
                result_set_free(result);
                return ERR;
            }
        }
    }
    
    return OK;
}

/* ========== 字段比较 ========== */

/* 按指定字段比较两条记录 */
int compare_by_field(const Record *a, const Record *b, int field, int direction)
{
    int cmp = 0;
    
    switch (field) {
        case SORT_FIELD_STUDENT_ID:
            cmp = strcmp(a->student_id, b->student_id);
            break;
        case SORT_FIELD_NAME:
            cmp = strcmp(a->name, b->name);
            break;
        case SORT_FIELD_COLLEGE:
            cmp = strcmp(a->college, b->college);
            break;
        case SORT_FIELD_COURSE_ID:
            cmp = strcmp(a->course_id, b->course_id);
            break;
        case SORT_FIELD_COURSE_NAME:
            cmp = strcmp(a->course_name, b->course_name);
            break;
        case SORT_FIELD_CREDIT:
            if (a->credit < b->credit) cmp = -1;
            else if (a->credit > b->credit) cmp = 1;
            else cmp = 0;
            break;
        case SORT_FIELD_SEMESTER:
            cmp = strcmp(a->semester, b->semester);
            break;
        case SORT_FIELD_SCORE:
            if (a->score < b->score) cmp = -1;
            else if (a->score > b->score) cmp = 1;
            else cmp = 0;
            break;
        case SORT_FIELD_ENROLL_DATE:
            cmp = date_compare(&a->enroll_date, &b->enroll_date);
            break;
        default:
            cmp = 0;
            break;
    }
    
    /* 降序反转 */
    if (direction == SORT_DESC) {
        cmp = -cmp;
    }
    
    return cmp;
}

/* 多关键字比较 */
int compare_multi_key(const Record *a, const Record *b, const SortKey *keys, int key_count)
{
    int i;
    int cmp;
    
    for (i = 0; i < key_count; i++) {
        cmp = compare_by_field(a, b, keys[i].field, keys[i].direction);
        if (cmp != 0) {
            return cmp;
        }
    }
    
    return 0;
}

/* ========== 快速排序 ========== */

/* 数组分区 */
/* 旧 partition 函数已被 partition_opt + median_of_three 替代 */

/* 三数取中：选low、mid、high的中值作为pivot */
/* 将中值交换到high位置，返回pivot值 */
static void median_of_three(Record *records, int low, int high,
                             const SortKey *keys, int key_count, Record *pivot)
{
    int mid = low + (high - low) / 2;
    Record temp;
    
    /* 比较 low, mid, high，把中值放到 high */
    if (compare_multi_key(&records[low], &records[mid], keys, key_count) > 0) {
        temp = records[low]; records[low] = records[mid]; records[mid] = temp;
    }
    if (compare_multi_key(&records[low], &records[high], keys, key_count) > 0) {
        temp = records[low]; records[low] = records[high]; records[high] = temp;
    }
    if (compare_multi_key(&records[mid], &records[high], keys, key_count) > 0) {
        temp = records[mid]; records[mid] = records[high]; records[high] = temp;
    }
    /* 现在 high 位置是中值 */
    *pivot = records[high];
}

/* 数组分区（标准Lomuto分区，pivot在high位置） */
static int partition_opt(Record *records, int low, int high,
                          const SortKey *keys, int key_count)
{
    Record pivot;
    int i, j;
    Record temp;
    
    if (high - low > 1) {
        median_of_three(records, low, high, keys, key_count, &pivot);
    } else {
        pivot = records[high];
    }
    
    i = low - 1;
    for (j = low; j < high; j++) {
        if (compare_multi_key(&records[j], &pivot, keys, key_count) <= 0) {
            i++;
            temp = records[i]; records[i] = records[j]; records[j] = temp;
        }
    }
    temp = records[i + 1]; records[i + 1] = records[high]; records[high] = temp;
    
    return i + 1;
}

/* 
 * 快速排序（尾递归优化版）
 * 每次递归处理较小的分区，较大的分区用循环处理
 * 保证最坏情况下递归深度为 O(log n)
 */
void quick_sort_records(Record *records, int low, int high,
                        const SortKey *keys, int key_count)
{
    int pi;
    
    while (low < high) {
        pi = partition_opt(records, low, high, keys, key_count);
        
        /* 尾递归优化：只递归处理较小的分区 */
        if (pi - low < high - pi) {
            quick_sort_records(records, low, pi - 1, keys, key_count);
            low = pi + 1;  /* 循环处理较大的右分区 */
        } else {
            quick_sort_records(records, pi + 1, high, keys, key_count);
            high = pi - 1;  /* 循环处理较大的左分区 */
        }
    }
}

/* 排序入口 */
void sort_records(Record *records, int count, const SortKey *keys, int key_count)
{
    if (count <= 1 || keys == NULL || key_count <= 0) {
        return;
    }
    
    quick_sort_records(records, 0, count - 1, keys, key_count);
}
