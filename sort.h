/*
 * sort.h - 排序与筛选接口
 * 功能：提供多条件筛选、多关键字排序功能
 */

#ifndef SORT_H
#define SORT_H

#include "record.h"

/* 排序关键字结构体 */
typedef struct {
    int field;                  /* 字段类型 */
    int direction;              /* 排序方向 SORT_ASC / SORT_DESC */
} SortKey;

/* 字段类型常量 */
#define SORT_FIELD_STUDENT_ID   0
#define SORT_FIELD_NAME         1
#define SORT_FIELD_COLLEGE      2
#define SORT_FIELD_COURSE_ID    3
#define SORT_FIELD_COURSE_NAME  4
#define SORT_FIELD_CREDIT       5
#define SORT_FIELD_SEMESTER     6
#define SORT_FIELD_SCORE        7
#define SORT_FIELD_ENROLL_DATE  8

/* 筛选条件结构体 */
typedef struct {
    int field;                  /* 筛选字段 */
    char str_value[64];         /* 字符串值（用于精确/模糊匹配） */
    int int_min;                /* 整型最小值（用于区间） */
    int int_max;                /* 整型最大值（用于区间） */
    int is_fuzzy;               /* 是否模糊匹配 */
} FilterCondition;

/* 筛选结果集 */
typedef struct {
    Record *records;            /* 结果数组 */
    int count;                  /* 结果数量 */
    int capacity;               /* 容量 */
} ResultSet;

/* 结果集操作 */
void result_set_init(ResultSet *rs, int capacity);
int result_set_add(ResultSet *rs, const Record *rec);
void result_set_clear(ResultSet *rs);
void result_set_free(ResultSet *rs);

/* 筛选 */
int filter_records(const Record *records, int count, 
                   const FilterCondition *conditions, int cond_count,
                   ResultSet *result);

/* 排序 */
void sort_records(Record *records, int count, const SortKey *keys, int key_count);

/* 通用比较器 */
int compare_by_field(const Record *a, const Record *b, int field, int direction);
int compare_multi_key(const Record *a, const Record *b, const SortKey *keys, int key_count);

/* 排序算法 */
void quick_sort_records(Record *records, int low, int high, 
                        const SortKey *keys, int key_count);

#endif /* SORT_H */
