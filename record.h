/*
 * record.h - 选课记录数据结构定义
 * 功能：定义选课记录结构体、日期结构体及公共常量
 * 作者：课程设计
 */

#ifndef RECORD_H
#define RECORD_H

/* 常量定义 */
#define MAX_STUDENT_ID_LEN  14      /* 学号12位 + '\0' + 余量 */
#define MAX_NAME_LEN        21      /* 姓名最长20字 + '\0' */
#define MAX_COLLEGE_LEN     41      /* 学院最长40字 + '\0' */
#define MAX_COURSE_ID_LEN   9       /* 课程编号8位 + '\0' */
#define MAX_COURSE_NAME_LEN 51      /* 课程名称最长50字 + '\0' */
#define MAX_SEMESTER_LEN    8       /* 选课学期7位 + '\0' */
#define MAX_LINE_LEN        512     /* CSV一行最大长度 */
#define MAX_RECORDS         100000  /* 最大记录数 */

/* 日期结构体 */
typedef struct {
    int year;
    int month;
    int day;
} Date;

/* 选课记录结构体 */
typedef struct {
    char student_id[MAX_STUDENT_ID_LEN];   /* 学号 */
    char name[MAX_NAME_LEN];               /* 姓名 */
    char college[MAX_COLLEGE_LEN];         /* 学院 */
    char course_id[MAX_COURSE_ID_LEN];     /* 课程编号 */
    char course_name[MAX_COURSE_NAME_LEN]; /* 课程名称 */
    float credit;                          /* 学分 */
    char semester[MAX_SEMESTER_LEN];       /* 选课学期 */
    Date enroll_date;                      /* 选课日期 */
    int score;                             /* 成绩 0-100 */
} Record;

/* 比较结果常量 */
#define CMP_LESS    -1
#define CMP_EQUAL   0
#define CMP_GREATER 1

/* 排序方向 */
/* 排序方向 */
#define SORT_ASC    0
#define SORT_DESC   1

/* 日期比较结果 */
int date_compare(const Date *d1, const Date *d2);
int is_expired(const Record *rec, int base_year, int base_month, int base_day);
int record_compare(const Record *a, const Record *b);
int record_match_key(const Record *rec, const char *student_id, const char *course_id);

/* 数据生成 */
void generate_record(Record *rec, int index);
void generate_records(Record *records, int count);

/* 记录打印 */
void print_record(const Record *rec);
void print_record_header(void);
void print_record_footer(void);

/* 筛选字段类型 */
#define FILTER_COURSE_NAME  0
#define FILTER_SEMESTER     1
#define FILTER_SCORE_RANGE  2
#define FILTER_COLLEGE      3

/* 操作结果 */
#define OK      0
#define ERR    -1
#define NOT_FOUND -2
#define FULL   -3
#define DUPLICATE -4

#endif /* RECORD_H */
