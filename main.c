/*
 * main.c - 校园选课记录检索与大数据分析系统
 * 功能：主程序入口，命令行菜单交互
 */

#include "record.h"
#include "list.h"
#include "avl.h"
#include "hash.h"
#include "csv.h"
#include "sort.h"
#include "stat.h"
#include "performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 前向声明 */
static void clear_screen(void);
static void pause_msg(const char *msg);
static void do_menu_action(int choice);

/* 全局数据 */
static Record *g_records = NULL;        /* 记录数组 */
static int g_record_count = 0;          /* 记录数量 */
static int g_record_capacity = 0;       /* 记录容量 */

static LinkedList g_list;               /* 链表存储 */
static AVLTree g_avl;                   /* AVL树存储 */
static HashTable g_hash;                /* 哈希表存储 */

/* 当前使用哪种数据结构 0=链表 1=AVL树 2=哈希表 */
/* 保留以备将来扩展 */

/* 数据文件名（可自定义） */
#define DEFAULT_DATA_FILE "data/records.csv"
#define MAX_FILE_PATH 256
static char g_data_file[MAX_FILE_PATH] = DEFAULT_DATA_FILE;

/* 初始容量（不再有上限，动态扩容） */
#define INITIAL_CAPACITY 1000
#define EXPAND_FACTOR 2

/* ========== 辅助函数 ========== */

/* 清屏 */
static void clear_screen(void)
{
    system("cls");
}

/* 暂停 - 等待用户按键 */
static void pause_msg(const char *msg)
{
    char buf[32];
    
    if (msg != NULL && msg[0] != '\0') {
        printf("%s", msg);
    } else {
        printf("\n按回车键继续...");
    }
    fgets(buf, sizeof(buf), stdin);
}

/* ========== 数据加载与保存 ========== */

/* 从文件加载数据（含校验+去重） */
static int load_data(void)
{
    int ret;
    int raw_count = 0;
    int valid_count = 0;
    int i, j;
    int *valid_flags = NULL;
    int dup_skipped = 0;
    int invalid_skipped = 0;
    
    ret = csv_load_records(g_data_file, g_records, &raw_count, g_record_capacity);
    if (ret != OK) {
        printf("错误：加载数据文件 %s 失败\n", g_data_file);
        return ERR;
    }
    
    if (raw_count == 0) {
        g_record_count = 0;
        return OK;
    }
    
    /* 逐条校验并去重 */
    valid_flags = (int *)malloc(sizeof(int) * raw_count);
    if (valid_flags == NULL) {
        printf("错误：内存不足\n");
        return ERR;
    }
    
    for (i = 0; i < raw_count; i++) {
        valid_flags[i] = 1;  /* 默认有效 */
        
        /* 字段校验 */
        ret = validate_record(&g_records[i]);
        if (ret != OK) {
            printf("  ⚠ 第%d条记录校验失败：%s\n", i + 1, get_validate_error_msg(ret));
            valid_flags[i] = 0;
            invalid_skipped++;
            continue;
        }
        
        /* 去重检查（前面有效记录中是否已有相同三重键：学号+课程编号+学期） */
        for (j = 0; j < i; j++) {
            if (valid_flags[j] &&
                strcmp(g_records[j].student_id, g_records[i].student_id) == 0 &&
                strcmp(g_records[j].course_id, g_records[i].course_id) == 0 &&
                strcmp(g_records[j].semester, g_records[i].semester) == 0) {
                printf("  ⚠ 第%d条与第%d条（同学期）重复，跳过\n", i + 1, j + 1);
                valid_flags[i] = 0;
                dup_skipped++;
                break;
            }
        }
    }
    
    /* 压缩数组：只保留有效记录 */
    for (i = 0; i < raw_count; i++) {
        if (valid_flags[i]) {
            g_records[valid_count++] = g_records[i];
        }
    }
    
    g_record_count = valid_count;
    
    free(valid_flags);
    
    printf("成功加载 %d 条记录", g_record_count);
    if (invalid_skipped > 0) printf("（跳过 %d 条非法记录）", invalid_skipped);
    if (dup_skipped > 0)     printf("（去除 %d 条重复记录）", dup_skipped);
    printf("\n");
    
    return OK;
}

/* 保存数据到文件 */
static int save_data(void)
{
    int ret;
    
    ret = csv_save_records(g_data_file, g_records, g_record_count);
    if (ret != OK) {
        printf("错误：保存数据失败\n");
        return ERR;
    }
    
    printf("数据已保存至 %s (共 %d 条记录)\n", g_data_file, g_record_count);
    return OK;
}

/* ========== 同步数据到所有数据结构 ========== */

/* 将记录数组同步到所有数据结构 */
static void sync_to_all_structures(void)
{
    int i;
    
    /* 清空所有结构 */
    list_clear(&g_list);
    avl_clear(&g_avl);
    hash_clear(&g_hash);
    
    /* 重新插入 */
    for (i = 0; i < g_record_count; i++) {
        list_insert(&g_list, &g_records[i]);
        avl_insert(&g_avl, &g_records[i]);
        hash_insert(&g_hash, &g_records[i]);
    }
}

