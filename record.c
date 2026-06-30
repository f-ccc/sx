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

/* ========== 字段合法性校验 ========== */

/* 学号校验：12位纯数字 */
int validate_student_id(const char *student_id)
{
    int len;
    int i;
    
    if (student_id == NULL) {
        return INVALID_ID;
    }
    
    len = strlen(student_id);
    if (len != 12) {
        return INVALID_ID;
    }
    
    for (i = 0; i < len; i++) {
        if (student_id[i] < '0' || student_id[i] > '9') {
            return INVALID_ID;
        }
    }
    
    return OK;
}

/* 学期校验：格式 "YYYY-01" 或 "YYYY-02" */
int validate_semester(const char *semester)
{
    int len;
    int year;
    
    if (semester == NULL) {
        return INVALID_SEMESTER;
    }
    
    len = strlen(semester);
    if (len != 7) {
        return INVALID_SEMESTER;
    }
    
    /* 前4位必须是数字（年份） */
    if (semester[0] < '0' || semester[0] > '9') return INVALID_SEMESTER;
    if (semester[1] < '0' || semester[1] > '9') return INVALID_SEMESTER;
    if (semester[2] < '0' || semester[2] > '9') return INVALID_SEMESTER;
    if (semester[3] < '0' || semester[3] > '9') return INVALID_SEMESTER;
    
    /* 第5位必须是 '-' */
    if (semester[4] != '-') return INVALID_SEMESTER;
    
    /* 第6-7位必须是 "01" 或 "02" */
    if (semester[5] != '0') return INVALID_SEMESTER;
    if (semester[6] != '1' && semester[6] != '2') return INVALID_SEMESTER;
    
    /* 年份范围：2000-2099 */
    year = (semester[0] - '0') * 1000 + (semester[1] - '0') * 100 +
           (semester[2] - '0') * 10 + (semester[3] - '0');
    if (year < 2000 || year > 2099) {
        return INVALID_SEMESTER;
    }
    
    return OK;
}

/* 判断闰年 */
static int is_leap_year(int year)
{
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

/* 获取某月的最大天数 */
static int days_in_month(int year, int month)
{
    static const int days[13] = {0, 31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31};
    
    if (month < 1 || month > 12) {
        return 0;
    }
    
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    
    return days[month];
}

/* 选课日期校验：合法日期、与学期对应、不晚于当前 */
int validate_enroll_date(const Date *date, const char *semester)
{
    int sem_year;
    int earliest_year, earliest_month, earliest_day;
    int latest_year, latest_month, latest_day;
    Date earliest, latest, now_estimate;
    time_t raw_time;
    struct tm *time_info;
    
    if (date == NULL) {
        return INVALID_DATE;
    }
    
    /* 1. 基本日期合法性：年月日范围 */
    if (date->year < 2000 || date->year > 2099) {
        return INVALID_DATE;
    }
    if (date->month < 1 || date->month > 12) {
        return INVALID_DATE;
    }
    if (date->day < 1 || date->day > days_in_month(date->year, date->month)) {
        return INVALID_DATE;
    }
    
    /* 2. 日期与学期对应：从学期解析年份和月份 */
    if (semester != NULL && strlen(semester) >= 7) {
        sem_year = (semester[0] - '0') * 1000 + (semester[1] - '0') * 100 +
                   (semester[2] - '0') * 10 + (semester[3] - '0');
        
        /* 春季学期(01)：开学约2月，选课截止约3月 */
        /* 秋季学期(02)：开学约8月，选课截止约10月 */
        if (semester[6] == '1') {  /* 春季 */
            earliest_month = 2;
            earliest_day = 1;
            latest_month = 4;
            latest_day = 30;
        } else {                    /* 秋季 */
            earliest_month = 8;
            earliest_day = 1;
            latest_month = 10;
            latest_day = 31;
        }
        earliest_year = sem_year;
        latest_year = sem_year;
        
        earliest.year = earliest_year;
        earliest.month = earliest_month;
        earliest.day = earliest_day;
        latest.year = latest_year;
        latest.month = latest_month;
        latest.day = latest_day;
        
        /* 日期不能早于开学时间 */
        if (date_compare(date, &earliest) == CMP_LESS) {
            return INVALID_DATE;
        }
        /* 日期不能晚于选课截止时间 */
        if (date_compare(date, &latest) == CMP_GREATER) {
            return INVALID_DATE;
        }
    }
    
    /* 3. 禁止未来日期（以编译时间为准的当前年判断） */
    /* 用 time 获取当前年份作为粗略检查 */
    raw_time = time(NULL);
    time_info = localtime(&raw_time);
    if (time_info != NULL) {
        int cur_year = time_info->tm_year + 1900;
        int cur_month = time_info->tm_mon + 1;
        int cur_day = time_info->tm_mday;
        
        now_estimate.year = cur_year;
        now_estimate.month = cur_month;
        now_estimate.day = cur_day;
        
        if (date_compare(date, &now_estimate) == CMP_GREATER) {
            return INVALID_DATE;
        }
    }
    
    return OK;
}

/* 成绩校验：0-100 */
int validate_score(int score)
{
    if (score < 0 || score > 100) {
        return INVALID_SCORE;
    }
    return OK;
}

/* 非空校验 */
int validate_not_empty(const char *str, const char *field_name)
{
    int i;
    
    (void)field_name;  /* 保留参数以备后续扩展 */
    
    if (str == NULL || str[0] == '\0') {
        return EMPTY_FIELD;
    }
    
    /* 检查是否纯空白 */
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ' && str[i] != '\t') {
            return OK;  /* 至少有一个非空白字符 */
        }
    }
    
    return EMPTY_FIELD;  /* 全是空白 */
}

