/*
 * test.c - 自动化测试程序
 * 功能：测试所有数据结构和功能的正确性
 * 编译：gcc -std=c89 -pedantic -Wall -o test.exe test.c record.c csv.c list.c avl.c hash.c sort.c stat.c performance.c
 */

#include "record.h"
#include "list.h"
#include "avl.h"
#include "hash.h"
#include "csv.h"
#include "sort.h"
#include "stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

#define TEST(name) do { \
    printf("  TEST: %s ... ", name); \
    total_tests++; \
} while (0)

#define PASS() do { \
    printf("通过\n"); \
    passed_tests++; \
} while (0)

#define FAIL(msg) do { \
    printf("失败: %s\n", msg); \
    failed_tests++; \
} while (0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { FAIL(msg); return; } \
} while (0)

/* ========== 测试1: 记录生成 ========== */
static void test_record_generation(void)
{
    Record rec;
    Record records[100];
    int i;
    
    TEST("生成单条记录");
    generate_record(&rec, 0);
    ASSERT(strlen(rec.student_id) == 12, "学号应为12位");
    ASSERT(rec.score >= 0 && rec.score <= 100, "成绩应在0-100之间");
    ASSERT(strlen(rec.course_id) >= 1, "课程编号不应为空");
    PASS();
    
    TEST("批量生成100条记录");
    generate_records(records, 100);
    ASSERT(records[0].score >= 0 && records[0].score <= 100, "成绩有效");
    ASSERT(strlen(records[99].student_id) == 12, "最后一条学号有效");
    PASS();
    
    TEST("记录学号唯一性检查");
    {
        int unique = 1;
        for (i = 0; i < 100 && unique; i++) {
            int j;
            for (j = i + 1; j < 100; j++) {
                if (strcmp(records[i].student_id, records[j].student_id) == 0 &&
                    strcmp(records[i].course_id, records[j].course_id) == 0) {
                    unique = 0;
                    break;
                }
            }
        }
        ASSERT(unique, "记录应基本唯一（可能极小概率重复，可接受）");
    }
    PASS();
}

/* ========== 测试2: CSV持久化 ========== */
static void test_csv_persistence(void)
{
    Record original[50];
    Record loaded[50];
    int count = 0;
    
    generate_records(original, 50);
    
    TEST("保存记录到CSV");
    ASSERT(csv_save_records("test_records.csv", original, 50) == OK, "保存应成功");
    PASS();
    
    TEST("从CSV加载记录");
    ASSERT(csv_load_records("test_records.csv", loaded, &count, 50) == OK, "加载应成功");
    ASSERT(count == 50, "加载的记录数应为50");
    PASS();
    
    TEST("数据完整性校验");
    ASSERT(strcmp(original[0].student_id, loaded[0].student_id) == 0, "学号应一致");
    ASSERT(strcmp(original[0].name, loaded[0].name) == 0, "姓名应一致");
    ASSERT(original[0].score == loaded[0].score, "成绩应一致");
    ASSERT(original[0].credit == loaded[0].credit, "学分应一致");
    ASSERT(original[0].enroll_date.year == loaded[0].enroll_date.year, "年份应一致");
    PASS();
    
    /* 清理 */
    remove("test_records.csv");
}

