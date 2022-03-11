/**
 * @file    avltree.c
 * @author  Andrey Yuzhakov
 * @version 1.0
 *
 */

#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include "avltree.h"

static inline int imax(int a, int b) {
    return a > b ? a : b;
}

static struct avlnode* avltree_right_rotate(struct avlnode *p) {
    struct avlnode *left = p->l;
    p->l = left->r;
    left->r = p;
    p->height = imax(avltree_node_height(p->l), avltree_node_height(p->r)) + 1;
    left->height = imax(avltree_node_height(left->l), p->height) + 1;
    return left;
}

static struct avlnode* avltree_left_rotate(struct avlnode *p) {
    struct avlnode *right = p->r;
    p->r = right->l;
    right->l = p;
    p->height = imax(avltree_node_height(p->l), avltree_node_height(p->r)) + 1;
    right->height = imax(avltree_node_height(right->r), p->height) + 1;
    return right;
}

static struct avlnode* avltree_leftright_rotate(struct avlnode *p) {
    p->l = avltree_left_rotate(p->l);
    return avltree_right_rotate(p);
}

static struct avlnode* avltree_rightleft_rotate(struct avlnode *p) {
    p->r = avltree_right_rotate(p->r);
    return avltree_left_rotate(p);
}

static struct avlnode* avltree_node_add(struct avltree *tree, struct avlnode *p, struct avlnode *node) {
    if (!p) {
        tree->ref = node;
        tree->count += 1;
        node->l = node->r = NULL;
        node->height=0;
        return node;
    }
    int cmp = tree->cmpnode(p, node);
    if (cmp > 0) {
        p->l = avltree_node_add(tree, p->l, node);
        if (avltree_node_balance(p) > 1) {
            if (tree->cmpnode(p->l, node) > 0) {
                p = avltree_right_rotate(p);
            } else {
                p = avltree_leftright_rotate(p);
            }
        }
    } else if (cmp < 0) {
        p->r = avltree_node_add(tree, p->r, node);
        if (avltree_node_balance(p) < -1) {
            if (tree->cmpnode(p->r, node) < 0) {
                p = avltree_left_rotate(p);
            } else {
                p = avltree_rightleft_rotate(p);
            }
        }
    }
    p->height = imax(avltree_node_height(p->l), avltree_node_height(p->r)) + 1;
    return p;
}

/**
 * @brief Add node to the tree.
 * @return 0 on success, -1 if node exist in the tree
 */
int avltree_add(struct avltree *tree, struct avlnode *node) {
    tree->ref = NULL;
    tree->root = avltree_node_add(tree, tree->root, node);
    return tree->ref ? 0 : -1;
}

struct avlnode* avltree_find(struct avltree *tree, const void *key) {
    struct avlnode *c = tree->root;
    while (c) {
        int cmp = tree->cmpkey(c, key);
        if (cmp == 0) {
            break;
        }
        c = (cmp < 0) ? c->r : c->l;
    }
    return c;
}

static struct avlnode* avltree_node_del_node(struct avltree *tree, struct avlnode *p, struct avlnode *node) {
    if (!p)
        return p;
    int cmp = tree->cmpnode(p, node);
    if (cmp > 0) {
        p->l = avltree_node_del_node(tree, p->l, node);
    } else if (cmp < 0) {
        p->r = avltree_node_del_node(tree, p->r, node);
    } else {
        tree->ref = p;
        if (!p->r) {
            p = p->l;
        } else if (!p->l) {
            p = p->r;
        } else {
            struct avlnode *m = p->r;
            while(m->l) // Find smallest in the right
              m = m->l;
            m->r = avltree_node_del_node(tree, p->r, m);
            m->l = p->l;
            tree->ref = p;
            p = m;
        }
    }
    if (!p) return p;
    p->height = imax(avltree_node_height(p->l), avltree_node_height(p->r)) + 1;

    int balance = avltree_node_balance(p);
    if (balance > 1) {
        if (avltree_node_balance(p->l) >= 0)
           p = avltree_right_rotate(p);
        else
           p = avltree_leftright_rotate(p);
    } else if (balance < -1) {
        if (avltree_node_balance(p->r) <= 0)
          p = avltree_left_rotate(p);
        else
          p = avltree_rightleft_rotate(p);
    }
    return p;
}

struct avlnode* avltree_node_del(struct avltree *tree, struct avlnode *node) {
    tree->ref = NULL;
    tree->root = avltree_node_del_node(tree, tree->root, node);
    if (tree->ref)
        tree->count -= 1;
    return tree->ref;
}

static struct avlnode* avltree_node_del_key(struct avltree *tree, struct avlnode *p, const void *key) {
    if (!p)
        return p;
    int cmp = tree->cmpkey(p, key);
    if (cmp > 0) {
        p->l = avltree_node_del_key(tree, p->l, key);
    } else if (cmp < 0) {
        p->r = avltree_node_del_key(tree, p->r, key);
    } else {
        tree->ref = p;
        if (!p->r) {
            p = p->l;
        } else if (!p->l) {
            p = p->r;
        } else {
            struct avlnode *m = p->r;
            while(m->l) // Find smallest in the right
              m = m->l;
            m->r = avltree_node_del_node(tree, p->r, m);
            m->l = p->l;
            tree->ref = p;
            p = m;
        }
    }
    if (!p) return p;
    p->height = imax(avltree_node_height(p->l), avltree_node_height(p->r)) + 1;

