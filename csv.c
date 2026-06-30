/*
 * csv.c - CSV文件读写实现
 * 功能：实现选课记录的CSV格式持久化
 */

#include "csv.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* 
 * 确保文件路径的目录存在，不存在则创建
 * 如 "data/sub/my.csv" 会创建 data/ 和 data/sub/
 */
static int ensure_dir_exists(const char *filename)
{
    char dir[MAX_LINE_LEN];
    int i;
    int len;
    
    /* 复制路径 */
    len = strlen(filename);
    if (len >= MAX_LINE_LEN - 1) {
        return ERR;
    }
    for (i = 0; i <= len; i++) {
        dir[i] = filename[i];
    }
    
    /* 从根开始逐级创建目录 */
    for (i = 0; dir[i] != '\0'; i++) {
        if (dir[i] == '/' || dir[i] == '\\') {
            char saved = dir[i];
            dir[i] = '\0';
            /* 尝试创建目录，已存在则忽略错误 */
            mkdir(dir);
            dir[i] = saved;
        }
    }
    
    return OK;
}

/* 
 * CSV格式说明：
 * 第一行为表头
 * 每行一条记录，字段用逗号分隔
 * 字段顺序：学号,姓名,学院,课程编号,课程名称,学分,选课学期,选课日期(YYYY-MM-DD),成绩
 */

/* 保存记录到CSV文件 */
int csv_save_records(const char *filename, const Record *records, int count)
{
    FILE *fp;
    int i;
    
    fp = fopen(filename, "w");
    if (fp == NULL) {
        /* 尝试创建目录后重试 */
        if (ensure_dir_exists(filename) == OK) {
            fp = fopen(filename, "w");
        }
    }
    if (fp == NULL) {
        printf("错误：无法创建/写入文件 %s\n", filename);
        printf("可能的原因：\n");
        printf("  1. 路径中含有非法字符（如 : * ? \" < > | 等）\n");
        printf("  2. 磁盘空间不足或无写入权限\n");
        printf("  3. 路径过长\n");
        return ERR;
    }
    
    /* 写入表头 */
    fprintf(fp, "学号,姓名,学院,课程编号,课程名称,学分,选课学期,选课日期,成绩\n");
    
    /* 写入数据 */
    for (i = 0; i < count; i++) {
        fprintf(fp, "%s,%s,%s,%s,%s,%.1f,%s,%04d-%02d-%02d,%d\n",
                records[i].student_id,
                records[i].name,
                records[i].college,
                records[i].course_id,
                records[i].course_name,
                records[i].credit,
                records[i].semester,
                records[i].enroll_date.year,
                records[i].enroll_date.month,
                records[i].enroll_date.day,
                records[i].score);
    }
    
    fclose(fp);
    return OK;
}

/* 安全字符串拷贝（带边界检查） */
static void safe_strcpy(char *dest, const char *src, int max_len)
{
    int i;
    if (dest == NULL || max_len <= 0) return;
    for (i = 0; i < max_len - 1 && src != NULL && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

/* 
 * 从CSV文件加载记录
 * 注意：简单CSV解析，假设字段内不含逗号和换行符
 */
int csv_load_records(const char *filename, Record *records, int *count, int max_count)
{
    FILE *fp;
    char line[MAX_LINE_LEN];
    int line_no = 0;
    int idx = 0;
    char *token;
    
    fp = fopen(filename, "r");
    if (fp == NULL) {
        /* 文件不存在不是致命错误，返回0条记录即可 */
        *count = 0;
        return OK;
    }
    
    /* 跳过表头行 */
    if (fgets(line, MAX_LINE_LEN, fp) == NULL) {
        fclose(fp);
        *count = 0;
        return OK;
    }
    line_no++;
    
    /* 逐行读取 */
    while (fgets(line, MAX_LINE_LEN, fp) != NULL && idx < max_count) {
        int field_no = 0;
        char *p;
        
        line_no++;
        
        /* 去掉末尾换行符 */
        p = line;
        while (*p != '\0') {
            if (*p == '\n' || *p == '\r') {
                *p = '\0';
                break;
            }
            p++;
        }
        
        /* 解析字段 */
        token = strtok(line, ",");
        while (token != NULL && field_no < 9) {
            switch (field_no) {
                case 0: /* 学号 */
                    safe_strcpy(records[idx].student_id, token, MAX_STUDENT_ID_LEN);
                    break;
                case 1: /* 姓名 */
                    safe_strcpy(records[idx].name, token, MAX_NAME_LEN);
                    break;
                case 2: /* 学院 */
                    safe_strcpy(records[idx].college, token, MAX_COLLEGE_LEN);
                    break;
                case 3: /* 课程编号 */
                    safe_strcpy(records[idx].course_id, token, MAX_COURSE_ID_LEN);
                    break;
                case 4: /* 课程名称 */
                    safe_strcpy(records[idx].course_name, token, MAX_COURSE_NAME_LEN);
                    break;
                case 5: /* 学分 */
                    records[idx].credit = (float)atof(token);
                    break;
                case 6: /* 选课学期 */
                    safe_strcpy(records[idx].semester, token, MAX_SEMESTER_LEN);
                    break;
                case 7: /* 选课日期 */
                    sscanf(token, "%d-%d-%d", 
                           &records[idx].enroll_date.year,
                           &records[idx].enroll_date.month,
                           &records[idx].enroll_date.day);
                    break;
                case 8: /* 成绩 */
                    records[idx].score = atoi(token);
                    break;
            }
            field_no++;
            token = strtok(NULL, ",");
        }
        
        if (field_no >= 9) {
            idx++;
        }
    }
    
    fclose(fp);
    *count = idx;
    return OK;
}

/* 从列表中保存记录到CSV（用于筛选结果导出） */
int csv_save_records_from_list(const char *filename, const Record *records, int count)
{
    return csv_save_records(filename, records, count);
}
