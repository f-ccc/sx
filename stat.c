/*
 * stat.c - 统计分析实现
 * 功能：实现数据统计分析功能（至少完成3项，这里实现全部5项）
 */

#include "stat.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STAT_ITEMS 1000

/* 内部：查找或添加统计项 */
static int find_or_add_course(CourseStat *stats, int *count, 
                               const char *course_id, const char *course_name)
{
    int i;
    
    for (i = 0; i < *count; i++) {
        if (strcmp(stats[i].course_id, course_id) == 0) {
            return i;
        }
    }
    
    /* 添加新项 */
    strcpy(stats[*count].course_id, course_id);
    strcpy(stats[*count].course_name, course_name);
    stats[*count].student_count = 0;
    stats[*count].capacity = 60;  /* 默认容量 */
    stats[*count].usage_rate = 0.0f;
    (*count)++;
    
    return *count - 1;
}

/* 统计1：每门课程的选课人数与容量使用率 */
void stat_course_enrollment(const Record *records, int count)
{
    CourseStat course_stats[MAX_STAT_ITEMS];
    int course_count = 0;
    int i;
    int idx;
    
    for (i = 0; i < count; i++) {
        idx = find_or_add_course(course_stats, &course_count,
                                  records[i].course_id, records[i].course_name);
        course_stats[idx].student_count++;
    }
    
    /* 计算使用率 */
    for (i = 0; i < course_count; i++) {
        course_stats[i].usage_rate = 
            (float)course_stats[i].student_count / course_stats[i].capacity * 100.0f;
    }
    
    /* 输出 */
    printf("\n========== 每门课程选课人数与容量使用率 ==========\n");
    printf("+----------+------------------+--------------+----------+----------+\n");
    printf("| 课程编号 | 课程名称         | 选课人数     | 容量     | 使用率%%  |\n");
    printf("+----------+------------------+--------------+----------+----------+\n");
    
    for (i = 0; i < course_count; i++) {
        printf("| %-8s | %-16s | %-12d | %-8d | %-7.1f |\n",
               course_stats[i].course_id,
               course_stats[i].course_name,
               course_stats[i].student_count,
               course_stats[i].capacity,
               course_stats[i].usage_rate);
    }
    
    printf("+----------+------------------+--------------+----------+----------+\n");
    printf("共 %d 门课程\n", course_count);
}

/* 内部：查找或添加学生统计项 */
static int find_or_add_student(StudentStat *stats, int *count,
                                const char *student_id, const char *name)
{
    int i;
    
    for (i = 0; i < *count; i++) {
        if (strcmp(stats[i].student_id, student_id) == 0) {
            return i;
        }
    }
    
    strcpy(stats[*count].student_id, student_id);
    strcpy(stats[*count].name, name);
    stats[*count].course_count = 0;
    stats[*count].total_credit = 0.0f;
    (*count)++;
    
    return *count - 1;
}

/* 统计2：每位学生的选课门数与总学分 */
void stat_student_courses(const Record *records, int count)
{
    StudentStat student_stats[MAX_STAT_ITEMS];
    int student_count = 0;
    int i;
    int idx;
    
    for (i = 0; i < count; i++) {
        idx = find_or_add_student(student_stats, &student_count,
                                   records[i].student_id, records[i].name);
        student_stats[idx].course_count++;
        student_stats[idx].total_credit += records[i].credit;
    }
    
    printf("\n========== 每位学生选课门数与总学分 ==========\n");
    printf("+--------------+----------+--------------+------------+\n");
    printf("| 学号         | 姓名    | 选课门数     | 总学分     |\n");
    printf("+--------------+----------+--------------+------------+\n");
    
    for (i = 0; i < student_count && i < 20; i++) {
        printf("| %-12s | %-8s | %-12d | %-10.1f |\n",
               student_stats[i].student_id,
               student_stats[i].name,
               student_stats[i].course_count,
               student_stats[i].total_credit);
    }
    
    if (student_count > 20) {
        printf("| ... (共 %d 位学生，仅显示前20位) ... |\n", student_count);
    }
    
    printf("+--------------+----------+--------------+------------+\n");
    printf("共 %d 位学生\n", student_count);
}

/* 内部：查找或添加学院统计项 */
static int find_or_add_college(CollegeStat *stats, int *count, const char *college)
{
    int i;
    
    for (i = 0; i < *count; i++) {
        if (strcmp(stats[i].college, college) == 0) {
            return i;
        }
    }
    
    strcpy(stats[*count].college, college);
    stats[*count].student_count = 0;
    (*count)++;
    
    return *count - 1;
}

