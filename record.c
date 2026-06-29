/*
 * record.c - 选课记录操作实现
 * 功能：实现记录的基本操作、数据生成、记录比较等
 */

#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 字符串安全拷贝 */
static void str_copy(char *dest, const char *src, int max_len)
{
    int i;
    for (i = 0; i < max_len - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

/* 比较两个日期 */
int date_compare(const Date *d1, const Date *d2)
{
    if (d1->year != d2->year) {
        return (d1->year > d2->year) ? CMP_GREATER : CMP_LESS;
    }
    if (d1->month != d2->month) {
        return (d1->month > d2->month) ? CMP_GREATER : CMP_LESS;
    }
    if (d1->day != d2->day) {
        return (d1->day > d2->day) ? CMP_GREATER : CMP_LESS;
    }
    return CMP_EQUAL;
}

/* 检查记录是否过期（早于基准日期） */
int is_expired(const Record *rec, int base_year, int base_month, int base_day)
{
    Date three_years_ago;
    
    three_years_ago.year = base_year - 3;
    three_years_ago.month = base_month;
    three_years_ago.day = base_day;
    
    return (date_compare(&rec->enroll_date, &three_years_ago) == CMP_LESS);
}

/* 记录比较：按学号+课程编号比较 */
int record_compare(const Record *a, const Record *b)
{
    int cmp = strcmp(a->student_id, b->student_id);
    if (cmp != 0) return (cmp > 0) ? CMP_GREATER : CMP_LESS;
    cmp = strcmp(a->course_id, b->course_id);
    if (cmp != 0) return (cmp > 0) ? CMP_GREATER : CMP_LESS;
    return CMP_EQUAL;
}

/* 检查学号和课程编号是否匹配 */
int record_match_key(const Record *rec, const char *student_id, const char *course_id)
{
    return (strcmp(rec->student_id, student_id) == 0 && 
            strcmp(rec->course_id, course_id) == 0);
}

/* ========== 数据生成 ========== */

/* 姓氏库 */
static const char *surnames[] = {
    "赵","钱","孙","李","周","吴","郑","王","冯","陈",
    "褚","卫","蒋","沈","韩","杨","朱","秦","尤","许",
    "何","吕","施","张","孔","曹","严","华","金","魏",
    "陶","姜","戚","谢","邹","喻","柏","水","窦","章",
    "云","苏","潘","葛","范","彭","郎","鲁","韦","昌",
    "马","苗","凤","花","方","俞","任","袁","柳","邓"
};
#define SURNAME_COUNT 60

/* 名字库（单个字） */
static const char *given_names[] = {
    "伟","芳","娜","敏","静","丽","强","磊","军","洋",
    "勇","艳","杰","娟","涛","明","超","秀","霞","平",
    "刚","华","飞","鑫","鑫","宇","浩","雷","林","峰",
    "辉","玲","琳","婷","萱","颖","文","博","智","辉",
    "天","宇","翔","瑞","晨","阳","嘉","怡","俊","帅"
};
#define GIVEN_NAME_COUNT 50

/* 学院列表 */
static const char *colleges[] = {
    "计算机科学与工程学院",
    "数学与统计学院",
    "电子信息工程学院",
    "机电工程学院",
    "土木工程学院",
    "经济与管理学院",
    "外国语学院",
    "法学院"
};
#define COLLEGE_COUNT 8

/* 课程信息 */
static const char *course_data[][3] = {
    {"CS300101", "程序设计基础", "3.5"},
    {"CS300102", "数据结构与算法", "3.5"},
    {"CS300103", "操作系统原理", "3.0"},
    {"CS300104", "计算机网络", "3.0"},
    {"CS300105", "数据库系统原理", "3.0"},
    {"CS300201", "软件工程", "2.5"},
    {"CS300202", "算法设计与分析", "3.0"},
    {"CS300203", "人工智能导论", "2.5"},
    {"CS300204", "编译原理", "3.0"},
    {"CS300205", "计算机组成原理", "3.5"},
    {"MA400101", "高等数学A", "5.0"},
    {"MA400102", "线性代数", "3.0"},
    {"MA400103", "概率论与数理统计", "3.0"},
    {"EE500101", "电路分析", "3.0"},
    {"EE500102", "数字逻辑", "2.5"},
    {"ME600101", "工程制图", "2.0"},
    {"FL700101", "大学英语", "4.0"},
    {"LA800101", "法律基础", "2.0"}
};
#define COURSE_COUNT 18

/* 学期列表 */
static const char *semesters[] = {
    "2020-01", "2020-02",
    "2021-01", "2021-02",
    "2022-01", "2022-02",
    "2023-01", "2023-02",
    "2024-01", "2024-02",
    "2025-01", "2025-02",
    "2026-01"
};
#define SEMESTER_COUNT 13

/* 从学期解析年份和月份 */
static void parse_semester_date(const char *semester, int *year, int *month)
{
    char buf[8];
    int i;
    
    for (i = 0; i < 6 && semester[i] != '\0'; i++) {
        buf[i] = semester[i];
    }
    buf[i] = '\0';
    
    *year = (buf[0] - '0') * 1000 + (buf[1] - '0') * 100 + 
            (buf[2] - '0') * 10 + (buf[3] - '0');
    
    if (buf[5] == '1') {
        *month = 3;  /* 春季，约3月 */
    } else {
        *month = 9;  /* 秋季，约9月 */
    }
}

/* 生成随机姓名 */
static void generate_name(char *name)
{
    int sur_idx = rand() % SURNAME_COUNT;
    int given_idx = rand() % GIVEN_NAME_COUNT;
    
    /* 2字名或3字名 */
    if (rand() % 3 == 0) {
        /* 单字名 */
        sprintf(name, "%s%s", surnames[sur_idx], given_names[given_idx]);
    } else {
        /* 双字名 */
        int given_idx2 = rand() % GIVEN_NAME_COUNT;
        sprintf(name, "%s%s%s", surnames[sur_idx], 
                given_names[given_idx], given_names[given_idx2]);
    }
}

/* 生成一条随机选课记录 */
void generate_record(Record *rec, int index)
{
    int year;
    int course_idx;
    int semester_idx;
    int sem_year, sem_month;
    
    /* 学号：前4位入学年份 + 2位学院代码 + 6位序号 */
    year = 2020 + (index / 2000) % 4;  /* 2020-2023级 */
    {
        int college_code = (rand() % 8) + 1;
        int seq = (index % 999999) + 1;
        sprintf(rec->student_id, "%04d%02d%06d", year, college_code, seq);
    }
    
    /* 姓名 */
    generate_name(rec->name);
    
    /* 学院 */
    str_copy(rec->college, colleges[rand() % COLLEGE_COUNT], MAX_COLLEGE_LEN);
    
    /* 课程 */
    course_idx = rand() % COURSE_COUNT;
    str_copy(rec->course_id, course_data[course_idx][0], MAX_COURSE_ID_LEN);
    str_copy(rec->course_name, course_data[course_idx][1], MAX_COURSE_NAME_LEN);
    rec->credit = (float)atof(course_data[course_idx][2]);
    
    /* 选课学期 */
    semester_idx = rand() % SEMESTER_COUNT;
    str_copy(rec->semester, semesters[semester_idx], MAX_SEMESTER_LEN);
    
    /* 选课日期 - 与学期对应 */
    parse_semester_date(rec->semester, &sem_year, &sem_month);
    rec->enroll_date.year = sem_year;
    rec->enroll_date.month = sem_month;
    rec->enroll_date.day = (rand() % 28) + 1;
    {
        int r1, sum;
        double avg;
        
        sum = 0;
        for (r1 = 0; r1 < 3; r1++) {
            sum += rand() % 41;  /* 0-40 */
        }
        avg = 55.0 + (double)sum / 3.0;  /* 范围55-95 */
        
        rec->score = (int)avg;
        if (rec->score > 100) rec->score = 100;
        if (rec->score < 0) rec->score = 0;
    }
}

/* 批量生成选课记录 */
void generate_records(Record *records, int count)
{
    int i;
    
    srand((unsigned int)time(NULL));
    
    for (i = 0; i < count; i++) {
        generate_record(&records[i], i);
    }
}

/* 打印一条记录 */
void print_record(const Record *rec)
{
    printf("| %-12s | %-8s | %-8s | %-8s | %-16s | %-2.1f | %-7s | %04d-%02d-%02d | %3d |\n",
           rec->student_id, rec->name, rec->college, rec->course_id, rec->course_name,
           rec->credit, rec->semester,
           rec->enroll_date.year, rec->enroll_date.month, rec->enroll_date.day,
           rec->score);
}

/* 打印表头 */
void print_record_header(void)
{
    printf("+--------------+----------+----------+----------+------------------+-----+---------+------------+-----+\n");
    printf("| 学号         | 姓名    | 学院    | 课程编号 | 课程名称         |学分 | 学期   | 选课日期   |成绩 |\n");
    printf("+--------------+----------+----------+----------+------------------+-----+---------+------------+-----+\n");
}

/* 打印表尾 */
void print_record_footer(void)
{
    printf("+--------------+----------+----------+----------+------------------+-----+---------+------------+-----+\n");
}