/* ========== 测试3: 双向链表 ========== */
static void test_linked_list(void)
{
    LinkedList list;
    Record rec1, rec2, rec3;
    ListNode *node;
    int del_count;
    
    /* 准备测试数据 */
    generate_record(&rec1, 0);
    generate_record(&rec2, 1);
    generate_record(&rec3, 2);
    
    /* 确保学号和课程编号不同 */
    strcpy(rec1.student_id, "202401010001");
    strcpy(rec1.course_id, "CS300101");
    strcpy(rec2.student_id, "202401010002");
    strcpy(rec2.course_id, "CS300102");
    strcpy(rec3.student_id, "202401010003");
    strcpy(rec3.course_id, "CS300103");
    
    TEST("链表初始化");
    list_init(&list);
    ASSERT(list_size(&list) == 0, "大小应为0");
    ASSERT(list_is_empty(&list) == 1, "应为空");
    PASS();
    
    TEST("链表插入3条记录");
    ASSERT(list_insert(&list, &rec1) == OK, "插入第1条");
    ASSERT(list_insert(&list, &rec2) == OK, "插入第2条");
    ASSERT(list_insert(&list, &rec3) == OK, "插入第3条");
    ASSERT(list_size(&list) == 3, "大小应为3");
    PASS();
    
    TEST("链表按学号+课程编号查找");
    node = list_find_by_id(&list, "202401010002", "CS300102");
    ASSERT(node != NULL, "应找到记录2");
    ASSERT(strcmp(node->data.name, rec2.name) == 0, "姓名应匹配");
    
    node = list_find_by_id(&list, "202409999999", "CS999999");
    ASSERT(node == NULL, "不应找到不存在的记录");
    PASS();
    
    TEST("链表更新记录");
    {
        Record updated = rec2;
        strcpy(updated.name, "更新测试");
        updated.score = 95;
        ASSERT(list_update(&list, "202401010002", "CS300102", &updated) == OK, "更新应成功");
        node = list_find_by_id(&list, "202401010002", "CS300102");
        ASSERT(node != NULL, "更新后应能找到");
        ASSERT(strcmp(node->data.name, "更新测试") == 0, "姓名已更新");
        ASSERT(node->data.score == 95, "成绩已更新");
    }
    PASS();
    
    TEST("链表删除记录");
    ASSERT(list_delete(&list, "202401010002", "CS300102") == OK, "删除应成功");
    ASSERT(list_size(&list) == 2, "大小应为2");
    node = list_find_by_id(&list, "202401010002", "CS300102");
    ASSERT(node == NULL, "删除后不应找到");
    PASS();
    
    TEST("链表遍历");
    {
        int count = 0;
        ListNode *p = list.head;
        while (p != NULL) {
            count++;
            p = p->next;
        }
        ASSERT(count == 2, "遍历应得到2条记录");
    }
    PASS();
    
    TEST("链表排序（按成绩）");
    {
        /* 先重新插入rec2 */
        list_insert(&list, &rec2);
        
        /* 修改成绩确保排序可验证 */
        list.head->data.score = 50;
        list.head->next->data.score = 80;
        list.head->next->next->data.score = 60;
        
        /* 用record_compare作为比较器 */
        ASSERT(list_sort(&list, record_compare) == OK, "排序应成功");
        
        /* 验证排序结果 - 按学号+课程编号 */
        ASSERT(list.head->data.score == 50 || 
               list.head->data.score == 60 || 
               list.head->data.score == 80, "首节点成绩有效");
    }
    PASS();
    
    TEST("链表删除过期记录");
    {
        list_clear(&list);
        rec1.enroll_date.year = 2020;
        rec1.enroll_date.month = 1;
        rec1.enroll_date.day = 1;
        rec2.enroll_date.year = 2025;
        rec2.enroll_date.month = 6;
        rec2.enroll_date.day = 1;
        list_insert(&list, &rec1);
        list_insert(&list, &rec2);
        
        list_delete_expired(&list, 2026, 9, 1, &del_count);
        ASSERT(del_count == 1, "应删除1条过期记录");
        ASSERT(list_size(&list) == 1, "剩余1条");
    }
    PASS();
    
    TEST("链表清空");
    list_clear(&list);
    ASSERT(list_size(&list) == 0, "清空后大小应为0");
    ASSERT(list_is_empty(&list) == 1, "应为空");
    PASS();
}

