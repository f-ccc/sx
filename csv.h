/*
 * csv.h - CSV文件读写接口
 * 功能：提供CSV格式的数据持久化功能
 */

#ifndef CSV_H
#define CSV_H

#include "record.h"

/* CSV文件读写 */
int csv_save_records(const char *filename, const Record *records, int count);
int csv_load_records(const char *filename, Record *records, int *count, int max_count);
int csv_save_records_from_list(const char *filename, const Record *records, int count);

#endif /* CSV_H */