/* 统计3：各学院选课人数分布 */
void stat_college_distribution(const Record *records, int count)
{
    CollegeStat college_stats[MAX_STAT_ITEMS];
    int college_count = 0;
    int i;
    int idx;
    
    for (i = 0; i < count; i++) {
        idx = find_or_add_college(college_stats, &college_count, records[i].college);
        college_stats[idx].student_count++;
    }
    
    printf("\n========== 各学院选课人数分布 ==========\n");
    printf("+------------------------+--------------+\n");
    printf("| 学院                   | 选课人数     |\n");
    printf("+------------------------+--------------+\n");
    
    for (i = 0; i < college_count; i++) {
        printf("| %-22s | %-12d |\n",
               college_stats[i].college,
               college_stats[i].student_count);
    }
    
    printf("+------------------------+--------------+\n");
    printf("共 %d 个学院\n", college_count);
}

/* 成绩分布统计 - 直接实现无需辅助函数 */
void stat_semester_distribution(const Record *records, int count)
{
    SemesterStat semester_stats[MAX_STAT_ITEMS];
    int sem_count = 0;
    int i, j;
    
    for (i = 0; i < count; i++) {
        /* 查找或添加学期 */
        int found = 0;
        for (j = 0; j < sem_count; j++) {
            if (strcmp(semester_stats[j].semester, records[i].semester) == 0) {
                semester_stats[j].student_count++;
                found = 1;
                break;
            }
        }
        
        if (!found) {
            strcpy(semester_stats[sem_count].semester, records[i].semester);
            semester_stats[sem_count].student_count = 1;
            semester_stats[sem_count].course_count = 0;
            sem_count++;
        }
    }
    
    /* 计算每学期不同课程数 */
    for (i = 0; i < sem_count; i++) {
        int course_seen = 0;
        char seen_courses[MAX_STAT_ITEMS][MAX_COURSE_ID_LEN];
        
        for (j = 0; j < count; j++) {
            if (strcmp(records[j].semester, semester_stats[i].semester) == 0) {
                int k;
                int already = 0;
                
                for (k = 0; k < course_seen; k++) {
                    if (strcmp(seen_courses[k], records[j].course_id) == 0) {
                        already = 1;
                        break;
                    }
                }
                
                if (!already) {
                    strcpy(seen_courses[course_seen], records[j].course_id);
                    course_seen++;
                }
            }
        }
        
        semester_stats[i].course_count = course_seen;
    }
    
    printf("\n========== 按学期统计选课人数与课程数分布 ==========\n");
    printf("+----------+------------------+------------------+\n");
    printf("| 学期     | 选课总人数       | 课程数           |\n");
    printf("+----------+------------------+------------------+\n");
    
    for (i = 0; i < sem_count; i++) {
        printf("| %-8s | %-16d | %-16d |\n",
               semester_stats[i].semester,
               semester_stats[i].student_count,
               semester_stats[i].course_count);
    }
    
    printf("+----------+------------------+------------------+\n");
    printf("共 %d 个学期\n", sem_count);
}

/* 统计5：课程成绩分布统计 */
void stat_score_distribution(const Record *records, int count)
{
    ScoreDistStat stat;
    int i;
    
    stat.excellent_count = 0;  /* 90-100 */
    stat.good_count = 0;       /* 80-89 */
    stat.medium_count = 0;     /* 70-79 */
    stat.pass_count = 0;       /* 60-69 */
    stat.fail_count = 0;       /* 0-59 */
    stat.total_count = count;
    
    for (i = 0; i < count; i++) {
        int score = records[i].score;
        
        if (score >= 90) {
            stat.excellent_count++;
        } else if (score >= 80) {
            stat.good_count++;
        } else if (score >= 70) {
            stat.medium_count++;
        } else if (score >= 60) {
            stat.pass_count++;
        } else {
            stat.fail_count++;
        }
    }
    
    printf("\n========== 成绩分布统计 ==========\n");
    printf("+------------+--------------+----------------+\n");
    printf("| 等级       | 分数段       | 人数           |\n");
    printf("+------------+--------------+----------------+\n");
    printf("| 优秀       | 90-100       | %-14d |\n", stat.excellent_count);
    printf("| 良好       | 80-89        | %-14d |\n", stat.good_count);
    printf("| 中等       | 70-79        | %-14d |\n", stat.medium_count);
    printf("| 及格       | 60-69        | %-14d |\n", stat.pass_count);
    printf("| 不及格     | 0-59         | %-14d |\n", stat.fail_count);
    printf("+------------+--------------+----------------+\n");
    printf("| 合计       |              | %-14d |\n", stat.total_count);
    printf("+------------+--------------+----------------+\n");
    
    if (stat.total_count > 0) {
        printf("优秀率: %.1f%%\n", 
               (float)stat.excellent_count / stat.total_count * 100.0f);
        printf("不及格率: %.1f%%\n", 
               (float)stat.fail_count / stat.total_count * 100.0f);
    }
}