    int balance = avltree_node_balance(p);
    if (balance > 1) {
        if (avltree_node_balance(p->l) >= 0)
           p = avltree_right_rotate(p);
        else
           p = avltree_leftright_rotate(p);
    } else if (balance < -1) {
        if (avltree_node_balance(p->r) <= 0)
          p = avltree_left_rotate(p);
        else
          p = avltree_rightleft_rotate(p);
    }
    return p;
}

struct avlnode* avltree_del(struct avltree *tree, const void *key) {
    tree->ref = NULL;
    tree->root = avltree_node_del_key(tree, tree->root, key);
    if (tree->ref)
        tree->count -= 1;
    return tree->ref;
}

static void avltree_node_free(struct avltree *tree, struct avlnode *c) {
    if (c) {
        if (c->l) {
            avltree_node_free(tree, c->l);
        }
        if (c->r) {
            avltree_node_free(tree, c->r);
        }
        tree->freenode(c);
    }
}

void avltree_free(struct avltree *tree) {
    avltree_node_free(tree, tree->root);
    tree->root = NULL;
}

static struct avlnode* avltree_node_inorder(struct avltree *tree, struct avlnode *p,
                                        avlnode_callback_fn cb, void *ptr)
{
    if (p) {
        struct avlnode *n, *c = p;
        if (c->l) {
            if((n = avltree_node_inorder(tree, c->l, cb, ptr)))
                return n;
        }
        if (cb(c, ptr)) {
            return p;
        }
        if (c->r) {
           if ((n = avltree_node_inorder(tree, c->r, cb, ptr)))
               return n;
        }
    }
    return NULL;
}

struct avlnode* avltree_inorder(struct avltree *tree, avlnode_callback_fn cb, void *ptr)
{
    return tree->root ? avltree_node_inorder(tree, tree->root, cb, ptr) : NULL;
}

static struct avlnode* avltree_node_preorder(struct avltree *tree, struct avlnode *p,
                                        avlnode_callback_fn cb, void *ptr)
{
    if (p) {
        struct avlnode *n, *c = p;
        if (cb(c, ptr)) {
            return p;
        }
        if (c->l) {
            if((n = avltree_node_preorder(tree, c->l, cb, ptr)))
                return n;
        }
        if (c->r) {
           if ((n = avltree_node_preorder(tree, c->r, cb, ptr)))
               return n;
        }
    }
    return NULL;
}

struct avlnode* avltree_preorder(struct avltree *tree, avlnode_callback_fn cb, void *ptr)
{
    return tree->root ? avltree_node_preorder(tree, tree->root, cb, ptr) : NULL;
}

static struct avlnode* avltree_node_revorder(struct avltree *tree, struct avlnode *p,
                                        avlnode_callback_fn cb, void *ptr)
{
    if (p) {
        struct avlnode *n, *c = p;
        if (c->r) {
           if ((n = avltree_node_revorder(tree, c->r, cb, ptr)))
               return n;
        }
        if (cb(c, ptr)) {
            return p;
        }
        if (c->l) {
            if((n = avltree_node_revorder(tree, c->l, cb, ptr)))
                return n;
        }
    }
    return NULL;
}

struct avlnode* avltree_revorder(struct avltree *tree, avlnode_callback_fn cb, void *ptr)
{
    return tree->root ? avltree_node_revorder(tree, tree->root, cb, ptr) : NULL;
}

/**
 * @brief Day-Stout-Warren algorithm
 */

void avlnode_to_list(struct avlnode *root, size_t *size) {
    struct avlnode *tail = root;
    struct avlnode *rest = tail->r;
    while(rest) {
        if (!rest->l) {
            tail = rest;
            rest->height = 0;
            rest = rest->r;
            *size += 1;
        } else {
            struct avlnode *temp = rest->l;
            rest->l = temp->r;
            temp->r = rest;
            rest = temp;
            tail->r = temp;
        }
    }
}

static void compress(struct avlnode *root, size_t size) {
    struct avlnode *scan = root;
    for (size_t i = 0; i < size; i++) {
        struct avlnode *chld = scan->r;
        scan->r = chld->r;
        scan = scan->r;
        chld->r = scan->l;
        scan->l = chld;
        scan->height = imax(avltree_node_height(scan->l),
                            avltree_node_height(scan->r)) + 1;
    }
}

void avlnode_from_list(struct avlnode *root, size_t size) {
    size_t pow = log2(size + 1);
    size_t leaves = size + 1 - (1 << pow);
    compress(root, leaves);
    size = size - leaves;
    while (size > 1) {
        compress(root, size/2);
        size = size / 2;
    }
}

void avltree_balance(struct avltree *tree) {
    if (tree->root) {
        struct avlnode root = {.l = NULL, .r = tree->root};
        size_t size = 0;
        avlnode_to_list(&root, &size);
        avlnode_from_list(&root, size);
        tree->root = root.r;
    }
}