/* 全面校验一条记录 */
int validate_record(const Record *rec)
{
    int ret;
    
    if (rec == NULL) return ERR;
    
    ret = validate_not_empty(rec->student_id, "学号");
    if (ret != OK) return ret;
    ret = validate_student_id(rec->student_id);
    if (ret != OK) return ret;
    
    ret = validate_not_empty(rec->name, "姓名");
    if (ret != OK) return ret;
    
    ret = validate_not_empty(rec->college, "学院");
    if (ret != OK) return ret;
    
    ret = validate_not_empty(rec->course_id, "课程编号");
    if (ret != OK) return ret;
    
    ret = validate_not_empty(rec->course_name, "课程名称");
    if (ret != OK) return ret;
    
    if (rec->credit <= 0.0f || rec->credit > 20.0f) {
        return INVALID_SCORE;  /* 复用INVALID_SCORE表示学分不合理 */
    }
    
    ret = validate_semester(rec->semester);
    if (ret != OK) return ret;
    
    ret = validate_enroll_date(&rec->enroll_date, rec->semester);
    if (ret != OK) return ret;
    
    ret = validate_score(rec->score);
    if (ret != OK) return ret;
    
    return OK;
}

/*
 * 在数组中检查是否存在三重键（学号+课程编号+学期）完全一致的重复记录
 * 用于区分：同一学生同一课程"同学期"=重复拦截，"不同学期"=重修放行
 * 返回 1=已存在（重复）, 0=不存在（正常/重修）
 */
int check_duplicate_triple_key(const Record *records, int count,
                                const char *student_id,
                                const char *course_id,
                                const char *semester)
{
    int i;
    
    if (records == NULL || student_id == NULL || course_id == NULL || semester == NULL) {
        return 0;
    }
    
    for (i = 0; i < count; i++) {
        if (strcmp(records[i].student_id, student_id) == 0 &&
            strcmp(records[i].course_id, course_id) == 0 &&
            strcmp(records[i].semester, semester) == 0) {
            return 1;  /* 同一学期重复选课 */
        }
    }
    
    return 0;  /* 不重复（或是不同学期的重修） */
}

/* 在数组中检查是否存在相同键（学号+课程编号）的记录 */
int check_duplicate_in_array(const Record *records, int count,
                              const char *student_id, const char *course_id)
{
    int i;
    
    if (records == NULL || student_id == NULL || course_id == NULL) {
        return 0;
    }
    
    for (i = 0; i < count; i++) {
        if (strcmp(records[i].student_id, student_id) == 0 &&
            strcmp(records[i].course_id, course_id) == 0) {
            return 1;  /* 重复 */
        }
    }
    
    return 0;  /* 不重复 */
}

/* 错误码 → 中文提示 */
const char *get_validate_error_msg(int err_code)
{
    switch (err_code) {
        case INVALID_ID:       return "学号格式错误：必须为12位纯数字";
        case INVALID_DATE:     return "选课日期错误：非法日期或与学期不匹配";
        case INVALID_SEMESTER: return "学期格式错误：必须为YYYY-01或YYYY-02";
        case INVALID_SCORE:    return "成绩错误：必须在0-100之间";
        case EMPTY_FIELD:      return "字段不能为空或纯空白";
        case DUPLICATE:        return "该课程本学期已完成选课，不可重复提交";
        default:               return "未知错误";
    }
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
    /* "YYYY-01" = 7 chars, 复制全部到buf */
    *year = (semester[0] - '0') * 1000 + (semester[1] - '0') * 100 +
            (semester[2] - '0') * 10 + (semester[3] - '0');
    
    /* 第6位(下标6): '1'=春季, '2'=秋季 */
    if (semester[6] == '1') {
        *month = 3;  /* 春季，约3月中旬 */
    } else {
        *month = 9;  /* 秋季，约9月中旬 */
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
    
    /* 选课日期 - 与学期对应的选课时段内随机 */
    parse_semester_date(rec->semester, &sem_year, &sem_month);
    rec->enroll_date.year = sem_year;
    if (rec->semester[6] == '1') {
        /* 春季学期：2月1日~4月30日 */
        rec->enroll_date.month = 2 + (rand() % 3);     /* 2,3,4月 */
    } else {
        /* 秋季学期：8月1日~10月31日 */
        rec->enroll_date.month = 8 + (rand() % 3);     /* 8,9,10月 */
    }
    rec->enroll_date.day = 1 + (rand() % 28);           /* 1~28日 */
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

/* 批量生成选课记录（含去重+校验，确保每条记录合法） */
void generate_records(Record *records, int count)
{
    int i;
    int attempts;
    const int MAX_ATTEMPTS = 100;  /* 防止死循环 */
    
    srand((unsigned int)time(NULL));
    
    for (i = 0; i < count; i++) {
        attempts = 0;
        do {
            generate_record(&records[i], i);
            attempts++;
            
            /* 最多尝试 MAX_ATTEMPTS 次生成合法且不重复的记录 */
        } while (attempts < MAX_ATTEMPTS && 
                 (validate_record(&records[i]) != OK ||   /* 字段不合法 */
                  check_duplicate_triple_key(records, i,   /* 三重键重复 */
                    records[i].student_id, 
                    records[i].course_id, 
                    records[i].semester)));
        
        if (attempts >= MAX_ATTEMPTS) {
            /* 极端情况下强行写入（几乎不会发生） */
            records[i].score = 75;
        }
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
