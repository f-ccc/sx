/*
 * avl.h - AVL树数据结构
 * 功能：定义AVL树节点及操作接口
 */

#ifndef AVL_H
#define AVL_H

#include "record.h"

/* AVL树节点 */
typedef struct AVLNode {
    Record data;                /* 数据域 */
    struct AVLNode *left;       /* 左子树 */
    struct AVLNode *right;      /* 右子树 */
    int height;                 /* 节点高度 */
} AVLNode;

/* AVL树结构体 */
typedef struct {
    AVLNode *root;              /* 根节点 */
    int size;                   /* 节点数量 */
} AVLTree;

/* 关键字提取函数类型 - 从Record提取用于比较的关键字 */
typedef int (*KeyComparator)(const Record *, const Record *);

/* 基本操作 */
void avl_init(AVLTree *tree);
int avl_insert(AVLTree *tree, const Record *rec);
int avl_delete(AVLTree *tree, const char *student_id, const char *course_id);
int avl_update(AVLTree *tree, const char *student_id, const char *course_id, const Record *new_rec);
AVLNode *avl_find(AVLTree *tree, const char *student_id, const char *course_id);
void avl_traverse_inorder(AVLTree *tree, void (*visit)(const Record *));
int avl_size(const AVLTree *tree);
int avl_is_empty(const AVLTree *tree);
void avl_clear(AVLTree *tree);

/* 批量操作 */
int avl_delete_expired(AVLTree *tree, int base_year, int base_month, int base_day, int *deleted_count);

/* 内部操作（外部也可用于性能测试） */
AVLNode *avl_create_node(const Record *rec);
int avl_node_height(AVLNode *node);
int avl_balance_factor(AVLNode *node);
AVLNode *avl_rotate_left(AVLNode *y);
AVLNode *avl_rotate_right(AVLNode *x);
AVLNode *avl_insert_node(AVLNode *node, const Record *rec, int *result);
AVLNode *avl_delete_node(AVLNode *node, const char *student_id, const char *course_id, int *result);
AVLNode *avl_find_node(AVLNode *node, const char *student_id, const char *course_id);
AVLNode *avl_min_value_node(AVLNode *node);

#endif /* AVL_H */
