#include "record.h"
#include <stdio.h>
#include <string.h>
int main(void) {
    int r;
    char buf[32];
    float f;
    
    r = validate_student_id("202401011234");
    printf("学号1(合法12位): %s\n", r==OK ? "通过" : "不过");
    
    r = validate_student_id("20240101123 ");
    printf("学号2(12位含空格): %s\n", r==OK ? "通过" : "不过");
    
    r = validate_student_id("20240101123");
    printf("学号3(11位): %s\n", r==OK ? "通过" : "不过");
    
    strcpy(buf, "3.5\n");
    f = (float)atof(buf);
    printf("atof学分(3.5\n): %s\n", (f>0&&f<=20) ? "通过" : "不过");
    
    strcpy(buf, " 3.5\n");
    f = (float)atof(buf);
    printf("atof学分(空格3.5\n): %s\n", (f>0&&f<=20) ? "通过" : "不过");
    
    strcpy(buf, "0.5\n");
    f = (float)atof(buf);
    printf("atof学分(0.5\n): %s\n", (f>0&&f<=20) ? "通过" : "不过");
    
    strcpy(buf, "20.0\n");
    f = (float)atof(buf);
    printf("atof学分(20.0\n): %s\n", (f>0&&f<=20) ? "通过" : "不过");
    
    strcpy(buf, "0.0\n");
    f = (float)atof(buf);
    printf("atof学分(0.0\n): %s\n", (f>0&&f<=20) ? "通过" : "不过");
    
    return 0;
}
