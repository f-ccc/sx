/*
 * csv.c - CSV文件读写实现
 * 功能：实现选课记录的CSV格式持久化
 */

#include "csv.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        printf("错误：无法打开文件 %s 进行写入\n", filename);
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
        printf("错误：无法打开文件 %s 进行读取\n", filename);
        return ERR;
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
                    strcpy(records[idx].student_id, token);
                    break;
                case 1: /* 姓名 */
                    strcpy(records[idx].name, token);
                    break;
                case 2: /* 学院 */
                    strcpy(records[idx].college, token);
                    break;
                case 3: /* 课程编号 */
                    strcpy(records[idx].course_id, token);
                    break;
                case 4: /* 课程名称 */
                    strcpy(records[idx].course_name, token);
                    break;
                case 5: /* 学分 */
                    records[idx].credit = (float)atof(token);
                    break;
                case 6: /* 选课学期 */
                    strcpy(records[idx].semester, token);
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