/* ========== 数据生成 ========== */

static void menu_generate_data(void)
{
    int count;
    char input[32];
    
    clear_screen();
    printf("========================================\n");
    printf("          生成测试数据\n");
    printf("========================================\n");
    printf("请输入要生成的记录数量（如100、1000、10000）: ");
    
    fgets(input, sizeof(input), stdin);
    count = atoi(input);
    
    if (count <= 0) {
        printf("输入无效，请输入正整数\n");
        pause_msg(NULL);
        return;
    }
    
    /* 分配空间 */
    if (g_records != NULL) {
        free(g_records);
    }
    g_records = (Record *)malloc(sizeof(Record) * count);
    if (g_records == NULL) {
        printf("内存分配失败！\n");
        pause_msg(NULL);
        return;
    }
    g_record_capacity = count;
    
    printf("正在生成 %d 条选课记录...\n", count);
    generate_records(g_records, count);
    g_record_count = count;
    
    printf("生成完成！\n");
    
    /* 同步到各数据结构 */
    sync_to_all_structures();
    printf("数据已同步到链表、AVL树、哈希表\n");
    
    /* 保存到文件 */
    save_data();
    
    pause_msg(NULL);
}

/* ========== 基本操作（任务1） ========== */

static void menu_insert_record(void)
{
    Record rec;
    char input[32];
    Record *new_records;
    int new_capacity;
    int ret;
    int duplicate;
    
    clear_screen();
    printf("========================================\n");
    printf("          插入选课记录\n");
    printf("========================================\n");
    
    /* 容量不足时动态扩容 */
    if (g_record_count >= g_record_capacity) {
        new_capacity = (g_record_capacity == 0) ? INITIAL_CAPACITY : g_record_capacity * EXPAND_FACTOR;
        new_records = (Record *)realloc(g_records, sizeof(Record) * new_capacity);
        if (new_records == NULL) {
            printf("错误：内存扩容失败！\n");
            pause_msg(NULL);
            return;
        }
        g_records = new_records;
        g_record_capacity = new_capacity;
    }
    
    /* 输入学号并立即校验 */
    while (1) {
        printf("请输入学号（12位数字）: ");
        fgets(rec.student_id, MAX_STUDENT_ID_LEN, stdin);
        rec.student_id[strcspn(rec.student_id, "\n")] = '\0';
        
        ret = validate_student_id(rec.student_id);
        if (ret == OK) break;
        printf("  ⚠ %s，请重新输入\n", get_validate_error_msg(ret));
    }
    
    /* 输入课程编号并立即校验非空 */
    while (1) {
        printf("请输入课程编号（8位）: ");
        fgets(rec.course_id, MAX_COURSE_ID_LEN, stdin);
        rec.course_id[strcspn(rec.course_id, "\n")] = '\0';
        
        ret = validate_not_empty(rec.course_id, "课程编号");
        if (ret == OK) break;
        printf("  ⚠ %s，请重新输入\n", get_validate_error_msg(ret));
    }
    
    /* 去重检查：学号+课程编号+学期 三重键（同一学期重复选课拦截） */
    duplicate = check_duplicate_triple_key(g_records, g_record_count,
                                          rec.student_id, rec.course_id, rec.semester);
    if (duplicate) {
        printf("  ❌ 错误：学号 %s 本学期（%s）已选修课程 %s，不可重复提交！\n",
               rec.student_id, rec.semester, rec.course_id);
        printf("  （如需重修请选择其他学期）\n");
        pause_msg(NULL);
        return;
    }
    
    /* 输入姓名 */
    while (1) {
        printf("请输入姓名: ");
        fgets(rec.name, MAX_NAME_LEN, stdin);
        rec.name[strcspn(rec.name, "\n")] = '\0';
        
        ret = validate_not_empty(rec.name, "姓名");
        if (ret == OK) break;
        printf("  ⚠ %s，请重新输入\n", get_validate_error_msg(ret));
    }
    
    /* 输入学院 */
    while (1) {
        printf("请输入学院: ");
        fgets(rec.college, MAX_COLLEGE_LEN, stdin);
        rec.college[strcspn(rec.college, "\n")] = '\0';
        
        ret = validate_not_empty(rec.college, "学院");
        if (ret == OK) break;
        printf("  ⚠ %s，请重新输入\n", get_validate_error_msg(ret));
    }
    
    /* 输入课程名称 */
    while (1) {
        printf("请输入课程名称: ");
        fgets(rec.course_name, MAX_COURSE_NAME_LEN, stdin);
        rec.course_name[strcspn(rec.course_name, "\n")] = '\0';
        
        ret = validate_not_empty(rec.course_name, "课程名称");
        if (ret == OK) break;
        printf("  ⚠ %s，请重新输入\n", get_validate_error_msg(ret));
    }
    
    /* 输入学分 */
    while (1) {
        printf("请输入学分: ");
        fgets(input, sizeof(input), stdin);
        rec.credit = (float)atof(input);
        if (rec.credit > 0.0f && rec.credit <= 20.0f) break;
        printf("  ⚠ 学分无效（应为0.5~20.0之间的数），请重新输入\n");
    }
    
    /* 输入学期 */
    while (1) {
        printf("请输入选课学期（格式 YYYY-01 春季 或 YYYY-02 秋季）: ");
        fgets(rec.semester, MAX_SEMESTER_LEN, stdin);
        rec.semester[strcspn(rec.semester, "\n")] = '\0';
        
        ret = validate_semester(rec.semester);
        if (ret == OK) break;
        printf("  ⚠ %s，请重新输入\n", get_validate_error_msg(ret));
    }
    
    /* 输入选课日期 */
    while (1) {
        printf("请输入选课日期（年 月 日，空格分隔）: ");
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%d %d %d", &rec.enroll_date.year, 
               &rec.enroll_date.month, &rec.enroll_date.day);
        
        ret = validate_enroll_date(&rec.enroll_date, rec.semester);
        if (ret == OK) break;
        printf("  ⚠ %s\n", get_validate_error_msg(ret));
        printf("   提示：本学期选课日期范围应为");
        if (rec.semester[6] == '1') printf(" 2月1日~4月30日\n");
        else                        printf(" 8月1日~10月31日\n");
    }
    
    /* 输入成绩 */
    while (1) {
        printf("请输入成绩（0-100）: ");
        fgets(input, sizeof(input), stdin);
        rec.score = atoi(input);
        
        ret = validate_score(rec.score);
        if (ret == OK) break;
        printf("  ⚠ %s，请重新输入\n", get_validate_error_msg(ret));
    }
    
    /* 插入到数组 */
    g_records[g_record_count] = rec;
    g_record_count++;
    
    /* 同步到各数据结构 */
    list_insert(&g_list, &rec);
    avl_insert(&g_avl, &rec);
    hash_insert(&g_hash, &rec);
    
    printf("\n✅ 插入成功！当前共 %d 条记录\n", g_record_count);
    print_record_header();
    print_record(&rec);
    print_record_footer();
    
    pause_msg(NULL);
}

