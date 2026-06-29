/*
 * avl.c - AVL树实现
 * 功能：实现AVL平衡二叉树的各种操作
 * 说明：以学号+课程编号作为唯一键进行比较
 */

#include "avl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 初始化AVL树 */
void avl_init(AVLTree *tree)
{
    tree->root = NULL;
    tree->size = 0;
}

/* 创建AVL节点 */
AVLNode *avl_create_node(const Record *rec)
{
    AVLNode *node;
    
    node = (AVLNode *)malloc(sizeof(AVLNode));
    if (node == NULL) {
        return NULL;
    }
    
    node->data = *rec;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;  /* 新节点高度为1 */
    
    return node;
}

/* 获取节点高度 */
int avl_node_height(AVLNode *node)
{
    if (node == NULL) {
        return 0;
    }
    return node->height;
}

/* 获取两数中较大值 */
static int max_int(int a, int b)
{
    return (a > b) ? a : b;
}

/* 计算平衡因子 */
int avl_balance_factor(AVLNode *node)
{
    if (node == NULL) {
        return 0;
    }
    return avl_node_height(node->left) - avl_node_height(node->right);
}

/* 更新节点高度 */
static void avl_update_height(AVLNode *node)
{
    if (node != NULL) {
        node->height = 1 + max_int(avl_node_height(node->left), 
                                    avl_node_height(node->right));
    }
}

/* 右旋转 */
AVLNode *avl_rotate_right(AVLNode *y)
{
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    
    /* 旋转 */
    x->right = y;
    y->left = T2;
    
    /* 更新高度 */
    avl_update_height(y);
    avl_update_height(x);
    
    return x;
}

/* 左旋转 */
AVLNode *avl_rotate_left(AVLNode *x)
{
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    
    /* 旋转 */
    y->left = x;
    x->right = T2;
    
    /* 更新高度 */
    avl_update_height(x);
    avl_update_height(y);
    
    return y;
}

/* 比较两个记录的键（学号+课程编号） */
static int avl_key_compare(const Record *a, const Record *b)
{
    int cmp;
    
    cmp = strcmp(a->student_id, b->student_id);
    if (cmp != 0) return cmp;
    
    cmp = strcmp(a->course_id, b->course_id);
    return cmp;
}

/* 比较记录与学号+课程编号 */
static int avl_key_compare_str(const Record *rec, const char *student_id, const char *course_id)
{
    int cmp;
    
    cmp = strcmp(rec->student_id, student_id);
    if (cmp != 0) return cmp;
    
    cmp = strcmp(rec->course_id, course_id);
    return cmp;
}

