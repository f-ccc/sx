/*
 * stat.h - 统计分析接口
 * 功能：提供数据统计分析功能
 */

#ifndef STAT_H
#define STAT_H

#include "record.h"

/* 统计结果结构体 */

/* 课程统计 */
typedef struct {
    char course_id[MAX_COURSE_ID_LEN];
    char course_name[MAX_COURSE_NAME_LEN];
    int student_count;          /* 选课人数 */
    int capacity;               /* 容量 */
    float usage_rate;           /* 使用率 */
} CourseStat;

/* 学生统计 */
typedef struct {
    char student_id[MAX_STUDENT_ID_LEN];
    char name[MAX_NAME_LEN];
    int course_count;           /* 选课门数 */
    float total_credit;         /* 总学分 */
} StudentStat;

/* 学院统计 */
typedef struct {
    char college[MAX_COLLEGE_LEN];
    int student_count;          /* 选课人数 */
} CollegeStat;

/* 学期统计 */
typedef struct {
    char semester[MAX_SEMESTER_LEN];
    int student_count;          /* 选课总人数 */
    int course_count;           /* 课程数 */
} SemesterStat;

/* 成绩分布统计 */
typedef struct {
    int excellent_count;        /* 优秀 90-100 */
    int good_count;             /* 良好 80-89 */
    int medium_count;           /* 中等 70-79 */
    int pass_count;             /* 及格 60-69 */
    int fail_count;             /* 不及格 0-59 */
    int total_count;
} ScoreDistStat;

/* 统计分析函数 */
void stat_course_enrollment(const Record *records, int count);
void stat_student_courses(const Record *records, int count);
void stat_college_distribution(const Record *records, int count);
void stat_semester_distribution(const Record *records, int count);
void stat_score_distribution(const Record *records, int count);

#endif /* STAT_H */