static void menu_delete_record(void)
{
    char student_id[MAX_STUDENT_ID_LEN];
    char course_id[MAX_COURSE_ID_LEN];
    
    clear_screen();
    printf("========================================\n");
    printf("          删除选课记录\n");
    printf("========================================\n");
    
    printf("请输入要删除记录的学号: ");
    fgets(student_id, MAX_STUDENT_ID_LEN, stdin);
    student_id[strcspn(student_id, "\n")] = '\0';
    
    printf("请输入要删除记录的课程编号: ");
    fgets(course_id, MAX_COURSE_ID_LEN, stdin);
    course_id[strcspn(course_id, "\n")] = '\0';
    
    /* 从所有结构中同步删除 */
    list_delete(&g_list, student_id, course_id);
    avl_delete(&g_avl, student_id, course_id);
    hash_delete(&g_hash, student_id, course_id);
    
    /* 也从数组删除 */
    {
        int i, j;
        int found = 0;
        for (i = 0; i < g_record_count; i++) {
            if (strcmp(g_records[i].student_id, student_id) == 0 &&
                strcmp(g_records[i].course_id, course_id) == 0) {
                found = 1;
                for (j = i; j < g_record_count - 1; j++) {
                    g_records[j] = g_records[j + 1];
                }
                g_record_count--;
                break;
            }
        }
        
        if (found) {
            printf("删除成功！当前共 %d 条记录\n", g_record_count);
        } else {
            printf("未找到匹配的记录\n");
        }
    }
    
    pause_msg(NULL);
}

static void menu_find_record(void)
{
    char student_id[MAX_STUDENT_ID_LEN];
    char course_id[MAX_COURSE_ID_LEN];
    HashNode *hnode;
    AVLNode *anode;
    ListNode *lnode;
    
    clear_screen();
    printf("========================================\n");
    printf("          查找选课记录\n");
    printf("========================================\n");
    
    printf("请输入学号: ");
    fgets(student_id, MAX_STUDENT_ID_LEN, stdin);
    student_id[strcspn(student_id, "\n")] = '\0';
    
    printf("请输入课程编号: ");
    fgets(course_id, MAX_COURSE_ID_LEN, stdin);
    course_id[strcspn(course_id, "\n")] = '\0';
    
    printf("\n查找结果:\n");
    
    /* 在三种结构中查找 */
    lnode = list_find_by_id(&g_list, student_id, course_id);
    anode = avl_find(&g_avl, student_id, course_id);
    hnode = hash_find(&g_hash, student_id, course_id);
    
    if (lnode != NULL) {
        printf("\n[链表中找到记录]\n");
        print_record_header();
        print_record(&lnode->data);
        print_record_footer();
    }
    
    if (anode != NULL) {
        printf("\n[AVL树中找到记录]\n");
        print_record_header();
        print_record(&anode->data);
        print_record_footer();
    }
    
    if (hnode != NULL) {
        printf("\n[哈希表中找到记录]\n");
        print_record_header();
        print_record(&hnode->data);
        print_record_footer();
    }
    
    if (lnode == NULL && anode == NULL && hnode == NULL) {
        printf("未找到匹配的记录\n");
    }
    
    pause_msg(NULL);
}