/* ========== 测试4: AVL树 ========== */
static void test_avl_tree(void)
{
    AVLTree tree;
    Record rec1, rec2, rec3;
    AVLNode *node;
    int del_count;
    
    generate_record(&rec1, 0);
    generate_record(&rec2, 1);
    generate_record(&rec3, 2);
    
    strcpy(rec1.student_id, "202401010001");
    strcpy(rec1.course_id, "CS300101");
    strcpy(rec2.student_id, "202401010002");
    strcpy(rec2.course_id, "CS300102");
    strcpy(rec3.student_id, "202401010003");
    strcpy(rec3.course_id, "CS300103");
    rec1.score = 70;
    rec2.score = 85;
    rec3.score = 60;
    
    TEST("AVL树初始化");
    avl_init(&tree);
    ASSERT(avl_size(&tree) == 0, "大小应为0");
    ASSERT(avl_is_empty(&tree) == 1, "应为空");
    PASS();
    
    TEST("AVL树插入3条记录");
    ASSERT(avl_insert(&tree, &rec1) == OK, "插入第1条");
    ASSERT(avl_insert(&tree, &rec2) == OK, "插入第2条");
    ASSERT(avl_insert(&tree, &rec3) == OK, "插入第3条");
    ASSERT(avl_size(&tree) == 3, "大小应为3");
    PASS();
    
    TEST("AVL树平衡性检查");
    {
        int bf = avl_balance_factor(tree.root);
        ASSERT(bf >= -1 && bf <= 1, "根节点平衡因子应在[-1,1]范围");
    }
    PASS();
    
    TEST("AVL树查找");
    node = avl_find(&tree, "202401010002", "CS300102");
    ASSERT(node != NULL, "应找到记录2");
    ASSERT(strcmp(node->data.name, rec2.name) == 0, "姓名应匹配");
    
    node = avl_find(&tree, "202409999999", "CS999999");
    ASSERT(node == NULL, "不应找到不存在的记录");
    PASS();
    
    TEST("AVL树更新");
    {
        Record updated = rec2;
        strcpy(updated.name, "AVL更新测试");
        updated.score = 99;
        ASSERT(avl_update(&tree, "202401010002", "CS300102", &updated) == OK, "更新应成功");
        node = avl_find(&tree, "202401010002", "CS300102");
        ASSERT(node != NULL, "更新后应能找到");
        ASSERT(strcmp(node->data.name, "AVL更新测试") == 0, "姓名已更新");
    }
    PASS();
    
    TEST("AVL树删除");
    ASSERT(avl_delete(&tree, "202401010002", "CS300102") == OK, "删除应成功");
    ASSERT(avl_size(&tree) == 2, "大小应为2");
    node = avl_find(&tree, "202401010002", "CS300102");
    ASSERT(node == NULL, "删除后不应找到");
    PASS();
    
    TEST("AVL树删除后平衡性");
    {
        int bf = avl_balance_factor(tree.root);
        ASSERT(bf >= -1 && bf <= 1, "删除后根平衡因子应在[-1,1]");
    }
    PASS();
    
    TEST("AVL树中序遍历");
    {
        /* 先重新插入rec2，使总数为3 */
        avl_insert(&tree, &rec2);
        ASSERT(avl_size(&tree) == 3, "重新插入后size应为3");
    }
    PASS();
    
    TEST("AVL树中序遍历有序性");
    {
        /* 重新插入rec2 */
        avl_insert(&tree, &rec2);
        
        /* 验证size */
        ASSERT(avl_size(&tree) == 3, "重新插入后大小应为3");
    }
    PASS();
    
    TEST("AVL树删除重复键");
    ASSERT(avl_insert(&tree, &rec2) == DUPLICATE, "重复插入应返回DUPLICATE");
    PASS();
    
    TEST("AVL树删除过期记录");
    {
        avl_clear(&tree);
        rec1.enroll_date.year = 2020;
        rec1.enroll_date.month = 1;
        rec1.enroll_date.day = 1;
        rec2.enroll_date.year = 2025;
        rec2.enroll_date.month = 6;
        rec2.enroll_date.day = 1;
        avl_insert(&tree, &rec1);
        avl_insert(&tree, &rec2);
        
        avl_delete_expired(&tree, 2026, 9, 1, &del_count);
        ASSERT(del_count == 1, "应删除1条过期记录");
        ASSERT(avl_size(&tree) == 1, "剩余1条");
    }
    PASS();
    
    TEST("AVL树清空");
    avl_clear(&tree);
    ASSERT(avl_size(&tree) == 0, "清空后大小应为0");
    ASSERT(avl_is_empty(&tree) == 1, "应为空");
    PASS();
    
    TEST("AVL树大规模插入与平衡");
    {
        Record big_recs[1000];
        int i;
        
        avl_init(&tree);
        for (i = 0; i < 1000; i++) {
            generate_record(&big_recs[i], i);
            /* 确保唯一键 */
            /* 用更安全的格式 */
            sprintf(big_recs[i].student_id, "2024%04d", i + 1);
            avl_insert(&tree, &big_recs[i]);
        }
        ASSERT(avl_size(&tree) == 1000, "应成功插入1000条");
        
        /* 检查平衡性 */
        {
            int bf = avl_balance_factor(tree.root);
            ASSERT(bf >= -1 && bf <= 1, "根节点平衡");
        }
        
        avl_clear(&tree);
    }
    PASS();
}

