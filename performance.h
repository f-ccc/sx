/*
 * performance.h - 性能测试接口
 * 功能：多结构性能对比测试
 */

#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "record.h"
#include "list.h"
#include "avl.h"
#include "hash.h"

/* 测试结果 */
typedef struct {
    int data_size;              /* 数据规模 */
    
    /* 链表测试结果 (ms) */
    double list_insert_time;
    double list_find_time;
    double list_delete_time;
    double list_traverse_time;
    double list_sort_time;
    long list_memory_bytes;
    
    /* AVL树测试结果 (ms) */
    double avl_insert_time;
    double avl_find_time;
    double avl_delete_time;
    double avl_traverse_time;
    double avl_sort_time;
    long avl_memory_bytes;
    
    /* 哈希表测试结果 (ms) */
    double hash_insert_time;
    double hash_find_time;
    double hash_delete_time;
    double hash_traverse_time;
    double hash_sort_time;
    long hash_memory_bytes;
} PerfResult;

/* 获取当前时间（毫秒） */
double get_time_ms(void);

/* 运行性能测试 */
void run_performance_test(const Record *records, int count, PerfResult *result);
void print_performance_report(const PerfResult *results, int result_count);
void print_complexity_analysis(void);

#endif /* PERFORMANCE_H */