static void menu_update_record(void)
{
    char student_id[MAX_STUDENT_ID_LEN];
    char course_id[MAX_COURSE_ID_LEN];
    Record new_rec;
    char input[32];
    int i;
    int found = 0;
    
    clear_screen();
    printf("========================================\n");
    printf("          修改选课记录\n");
    printf("========================================\n");
    
    printf("请输入要修改记录的学号: ");
    fgets(student_id, MAX_STUDENT_ID_LEN, stdin);
    student_id[strcspn(student_id, "\n")] = '\0';
    
    printf("请输入要修改记录的课程编号: ");
    fgets(course_id, MAX_COURSE_ID_LEN, stdin);
    course_id[strcspn(course_id, "\n")] = '\0';
    
    /* 查找原记录 */
    for (i = 0; i < g_record_count; i++) {
        if (strcmp(g_records[i].student_id, student_id) == 0 &&
            strcmp(g_records[i].course_id, course_id) == 0) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        printf("未找到匹配的记录\n");
        pause_msg(NULL);
        return;
    }
    
    /* 输入新数据 */
    printf("请输入新的姓名（原: %s）: ", g_records[i].name);
    fgets(new_rec.name, MAX_NAME_LEN, stdin);
    new_rec.name[strcspn(new_rec.name, "\n")] = '\0';
    
    printf("请输入新的学院（原: %s）: ", g_records[i].college);
    fgets(new_rec.college, MAX_COLLEGE_LEN, stdin);
    new_rec.college[strcspn(new_rec.college, "\n")] = '\0';
    
    printf("请输入新的课程名称（原: %s）: ", g_records[i].course_name);
    fgets(new_rec.course_name, MAX_COURSE_NAME_LEN, stdin);
    new_rec.course_name[strcspn(new_rec.course_name, "\n")] = '\0';
    
    printf("请输入新的学分（原: %.1f）: ", g_records[i].credit);
    fgets(input, sizeof(input), stdin);
    new_rec.credit = (float)atof(input);
    
    printf("请输入新的学期（原: %s）: ", g_records[i].semester);
    fgets(new_rec.semester, MAX_SEMESTER_LEN, stdin);
    new_rec.semester[strcspn(new_rec.semester, "\n")] = '\0';
    
    printf("请输入新的成绩（原: %d）: ", g_records[i].score);
    fgets(input, sizeof(input), stdin);
    new_rec.score = atoi(input);
    
    new_rec.enroll_date = g_records[i].enroll_date;
    strcpy(new_rec.student_id, student_id);
    strcpy(new_rec.course_id, course_id);
    
    /* 更新所有结构 */
    list_update(&g_list, student_id, course_id, &new_rec);
    avl_update(&g_avl, student_id, course_id, &new_rec);
    hash_update(&g_hash, student_id, course_id, &new_rec);
    
    /* 更新数组 */
    g_records[i] = new_rec;
    
    printf("修改成功！\n");
    pause_msg(NULL);
}

/* ========== 浏览记录 ========== */

static void menu_list_records(void)
{
    int i;
    int page = 0;
    int page_size = 20;
    int total_pages;
    char input[32];
    
    if (g_record_count == 0) {
        printf("当前无记录\n");
        pause_msg(NULL);
        return;
    }
    
    total_pages = (g_record_count + page_size - 1) / page_size;
    
    while (1) {
        clear_screen();
        printf("========================================\n");
        printf("      浏览选课记录 (共 %d 条)\n", g_record_count);
        printf("      第 %d/%d 页\n", page + 1, total_pages);
        printf("========================================\n");
        
        print_record_header();
        
        for (i = page * page_size; i < g_record_count && i < (page + 1) * page_size; i++) {
            print_record(&g_records[i]);
        }
        
        print_record_footer();
        
        printf("\n操作: n-下一页 p-上一页 q-返回: ");
        fgets(input, sizeof(input), stdin);
        
        if (input[0] == 'n' || input[0] == 'N') {
            if (page < total_pages - 1) page++;
        } else if (input[0] == 'p' || input[0] == 'P') {
            if (page > 0) page--;
        } else if (input[0] == 'q' || input[0] == 'Q') {
            break;
        }
    }
}

/* ========== 数据持久化（任务2） ========== */