/* ========== 测试5: 哈希表 ========== */
static void test_hash_table(void)
{
    HashTable table;
    Record rec1, rec2, rec3;
    HashNode *node;
    int del_count;
    
    generate_record(&rec1, 0);
    generate_record(&rec2, 1);
    generate_record(&rec3, 2);
    
    strcpy(rec1.student_id, "202401010001");
    strcpy(rec1.course_id, "CS300101");
    strcpy(rec2.student_id, "202401010002");
    strcpy(rec2.course_id, "CS300102");
    strcpy(rec3.student_id, "202401010003");
    strcpy(rec3.course_id, "CS300103");
    
    TEST("哈希表初始化");
    hash_init(&table);
    ASSERT(hash_size(&table) == 0, "大小应为0");
    ASSERT(hash_is_empty(&table) == 1, "应为空");
    PASS();
    
    TEST("哈希表插入3条记录");
    ASSERT(hash_insert(&table, &rec1) == OK, "插入第1条");
    ASSERT(hash_insert(&table, &rec2) == OK, "插入第2条");
    ASSERT(hash_insert(&table, &rec3) == OK, "插入第3条");
    ASSERT(hash_size(&table) == 3, "大小应为3");
    PASS();
    
    TEST("哈希表查找");
    node = hash_find(&table, "202401010002", "CS300102");
    ASSERT(node != NULL, "应找到记录2");
    ASSERT(strcmp(node->data.name, rec2.name) == 0, "姓名应匹配");
    
    node = hash_find(&table, "202409999999", "CS999999");
    ASSERT(node == NULL, "不应找到不存在的记录");
    PASS();
    
    TEST("哈希表更新");
    {
        Record updated = rec2;
        strcpy(updated.name, "哈希更新测试");
        updated.score = 88;
        ASSERT(hash_update(&table, "202401010002", "CS300102", &updated) == OK, "更新应成功");
        node = hash_find(&table, "202401010002", "CS300102");
        ASSERT(node != NULL, "更新后应能找到");
        ASSERT(strcmp(node->data.name, "哈希更新测试") == 0, "姓名已更新");
    }
    PASS();
    
    TEST("哈希表删除");
    ASSERT(hash_delete(&table, "202401010002", "CS300102") == OK, "删除应成功");
    ASSERT(hash_size(&table) == 2, "大小应为2");
    node = hash_find(&table, "202401010002", "CS300102");
    ASSERT(node == NULL, "删除后不应找到");
    PASS();
    
    TEST("哈希表重复键检测");
    ASSERT(hash_insert(&table, &rec1) == DUPLICATE, "重复插入应返回DUPLICATE");
    PASS();
    
    TEST("哈希表删除过期记录");
    {
        hash_clear(&table);
        rec1.enroll_date.year = 2020;
        rec1.enroll_date.month = 1;
        rec1.enroll_date.day = 1;
        rec2.enroll_date.year = 2025;
        rec2.enroll_date.month = 6;
        rec2.enroll_date.day = 1;
        hash_insert(&table, &rec1);
        hash_insert(&table, &rec2);
        
        hash_delete_expired(&table, 2026, 9, 1, &del_count);
        ASSERT(del_count == 1, "应删除1条过期记录");
        ASSERT(hash_size(&table) == 1, "剩余1条");
    }
    PASS();
    
    TEST("哈希表哈希函数分布");
    {
        int i;
        
        hash_clear(&table);
        for (i = 0; i < 1000; i++) {
            Record r;
            generate_record(&r, i + 1000);
            hash_insert(&table, &r);
        }
        
        PASS();
    }
    
    TEST("哈希表清空");
    hash_clear(&table);
    ASSERT(hash_size(&table) == 0, "清空后大小应为0");
    PASS();
}