/* 插入节点（递归） */
AVLNode *avl_insert_node(AVLNode *node, const Record *rec, int *result)
{
    int cmp;
    int balance;
    
    if (node == NULL) {
        /* 创建新节点 */
        AVLNode *new_node = avl_create_node(rec);
        if (new_node == NULL) {
            *result = ERR;
            return NULL;
        }
        *result = OK;
        return new_node;
    }
    
    cmp = avl_key_compare(rec, &node->data);
    
    if (cmp < 0) {
        node->left = avl_insert_node(node->left, rec, result);
    } else if (cmp > 0) {
        node->right = avl_insert_node(node->right, rec, result);
    } else {
        /* 已存在 */
        *result = DUPLICATE;
        return node;
    }
    
    /* 更新高度 */
    avl_update_height(node);
    
    /* 检查平衡 */
    balance = avl_balance_factor(node);
    
    /* 左左情况 */
    if (balance > 1 && avl_key_compare(rec, &node->left->data) < 0) {
        return avl_rotate_right(node);
    }
    
    /* 右右情况 */
    if (balance < -1 && avl_key_compare(rec, &node->right->data) > 0) {
        return avl_rotate_left(node);
    }
    
    /* 左右情况 */
    if (balance > 1 && avl_key_compare(rec, &node->left->data) > 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    
    /* 右左情况 */
    if (balance < -1 && avl_key_compare(rec, &node->right->data) < 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }
    
    return node;
}

/* 插入记录到AVL树 */
int avl_insert(AVLTree *tree, const Record *rec)
{
    int result = ERR;
    
    tree->root = avl_insert_node(tree->root, rec, &result);
    
    if (result == OK) {
        tree->size++;
    }
    
    return result;
}

/* 查找最小值节点 */
AVLNode *avl_min_value_node(AVLNode *node)
{
    AVLNode *current = node;
    
    while (current != NULL && current->left != NULL) {
        current = current->left;
    }
    
    return current;
}

/* 删除节点（递归） */
AVLNode *avl_delete_node(AVLNode *node, const char *student_id, const char *course_id, 
                         int *result)
{
    int cmp;
    int balance;
    
    if (node == NULL) {
        *result = NOT_FOUND;
        return NULL;
    }
    
    cmp = avl_key_compare_str(&node->data, student_id, course_id);
    
    if (cmp < 0) {
        node->left = avl_delete_node(node->left, student_id, course_id, result);
    } else if (cmp > 0) {
        node->right = avl_delete_node(node->right, student_id, course_id, result);
    } else {
        /* 找到要删除的节点 */
        AVLNode *temp;
        
        *result = OK;
        
        if (node->left == NULL || node->right == NULL) {
            /* 有一个或零个子节点 */
            temp = (node->left != NULL) ? node->left : node->right;
            
            if (temp == NULL) {
                /* 没有子节点 */
                free(node);
                return NULL;
            } else {
                /* 有一个子节点 */
                *node = *temp;  /* 复制子节点内容 */
                free(temp);
            }
        } else {
            /* 有两个子节点：找中序后继 */
            temp = avl_min_value_node(node->right);
            node->data = temp->data;
            node->right = avl_delete_node(node->right, temp->data.student_id, 
                                          temp->data.course_id, result);
        }
    }
    
    /* 更新高度 */
    avl_update_height(node);
    
    /* 检查平衡 */
    balance = avl_balance_factor(node);
    
    /* 左左情况 */
    if (balance > 1 && avl_balance_factor(node->left) >= 0) {
        return avl_rotate_right(node);
    }
    
    /* 左右情况 */
    if (balance > 1 && avl_balance_factor(node->left) < 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    
    /* 右右情况 */
    if (balance < -1 && avl_balance_factor(node->right) <= 0) {
        return avl_rotate_left(node);
    }
    
    /* 右左情况 */
    if (balance < -1 && avl_balance_factor(node->right) > 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }
    
    return node;
}

/* 从AVL树删除记录 */
int avl_delete(AVLTree *tree, const char *student_id, const char *course_id)
{
    int result = ERR;
    
    tree->root = avl_delete_node(tree->root, student_id, course_id, &result);
    
    if (result == OK) {
        tree->size--;
    }
    
    return result;
}

/* 更新记录 */
int avl_update(AVLTree *tree, const char *student_id, const char *course_id, 
               const Record *new_rec)
{
    AVLNode *node;
    
    node = avl_find(tree, student_id, course_id);
    if (node == NULL) {
        return NOT_FOUND;
    }
    
    /* 保留学号和课程编号，更新其他字段 */
    strcpy(node->data.name, new_rec->name);
    strcpy(node->data.college, new_rec->college);
    strcpy(node->data.course_name, new_rec->course_name);
    node->data.credit = new_rec->credit;
    strcpy(node->data.semester, new_rec->semester);
    node->data.enroll_date = new_rec->enroll_date;
    node->data.score = new_rec->score;
    
    return OK;
}

/* 查找节点 */
AVLNode *avl_find_node(AVLNode *node, const char *student_id, const char *course_id)
{
    int cmp;
    
    if (node == NULL) {
        return NULL;
    }
    
    cmp = avl_key_compare_str(&node->data, student_id, course_id);
    
    if (cmp == 0) {
        return node;
    } else if (cmp < 0) {
        return avl_find_node(node->left, student_id, course_id);
    } else {
        return avl_find_node(node->right, student_id, course_id);
    }
}

/* 查找记录 */
AVLNode *avl_find(AVLTree *tree, const char *student_id, const char *course_id)
{
    return avl_find_node(tree->root, student_id, course_id);
}

/* 中序遍历（递归） */
static void avl_inorder(AVLNode *node, void (*visit)(const Record *))
{
    if (node != NULL) {
        avl_inorder(node->left, visit);
        visit(&node->data);
        avl_inorder(node->right, visit);
    }
}

/* 中序遍历 */
void avl_traverse_inorder(AVLTree *tree, void (*visit)(const Record *))
{
    avl_inorder(tree->root, visit);
}

/* 获取大小 */
int avl_size(const AVLTree *tree)
{
    return tree->size;
}

/* 判断是否为空 */
int avl_is_empty(const AVLTree *tree)
{
    return (tree->root == NULL);
}

/* 清空树（递归） */
static void avl_clear_node(AVLNode *node)
{
    if (node != NULL) {
        avl_clear_node(node->left);
        avl_clear_node(node->right);
        free(node);
    }
}

/* 清空AVL树 */
void avl_clear(AVLTree *tree)
{
    avl_clear_node(tree->root);
    tree->root = NULL;
    tree->size = 0;
}

/* 删除过期记录（递归辅助） */
static AVLNode *avl_delete_expired_node(AVLNode *node, int base_year, int base_month, 
                                         int base_day, int *deleted_count)
{
    int balance;
    
    if (node == NULL) {
        return NULL;
    }
    
    /* 后序遍历删除 */
    node->left = avl_delete_expired_node(node->left, base_year, base_month, 
                                          base_day, deleted_count);
    node->right = avl_delete_expired_node(node->right, base_year, base_month, 
                                           base_day, deleted_count);
    
    /* 检查当前节点是否过期 */
    if (is_expired(&node->data, base_year, base_month, base_day)) {
        /* 删除此节点 */
        AVLNode *temp;
        int result;
        
        (*deleted_count)++;
        
        if (node->left == NULL || node->right == NULL) {
            temp = (node->left != NULL) ? node->left : node->right;
            if (temp == NULL) {
                free(node);
                return NULL;
            } else {
                *node = *temp;
                free(temp);
            }
        } else {
            temp = avl_min_value_node(node->right);
            node->data = temp->data;
            node->right = avl_delete_node(node->right, temp->data.student_id,
                                          temp->data.course_id, &result);
        }
        
        /* 重新平衡 */
        avl_update_height(node);
        balance = avl_balance_factor(node);
        
        if (balance > 1 && avl_balance_factor(node->left) >= 0) {
            return avl_rotate_right(node);
        }
        if (balance > 1 && avl_balance_factor(node->left) < 0) {
            node->left = avl_rotate_left(node->left);
            return avl_rotate_right(node);
        }
        if (balance < -1 && avl_balance_factor(node->right) <= 0) {
            return avl_rotate_left(node);
        }
        if (balance < -1 && avl_balance_factor(node->right) > 0) {
            node->right = avl_rotate_right(node->right);
            return avl_rotate_left(node);
        }
        
        return node;
    }
    
    return node;
}

/* 批量删除过期记录 */
int avl_delete_expired(AVLTree *tree, int base_year, int base_month, int base_day, 
                       int *deleted_count)
{
    *deleted_count = 0;
    tree->root = avl_delete_expired_node(tree->root, base_year, base_month, 
                                          base_day, deleted_count);
    tree->size -= *deleted_count;
    return OK;
}