static void menu_save_load(void)
{
    char input[32];
    
    clear_screen();
    printf("========================================\n");
    printf("          数据持久化\n");
    printf("========================================\n");
    printf("当前文件: %s\n", g_data_file);
    printf("----------------------------------------\n");
    printf("1. 保存数据到文件\n");
    printf("2. 从文件加载数据\n");
    printf("3. 修改保存/加载的文件路径\n");
    printf("0. 返回\n");
    printf("请选择: ");
    
    fgets(input, sizeof(input), stdin);
    
    switch (input[0]) {
        case '1':
            save_data();
            pause_msg(NULL);
            break;
        case '2':
            load_data();
            sync_to_all_structures();
            pause_msg(NULL);
            break;
        case '3':
            printf("\n请输入新的文件路径（如 data/my_records.csv）: ");
            fgets(g_data_file, MAX_FILE_PATH, stdin);
            {
                int len = strlen(g_data_file);
                if (len > 0 && g_data_file[len-1] == '\n') {
                    g_data_file[len-1] = '\0';
                }
            }
            printf("文件路径已修改为: %s\n", g_data_file);
            printf("提示：保存时将自动创建所需的目录\n");
            pause_msg(NULL);
            break;
    }
}

/* ========== 筛选与排序（任务3） ========== */

static void menu_filter_sort(void)
{
    FilterCondition conditions[4];
    int cond_count = 0;
    ResultSet result;
    char input[32];
    int i;
    int choice;
    
    clear_screen();
    printf("========================================\n");
    printf("      多条件筛选与排序\n");
    printf("========================================\n");
    
    if (g_record_count == 0) {
        printf("当前无数据，请先生成或加载数据\n");
        pause_msg(NULL);
        return;
    }
    
    /* 选择筛选条件 */
    printf("请选择筛选条件（可多选，输入0结束选择）:\n");
    printf("1. 课程名称\n");
    printf("2. 选课学期\n");
    printf("3. 成绩区间\n");
    printf("4. 学院\n");
    printf("0. 结束选择\n");
    
    while (cond_count < 4) {
        printf("选择: ");
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);
        
        if (choice == 0) break;
        
        switch (choice) {
            case 1:
                conditions[cond_count].field = FILTER_COURSE_NAME;
                printf("  请输入课程名称（支持模糊匹配）: ");
                fgets(conditions[cond_count].str_value, 64, stdin);
                conditions[cond_count].str_value[strcspn(conditions[cond_count].str_value, "\n")] = '\0';
                conditions[cond_count].is_fuzzy = 1;
                cond_count++;
                break;
            case 2:
                conditions[cond_count].field = FILTER_SEMESTER;
                printf("  请输入学期（如2024-02）: ");
                fgets(conditions[cond_count].str_value, 64, stdin);
                conditions[cond_count].str_value[strcspn(conditions[cond_count].str_value, "\n")] = '\0';
                conditions[cond_count].is_fuzzy = 0;
                cond_count++;
                break;
            case 3:
                conditions[cond_count].field = FILTER_SCORE_RANGE;
                printf("  请输入成绩最小值: ");
                fgets(input, sizeof(input), stdin);
                conditions[cond_count].int_min = atoi(input);
                printf("  请输入成绩最大值: ");
                fgets(input, sizeof(input), stdin);
                conditions[cond_count].int_max = atoi(input);
                cond_count++;
                break;
            case 4:
                conditions[cond_count].field = FILTER_COLLEGE;
                printf("  请输入学院名称: ");
                fgets(conditions[cond_count].str_value, 64, stdin);
                conditions[cond_count].str_value[strcspn(conditions[cond_count].str_value, "\n")] = '\0';
                conditions[cond_count].is_fuzzy = 0;
                cond_count++;
                break;
        }
    }
    
    /* 执行筛选 */
    if (cond_count > 0) {
        filter_records(g_records, g_record_count, conditions, cond_count, &result);
    } else {
        /* 无筛选条件，使用全部数据 */
        result_set_init(&result, g_record_count);
        for (i = 0; i < g_record_count; i++) {
            result_set_add(&result, &g_records[i]);
        }
    }
    
    printf("\n筛选结果: %d 条记录\n", result.count);
    
    if (result.count > 0) {
        /* 排序选项 */
        printf("\n是否进行排序? (y/n): ");
        fgets(input, sizeof(input), stdin);
        
        if (input[0] == 'y' || input[0] == 'Y') {
            SortKey keys[5];
            int key_count = 0;
            int key_field, key_dir;
            
            printf("请选择排序关键字（可多级，输入0结束）:\n");
            printf("1. 学号  2. 姓名  3. 学院  \n");
            printf("4. 课程编号  5. 课程名称  6. 学分\n");
            printf("7. 学期  8. 成绩  9. 选课日期\n");
            printf("0. 结束\n");
            
            while (key_count < 5) {
                printf("排序字段: ");
                fgets(input, sizeof(input), stdin);
                key_field = atoi(input);
                if (key_field == 0) break;
                
                printf("  排序方向 (1-升序 2-降序): ");
                fgets(input, sizeof(input), stdin);
                key_dir = atoi(input);
                
                if (key_field >= 1 && key_field <= 9 && (key_dir == 1 || key_dir == 2)) {
                    keys[key_count].field = key_field - 1;  /* 转换为内部字段索引 */
                    keys[key_count].direction = (key_dir == 1) ? SORT_ASC : SORT_DESC;
                    key_count++;
                }
            }
            
            if (key_count > 0) {
                sort_records(result.records, result.count, keys, key_count);
            }
        }
        
        /* 显示结果 */
        printf("\n筛选结果:\n");
        print_record_header();
        for (i = 0; i < result.count && i < 30; i++) {
            print_record(&result.records[i]);
        }
        if (result.count > 30) {
            printf("... (共 %d 条，仅显示前30条)\n", result.count);
        }
        print_record_footer();
        
        /* 导出选项 */
        printf("\n是否导出到文件? (y/n): ");
        fgets(input, sizeof(input), stdin);
        if (input[0] == 'y' || input[0] == 'Y') {
            csv_save_records_from_list("data/filter_result.csv", result.records, result.count);
            printf("结果已导出到 data/filter_result.csv\n");
        }
    }
    
    result_set_free(&result);
    pause_msg(NULL);
}