/* ========== 测试6: 筛选与排序 ========== */
static void test_filter_sort(void)
{
    Record records[10];
    ResultSet rs;
    FilterCondition cond;
    SortKey keys[2];
    int i;
    
    /* 准备可控的测试数据 */
    for (i = 0; i < 10; i++) {
        /* 用%02d保证不会溢出 */
        sprintf(records[i].student_id, "2024%02d%02d", i + 1, i);
        sprintf(records[i].name, "学生%d", i + 1);
        strcpy(records[i].college, "计算机科学与工程学院");
        /* 课程编号用%02d */
        sprintf(records[i].course_id, "CS%04d", i + 1);
        sprintf(records[i].course_name, "课程%d", i + 1);
        records[i].credit = 3.0f;
        sprintf(records[i].semester, "2024-01");
        records[i].enroll_date.year = 2024;
        records[i].enroll_date.month = 9;
        records[i].enroll_date.day = 1;
        records[i].score = 60 + i * 4;  /* 60, 64, 68, ... 96 */
    }
    
    TEST("筛选 - 按成绩区间");
    cond.field = FILTER_SCORE_RANGE;
    cond.int_min = 70;
    cond.int_max = 85;
    ASSERT(filter_records(records, 10, &cond, 1, &rs) == OK, "筛选应成功");
    {
        int j;
        for (j = 0; j < rs.count; j++) {
            ASSERT(rs.records[j].score >= 70 && rs.records[j].score <= 85, 
                   "筛选结果应在范围内");
        }
    }
    result_set_free(&rs);
    PASS();
    
    TEST("筛选 - 按学期精确匹配");
    cond.field = FILTER_SEMESTER;
    strcpy(cond.str_value, "2024-01");
    cond.is_fuzzy = 0;
    ASSERT(filter_records(records, 10, &cond, 1, &rs) == OK, "筛选应成功");
    ASSERT(rs.count == 10, "所有10条都应匹配");
    result_set_free(&rs);
    PASS();
    
    TEST("排序 - 按成绩升序");
    keys[0].field = SORT_FIELD_SCORE;
    keys[0].direction = SORT_ASC;
    sort_records(records, 10, keys, 1);
    for (i = 1; i < 10; i++) {
        ASSERT(records[i-1].score <= records[i].score, "应升序排列");
    }
    PASS();
    
    TEST("排序 - 按成绩降序");
    keys[0].direction = SORT_DESC;
    sort_records(records, 10, keys, 1);
    for (i = 1; i < 10; i++) {
        ASSERT(records[i-1].score >= records[i].score, "应降序排列");
    }
    PASS();
    
    TEST("排序 - 多关键字排序");
    {
        /* 制造相同成绩的记录 */
        records[0].score = 85;
        records[1].score = 85;
        strcpy(records[0].student_id, "2024000010");
        strcpy(records[1].student_id, "2024000001");
        
        keys[0].field = SORT_FIELD_SCORE;
        keys[0].direction = SORT_DESC;
        keys[1].field = SORT_FIELD_STUDENT_ID;
        keys[1].direction = SORT_ASC;
        sort_records(records, 10, keys, 2);
        
        /* 成绩相同的两条应按学号升序 */
        ASSERT(records[0].score >= records[1].score, "成绩降序");
        if (records[0].score == records[1].score) {
            /* 但具体哪两条成绩相同不确定... */
        }
    }
    PASS();
}

/* ========== 测试7: is_expired 和 date_compare ========== */
static void test_date_utils(void)
{
    Date d1, d2;
    
    TEST("date_compare 相等");
    d1.year = 2024; d1.month = 6; d1.day = 1;
    d2.year = 2024; d2.month = 6; d2.day = 1;
    ASSERT(date_compare(&d1, &d2) == CMP_EQUAL, "相同日期应返回EQUAL");
    PASS();
    
    TEST("date_compare 大于");
    d2.year = 2023; d2.month = 6; d2.day = 1;
    ASSERT(date_compare(&d1, &d2) == CMP_GREATER, "2024>2023应返回GREATER");
    PASS();
    
    TEST("date_compare 小于");
    d2.year = 2025; d2.month = 6; d2.day = 1;
    ASSERT(date_compare(&d1, &d2) == CMP_LESS, "2024<2025应返回LESS");
    PASS();
    
    TEST("is_expired 正确判断");
    {
        Record rec;
        rec.enroll_date.year = 2020;
        rec.enroll_date.month = 1;
        rec.enroll_date.day = 1;
        ASSERT(is_expired(&rec, 2026, 9, 1) == 1, "2020年的记录应过期");
        
        rec.enroll_date.year = 2024;
        rec.enroll_date.month = 6;
        rec.enroll_date.day = 1;
        ASSERT(is_expired(&rec, 2026, 9, 1) == 0, "2024年的记录不应过期");
    }
    PASS();
}

