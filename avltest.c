/**
 * @file avltest.c
 * @author Andrey Yuzhakov
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <assert.h>
#include "avltree.h"

struct area {
  long addr;
  long size;
  struct avlnode node;
};

typedef int(*node_callback_fn)(const struct avlnode *n, void *ptr);

#define avlnode_to_area(node_ptr) \
    container_of(node_ptr, struct area, node);

void print_node_level(const struct avlnode *n, node_callback_fn print, int level) {
    if (n) {
        if (n->l)
          print_node_level(n->l, print, level + 1);
        for (int i = 0; i < level; i++)
            printf("-");
        print(n, NULL);
        if (n->r)
          print_node_level(n->r, print, level + 1);
    }
}

void  print_tree(const struct avltree *tree, node_callback_fn print) {
    if(tree->root)
        print_node_level(tree->root, print, 0);
    printf("------------------------\n");
}

int print_area_node(const struct avlnode *n, void *user) {
    struct area *area = avlnode_to_area(n);
    printf("Area addr=%ld height=%d\n", area->addr, area->node.height);
    return 0;
}

/**
 * @brief Compare two areas
 */
int area_cmp_node(const struct avlnode *n1, const struct avlnode *n2)
{
    struct area *a1 = avlnode_to_area(n1);
    struct area *a2 = avlnode_to_area(n2);
    return (a1->addr==a2->addr)?0:(a1->addr<a2->addr)?-1:1;
}

/**
 * @brief Compare area and a key
 */
int area_cmp_addr(const struct avlnode *n, const void *key)
{
    const struct area *area = avlnode_to_area(n);
    long addr = *((const long*)key);
    int rc = ((area->addr <= addr)&&(area->addr + area->size > addr))?0:(area->addr<addr)?-1:1;
    return rc;
}

void free_area_node(struct avlnode *node) {
    struct area *a = avlnode_to_area(node);
    free(a);
}

int check_height(struct avlnode *c) {
    if (c) {
        int dl = check_height(c->l);
        int dr = check_height(c->r);
        int diff = dl - dr;
        if (diff < 0) diff *= -1;
        assert(diff < 2);
        int m = (dl > dr) ? dl : dr;
        assert(c->height == m + 1);
        return c->height;
    }
    return -1;
}

int check_order(struct avlnode *n, void *ctx) {
    struct avlnode **p = ctx;
    if (*p) {
        const struct area *area = avlnode_to_area(n);
        const struct area *parea = avlnode_to_area(*p);
        assert(area->addr > parea->addr);
    }
    *p = n;
    return 0;
}

int check_rorder(struct avlnode *n, void *ctx) {
    struct avlnode **p = ctx;
    if (*p) {
        const struct area *area = avlnode_to_area(n);
        const struct area *parea = avlnode_to_area(*p);
        assert(area->addr < parea->addr);
    }
    *p = n;
    return 0;
}

int main(int argc, char **argv) {
    struct avltree tree;
    struct area *a;
    struct avlnode *n;
    int opt;
    long addr, count = 16;
    int i, iter = 1;
    int list = 0;

    avltree_init(&tree, area_cmp_addr, area_cmp_node, free_area_node);

    while ( (opt = getopt(argc, argv, "n:i:bl")) != -1 ) {
    switch ( opt ) {
        case 'n':
        count = atoi(optarg);
        break;
        case 'i':
        iter = atoi(optarg);
        break;
        case 'l':
        list = 1;
        break;
        case '?':
        default:
        printf("Usage: -n <count=16> -i <search-iter=0> -l [list]\n");
        return -1;
        }
    }

    // Add elements to the tree
    for (addr = count; addr > 0; addr--) {
        a = calloc(1, sizeof(*a));
        a->addr = addr;
        a->size=1;
        int rc = avltree_add(&tree, &a->node);
        assert(rc == 0);
    }

    if (list)
        print_tree(&tree, print_area_node);

    // Search
    for (i = 0; i < iter; i++) {
        for (addr = 1; 0 && addr < count; addr++) {
            struct avlnode *n = avltree_find(&tree, &addr);
            assert(n);
            a = avlnode_to_area(n);
            assert(a);
            assert(a->addr == addr);
        }
    }

    // Ordered delete/add
    for (addr = 1; addr <= count; addr++) {
        n = avltree_del(&tree, &addr);
        if (n) {
            a = avlnode_to_area(n);
            assert(a);
            assert(a->addr == addr);
            assert(check_height(tree.root) == tree.root->height);
            avltree_add(&tree, &a->node);
            assert(check_height(tree.root) == tree.root->height);
        }
    }
    if (list)
        print_tree(&tree, print_area_node);

    // Random delete/add
    for (i = 0; i < iter; i++) {
        do {
            addr = rand() % count;
        } while (!addr);
        n = avltree_find(&tree, &addr);
        assert(n);
        a = avlnode_to_area(n);
        assert(a->addr == addr);
        n = avltree_node_del(&tree, n);
        assert(n);
        if(n) {
            a = avlnode_to_area(n);
            assert(a);
            assert(a->addr == addr);
            assert(check_height(tree.root) == tree.root->height);
            avltree_add(&tree, &a->node);
            assert(check_height(tree.root) == tree.root->height);
        }
    }
    if (list)
        print_tree(&tree, print_area_node);
    
    // Test normal order
    n = NULL;
    avltree_inorder(&tree, check_order, &n);

    // Test reverce order
    n = NULL;
    avltree_revorder(&tree, check_rorder, &n);

    avltree_free(&tree);
    assert(!tree.root);
    return 0;
}