/* ========== 统计分析（任务4） ========== */

static void menu_statistics(void)
{
    char input[32];
    
    if (g_record_count == 0) {
        printf("当前无数据，请先生成或加载数据\n");
        pause_msg(NULL);
        return;
    }
    
    clear_screen();
    printf("========================================\n");
    printf("          数据统计分析\n");
    printf("========================================\n");
    printf("1. 每门课程选课人数与容量使用率\n");
    printf("2. 每位学生选课门数与总学分\n");
    printf("3. 各学院选课人数分布\n");
    printf("4. 按学期统计选课人数与课程数分布\n");
    printf("5. 成绩分布统计\n");
    printf("0. 返回\n");
    printf("请选择: ");
    
    fgets(input, sizeof(input), stdin);
    
    switch (input[0]) {
        case '1':
            stat_course_enrollment(g_records, g_record_count);
            break;
        case '2':
            stat_student_courses(g_records, g_record_count);
            break;
        case '3':
            stat_college_distribution(g_records, g_record_count);
            break;
        case '4':
            stat_semester_distribution(g_records, g_record_count);
            break;
        case '5':
            stat_score_distribution(g_records, g_record_count);
            break;
    }
    
    if (input[0] >= '1' && input[0] <= '5') {
        pause_msg(NULL);
    }
}

/* ========== 批量删除过期记录（任务5） ========== */

static void menu_delete_expired(void)
{
    int deleted_list = 0, deleted_avl = 0, deleted_hash = 0;
    int total;
    char input[32];
    
    clear_screen();
    printf("========================================\n");
    printf("          批量删除过期记录\n");
    printf("========================================\n");
    printf("以2026年9月1日为基准，删除3年前（2023年9月1日前）的过期记录\n");
    
    /* 先统计 */
    {
        int i;
        total = 0;
        for (i = 0; i < g_record_count; i++) {
            if (is_expired(&g_records[i], 2026, 9, 1)) {
                total++;
            }
        }
    }
    
    if (total == 0) {
        printf("当前没有过期记录需要删除。\n");
        pause_msg(NULL);
        return;
    }
    
    printf("即将删除 %d 条过期记录，是否继续? (y/n): ", total);
    fgets(input, sizeof(input), stdin);
    
    if (input[0] != 'y' && input[0] != 'Y') {
        printf("操作已取消\n");
        pause_msg(NULL);
        return;
    }
    
    /* 从所有数据结构中删除 */
    list_delete_expired(&g_list, 2026, 9, 1, &deleted_list);
    avl_delete_expired(&g_avl, 2026, 9, 1, &deleted_avl);
    hash_delete_expired(&g_hash, 2026, 9, 1, &deleted_hash);
    
    /* 从数组中删除 */
    {
        int i, j;
        j = 0;
        for (i = 0; i < g_record_count; i++) {
            if (!is_expired(&g_records[i], 2026, 9, 1)) {
                g_records[j++] = g_records[i];
            }
        }
        g_record_count = j;
    }
    
    printf("删除完成！\n");
    printf("链表删除: %d 条\n", deleted_list);
    printf("AVL树删除: %d 条\n", deleted_avl);
    printf("哈希表删除: %d 条\n", deleted_hash);
    printf("当前共 %d 条记录\n", g_record_count);
    
    pause_msg(NULL);
}

/* ========== 性能对比测试（任务6） ========== */

static void menu_performance_test(void)
{
    PerfResult results[3];
    int result_count = 0;
    int sizes[] = {100, 1000, 10000};
    int i;
    Record *test_data = NULL;
    
    clear_screen();
    printf("========================================\n");
    printf("      多结构性能对比测试\n");
    printf("========================================\n");
    printf("将在 100/1000/10000 条数据规模下测试三种数据结构\n");
    printf("测试操作: 插入、查找、删除、遍历、排序\n\n");
    printf("正在生成测试数据...\n");
    
    /* 为每个规模分别生成测试数据 */
    for (i = 0; i < 3; i++) {
        /* 生成测试数据 */
        if (test_data != NULL) free(test_data);
        test_data = (Record *)malloc(sizeof(Record) * sizes[i]);
        if (test_data == NULL) {
            printf("内存分配失败！\n");
            pause_msg(NULL);
            return;
        }
        
        /* 固定随机种子，保证每次测试数据一致 */
        srand(12345);
        generate_records(test_data, sizes[i]);
        
        run_performance_test(test_data, sizes[i], &results[i]);
        result_count++;
    }
    
    /* 打印报告 */
    print_performance_report(results, result_count);
    print_complexity_analysis();
    
    if (test_data != NULL) free(test_data);
    pause_msg(NULL);
}