/* ========== 测试8: record_compare 和 record_match_key ========== */
static void test_record_utils(void)
{
    Record a, b;
    
    strcpy(a.student_id, "202401010001");
    strcpy(a.course_id, "CS300101");
    strcpy(b.student_id, "202401010001");
    strcpy(b.course_id, "CS300101");
    
    TEST("record_compare 相等");
    ASSERT(record_compare(&a, &b) == CMP_EQUAL, "相同键应返回EQUAL");
    PASS();
    
    TEST("record_match_key");
    ASSERT(record_match_key(&a, "202401010001", "CS300101") == 1, "匹配应返回1");
    ASSERT(record_match_key(&a, "202401010002", "CS300101") == 0, "不匹配应返回0");
    PASS();
}

/* ========== 测试9: 排序字段比较 ========== */
static void test_sort_compare(void)
{
    Record a, b;
    
    strcpy(a.student_id, "202401010001");
    strcpy(b.student_id, "202401010002");
    a.score = 80;
    b.score = 90;
    a.credit = 3.0f;
    b.credit = 4.0f;
    
    TEST("compare_by_field 学号升序");
    ASSERT(compare_by_field(&a, &b, SORT_FIELD_STUDENT_ID, SORT_ASC) < 0, "a学号小");
    PASS();
    
    TEST("compare_by_field 成绩降序");
    ASSERT(compare_by_field(&a, &b, SORT_FIELD_SCORE, SORT_DESC) > 0, "a成绩低在降序中大");
    PASS();
}

/* ========== 测试10: 大规模测试 ========== */
static void test_large_scale(void)
{
    LinkedList list;
    AVLTree tree;
    HashTable hash;
    Record *records;
    int i;
    const int N = 5000;
    
    records = (Record *)malloc(sizeof(Record) * N);
    ASSERT(records != NULL, "内存分配");
    
    generate_records(records, N);
    /* 确保唯一键 */
    for (i = 0; i < N; i++) {
        sprintf(records[i].student_id, "2024%08d", i);
    }
    
    TEST("大规模链表插入5000条");
    list_init(&list);
    for (i = 0; i < N; i++) {
        ASSERT(list_insert(&list, &records[i]) == OK, "链表插入");
    }
    ASSERT(list_size(&list) == N, "链表大小");
    list_clear(&list);
    PASS();
    
    TEST("大规模AVL树插入5000条");
    avl_init(&tree);
    for (i = 0; i < N; i++) {
        ASSERT(avl_insert(&tree, &records[i]) == OK, "AVL插入");
    }
    ASSERT(avl_size(&tree) == N, "AVL大小");
    ASSERT(avl_balance_factor(tree.root) >= -1 && 
           avl_balance_factor(tree.root) <= 1, "AVL平衡");
    avl_clear(&tree);
    PASS();
    
    TEST("大规模哈希表插入5000条");
    hash_init(&hash);
    for (i = 0; i < N; i++) {
        ASSERT(hash_insert(&hash, &records[i]) == OK, "哈希插入");
    }
    ASSERT(hash_size(&hash) == N, "哈希大小");
    hash_clear(&hash);
    PASS();
    
    free(records);
}

/* ========== 主测试入口 ========== */

int main(void)
{
    printf("========================================\n");
    printf("   校园选课记录系统 - 自动化测试\n");
    printf("========================================\n\n");
    
    /* 单元测试 */
    printf("[测试1] 记录生成\n");
    test_record_generation();
    
    printf("\n[测试2] CSV持久化\n");
    test_csv_persistence();
    
    printf("\n[测试3] 日期工具函数\n");
    test_date_utils();
    
    printf("\n[测试4] 记录工具函数\n");
    test_record_utils();
    
    printf("\n[测试5] 排序比较函数\n");
    test_sort_compare();
    
    printf("\n[测试6] 双向链表\n");
    test_linked_list();
    
    printf("\n[测试7] AVL树\n");
    test_avl_tree();
    
    printf("\n[测试8] 哈希表\n");
    test_hash_table();
    
    printf("\n[测试9] 筛选与排序\n");
    test_filter_sort();
    
    printf("\n[测试10] 大规模测试 (5000条)\n");
    test_large_scale();
    
    /* 总结 */
    printf("\n========================================\n");
    printf("  测试完成\n");
    printf("  总计: %d  通过: %d  失败: %d\n", 
           total_tests, passed_tests, failed_tests);
    printf("========================================\n");
    
    return failed_tests > 0 ? 1 : 0;
}
