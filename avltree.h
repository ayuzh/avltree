/**
 * @file    avltree.h
 * @author  Andrey Yuzhakov
 * @version 1.0
 *
 *   AVL tree
 */

#ifndef avl_tree_h
#define avl_tree_h

#ifndef offsetof
    #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

/**
 * AVL tree node
 */
struct avlnode {
  struct avlnode *l;
  struct avlnode *r;
  int height;
};

typedef int(*avlnode_cmpkey_fn)(const struct avlnode *n, const void *key);
typedef int(*avlnode_cmpnode_fn)(const struct avlnode *n1, const struct avlnode *n2);
typedef int(*avlnode_callback_fn)(struct avlnode *n, void *ptr);
typedef void(*avlnode_free_fn)(struct avlnode *n);

/**
 * AVL tree
 */
struct avltree {
  struct avlnode *root;
  avlnode_cmpnode_fn cmpnode;
  avlnode_cmpkey_fn  cmpkey;
  avlnode_free_fn    freenode;
  size_t count;
  struct avlnode *ref;
};

static inline void avlnode_init(struct avlnode *node) {
    node->l = node->r = 0;
    node->height = 0;
}

static inline int avltree_node_height(const struct avlnode *node) {
    return node ? node->height : -1;
}

static inline int avltree_node_balance(const struct avlnode *node) {
    return node ? avltree_node_height(node->l) - avltree_node_height(node->r) : 0;
}

static inline size_t avltree_count(struct avltree *tree) {
    return tree->count;
}

static inline int avltree_empty(struct avltree *tree) {
    return !tree->root;
}

/**
 * @brief Initialize tree structure
 * 
 * @param tree Tree
 * @param cmpkey Compare node-key
 * @param cmpnode Compare node-node
 * @param freenode Free node
 */
static inline void avltree_init(struct avltree *tree, avlnode_cmpkey_fn cmpkey,
                              avlnode_cmpnode_fn cmpnode, avlnode_free_fn freenode) {
    tree->root = 0;
    tree->count = 0;
    tree->cmpkey = cmpkey;
    tree->cmpnode = cmpnode;
    tree->freenode = freenode;
}

/**
 * @brief Add node to the tree
 */
int avltree_add(struct avltree *tree, struct avlnode *node);

/**
 * @brief Find a node for the given key
 */
struct avlnode* avltree_find(struct avltree *tree, const void *key);

/**
 * @brief Delete node
 */
struct avlnode* avltree_node_del(struct avltree *tree, struct avlnode *node);

/**
 * @brief Delete node for the given key
 */
struct avlnode* avltree_del(struct avltree *tree, const void *key);

/**
 * @brief Delete all nodes
 */
void avltree_free(struct avltree *tree);

/**
 * @brief Traverse tree in the order left-root-right.
 * Invoke callback for each node. If callback returns nonzero,
 * return address of the pointer to the node.
 */
struct avlnode* avltree_inorder(struct avltree *tree, avlnode_callback_fn cb, void *ptr);

/**
 * @brief Traverse tree in the order root-left-right.
 * Invoke callback for each node. If callback returns nonzero,
 * return address of the pointer to the node.
 */
struct avlnode* avltree_preorder(struct avltree *tree, avlnode_callback_fn cb, void *ptr);

/**
 * @brief Traverse tree in the order right-root-left.
 * Invoke callback for each node. If callback returns nonzero,
 * return address of the pointer to the node.
 */
struct avlnode* avltree_revorder(struct avltree *tree, avlnode_callback_fn cb, void *ptr);

/**
 * @brief Turn tree into a list
 */
void avlnode_to_list(struct avlnode *root, size_t *size);

/**
 * @brief Turn list into a tree
 */
void avlnode_from_list(struct avlnode *root, size_t size);

#endif