/* ========== 大规模压力测试（附加任务1） ========== */

static void menu_stress_test(void)
{
    int i;
    Record *test_data = NULL;
    const int STRESS_SIZE = 100000;
    LinkedList list;
    AVLTree tree;
    HashTable hash;
    double start, end;
    
    clear_screen();
    printf("========================================\n");
    printf("      大规模数据压力测试 (10万条)\n");
    printf("========================================\n");
    printf("正在生成 100000 条测试数据...\n");
    
    test_data = (Record *)malloc(sizeof(Record) * STRESS_SIZE);
    if (test_data == NULL) {
        printf("内存分配失败！\n");
        pause_msg(NULL);
        return;
    }
    
    srand(12345);
    generate_records(test_data, STRESS_SIZE);
    printf("数据生成完成！\n\n");
    
    /* 链表测试 */
    printf("--- 链表测试 ---\n");
    list_init(&list);
    start = get_time_ms();
    for (i = 0; i < STRESS_SIZE; i++) {
        list_insert(&list, &test_data[i]);
    }
    end = get_time_ms();
    printf("插入 %d 条: %.2f ms\n", STRESS_SIZE, end - start);
    
    start = get_time_ms();
    for (i = 0; i < 1000; i++) {
        list_find_by_id(&list, test_data[i * 100].student_id, test_data[i * 100].course_id);
    }
    end = get_time_ms();
    printf("查找 1000 次: %.2f ms\n", end - start);
    list_clear(&list);
    
    /* AVL树测试 */
    printf("\n--- AVL树测试 ---\n");
    avl_init(&tree);
    start = get_time_ms();
    for (i = 0; i < STRESS_SIZE; i++) {
        avl_insert(&tree, &test_data[i]);
    }
    end = get_time_ms();
    printf("插入 %d 条: %.2f ms\n", STRESS_SIZE, end - start);
    
    start = get_time_ms();
    for (i = 0; i < 1000; i++) {
        avl_find(&tree, test_data[i * 100].student_id, test_data[i * 100].course_id);
    }
    end = get_time_ms();
    printf("查找 1000 次: %.2f ms\n", end - start);
    avl_clear(&tree);
    
    /* 哈希表测试 */
    printf("\n--- 哈希表测试 ---\n");
    hash_init(&hash);
    start = get_time_ms();
    for (i = 0; i < STRESS_SIZE; i++) {
        hash_insert(&hash, &test_data[i]);
    }
    end = get_time_ms();
    printf("插入 %d 条: %.2f ms\n", STRESS_SIZE, end - start);
    
    start = get_time_ms();
    for (i = 0; i < 1000; i++) {
        hash_find(&hash, test_data[i * 100].student_id, test_data[i * 100].course_id);
    }
    end = get_time_ms();
    printf("查找 1000 次: %.2f ms\n", end - start);
    hash_clear(&hash);
    
    free(test_data);
    
    printf("\n压力测试完成！\n");
    pause_msg(NULL);
}

/* ========== 外部排序（附加任务2） ========== */

static void menu_external_sort(void)
{
    const int TOTAL = 100000;
    const int MEM_LIMIT = 1000;
    
    clear_screen();
    printf("========================================\n");
    printf("   内存受限环境下的外部排序\n");
    printf("========================================\n");
    printf("模拟内存仅能容纳 %d 条记录\n", MEM_LIMIT);
    printf("对 %d 条选课记录按学号进行外部排序\n\n", TOTAL);
    
    printf("外部排序策略：多路归并排序\n");
    printf("第1步：将10万条数据分成 %d 个块，每块 %d 条，分别排序后写入临时文件\n",
           (TOTAL + MEM_LIMIT - 1) / MEM_LIMIT, MEM_LIMIT);
    
    {
        int block_count = (TOTAL + MEM_LIMIT - 1) / MEM_LIMIT;
        int passes = 0;
        int merge_files;
        
        merge_files = block_count;
        printf("第1轮：生成 %d 个有序块\n", block_count);
        
        merge_files = block_count;
        while (merge_files > 1) {
            int merged;
            passes++;
            merged = (merge_files + 1) / 2;
            printf("第%d轮归并：%d个文件 → %d个文件\n", passes + 1, merge_files, merged);
            printf("  读写次数：约 %d 次\n", merge_files * 2);
            merge_files = merged;
        }
        
        printf("\n总轮数：%d\n", passes + 1);
        printf("临时文件数：约 %d 个\n", block_count);
        printf("总I/O读写次数：约 %d 次\n", block_count * 2 + (passes) * block_count * 2);
        printf("\n时间复杂度：O(n log n)\n");
        printf("空间复杂度：O(M) 其中M为内存限制（%d条）\n", MEM_LIMIT);
    }
    
    printf("\n外部排序分析完成！\n");
    pause_msg(NULL);
}

/* ========== 使用说明 ========== */

static void menu_help(void)
{
    clear_screen();
    printf("========================================\n");
    printf("          系统使用说明\n");
    printf("========================================\n");
    printf("项目名称：校园选课记录检索与大数据分析系统\n\n");
    printf("功能列表：\n");
    printf("  1. 生成测试数据  - 生成指定数量的随机选课记录\n");
    printf("  2. 插入记录      - 添加一条新的选课记录\n");
    printf("  3. 删除记录      - 按学号+课程编号删除记录\n");
    printf("  4. 修改记录      - 修改指定记录的信息\n");
    printf("  5. 查找记录      - 按学号+课程编号查找\n");
    printf("  6. 浏览记录      - 分页浏览所有记录\n");
    printf("  7. 数据持久化    - 保存/加载CSV文件\n");
    printf("  8. 筛选与排序    - 多条件筛选与多关键字排序\n");
    printf("  9. 统计分析      - 5种统计分析功能\n");
    printf("  10. 批量删除     - 删除3年前的过期记录\n");
    printf("  11. 性能测试     - 多结构性能对比与复杂度验证\n");
    printf("  12. 压力测试     - 10万条数据压力测试(附加)\n");
    printf("  13. 外部排序     - 内存受限外部排序分析(附加)\n\n");
    printf("数据结构：\n");
    printf("  - 双向链表：所有操作的O(n)基础实现\n");
    printf("  - AVL树：自平衡二叉搜索树，O(log n)操作\n");
    printf("  - 哈希表：链地址法，平均O(1)操作\n");
    printf("  - 三种结构自动同步，保证数据一致性\n\n");
    printf("数据文件：%s（可在数据持久化菜单中修改路径）\n", g_data_file);
    pause_msg(NULL);
}

/* ========== 主菜单 ========== */

int main(void)
{
    char input[32];
    
    /* 初始化 */
    g_records = NULL;
    g_record_count = 0;
    g_record_capacity = 0;
    
    list_init(&g_list);
    avl_init(&g_avl);
    hash_init(&g_hash);
    
    /* 尝试加载已有数据 */
    {
        g_records = (Record *)malloc(sizeof(Record) * INITIAL_CAPACITY);
        if (g_records != NULL) {
            g_record_capacity = INITIAL_CAPACITY;
            if (load_data() == OK) {
                sync_to_all_structures();
            }
        }
    }
    
    while (1) {
        clear_screen();
        printf("========================================\n");
        printf("   校园选课记录检索与大数据分析系统\n");
        printf("========================================\n");
        printf("当前记录数: %d\n", g_record_count);
        printf("----------------------------------------\n");
        printf("【基本操作】\n");
        printf("  1. 生成测试数据   2. 插入记录\n");
        printf("  3. 删除记录       4. 修改记录\n");
        printf("  5. 查找记录       6. 浏览记录\n");
        printf("【数据处理】\n");
        printf("  7. 数据持久化     8. 筛选与排序\n");
        printf("  9. 统计分析      10. 批量删除过期记录\n");
        printf("【性能测试】\n");
        printf(" 11. 性能对比测试   12. 压力测试(附加)\n");
        printf(" 13. 外部排序(附加)\n");
        printf("【其他】\n");
        printf("  h. 使用说明       0. 退出系统\n");
        printf("----------------------------------------\n");
        printf("请选择: ");
        
        fgets(input, sizeof(input), stdin);
        
        /* 使用atoi解析数字选项，支持1-13 */
        if (input[0] == 'h' || input[0] == 'H') {
            menu_help();
        } else {
            int choice = atoi(input);
            do_menu_action(choice);
        }
    }
    
    return 0;
}

/* ========== 菜单动作分发 ========== */

static void do_menu_action(int choice)
{
    switch (choice) {
        case 1:  menu_generate_data();    break;
        case 2:  menu_insert_record();    break;
        case 3:  menu_delete_record();    break;
        case 4:  menu_update_record();    break;
        case 5:  menu_find_record();      break;
        case 6:  menu_list_records();     break;
        case 7:  menu_save_load();        break;
        case 8:  menu_filter_sort();      break;
        case 9:  menu_statistics();       break;
        case 10: menu_delete_expired();   break;
        case 11: menu_performance_test(); break;
        case 12: menu_stress_test();      break;
        case 13: menu_external_sort();    break;
        case 0:
            if (g_record_count > 0) {
                save_data();
            }
            printf("感谢使用！\n");
            
            if (g_records != NULL) free(g_records);
            list_clear(&g_list);
            avl_clear(&g_avl);
            hash_clear(&g_hash);
            
            exit(0);
        default:
            printf("无效选择\n");
            pause_msg(NULL);
            break;
    }
}
