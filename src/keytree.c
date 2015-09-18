
#include <string.h>
#include "common.h"
#include "keytree.h"


typedef struct _keynode {
    int avail; 
    xkb_keycode_t code;
} keynode_t;

struct _key_tree {
    keynode_t *nodes;
    struct xkb_keymap *keymap;
    xkb_keycode_t min;
    xkb_keycode_t max;
};

static void keytree_build 
        (key_tree_t *tree,
         xkb_keycode_t l,
         xkb_keycode_t r,
         int i) {
    xkb_keycode_t m;

    if (l == r) {
        tree->nodes[i].code = l;
        tree->nodes[i].avail = 0;
        return;
    }

    m = (l + r) / 2;
    keytree_build (tree, l, m, i*2);
    keytree_build (tree, m+1, r, i*2+1);

    tree->nodes[i].code = 0;
    tree->nodes[i].avail = 0; 
}

static uint32_t calc_nodes (uint32_t range) {
    uint32_t i;

    for (i = 1; i < (range*2); i <<= 1);

    return (i + 4);
}

key_tree_t* keytree_create (struct xkb_keymap *keymap) {
    uint32_t len;
    key_tree_t *tree = xmalloc (sizeof (key_tree_t));

    tree->keymap = xkb_keymap_ref (keymap);
    tree->min = xkb_keymap_min_keycode (keymap);
    tree->max = xkb_keymap_max_keycode (keymap);

    len = calc_nodes (tree->max - tree->min + 1);
    tree->nodes = xmalloc (sizeof(keynode_t) * len);

    keytree_build (tree, tree->min, tree->max, 1);

    return tree;
}

xkb_keycode_t keytree_get_top_key (key_tree_t *tree) {
    if (tree->nodes[1].avail)
        return tree->nodes[1].code;
    else
        return 0;
}

static void keytree_key_up (key_tree_t *tree,
        xkb_keycode_t l, xkb_keycode_t r,
        int i, xkb_keycode_t key) {
    xkb_keycode_t m;

    if (l == r) {
        tree->nodes[i].avail = 0;
        return;
    }

    m = (l + r) / 2;
    if (m < key) {
        keytree_key_up (tree, m+1, r, i*2+1, key);
        if (tree->nodes[i*2+1].avail)
            memcpy (&tree->nodes[i],
                &tree->nodes[i*2+1], sizeof (keynode_t));
        else
            memcpy (&tree->nodes[i],
                &tree->nodes[i*2], sizeof (keynode_t));
    }
    else {
        keytree_key_up (tree, l, m, i*2, key);

        if (tree->nodes[i*2].avail)
            memcpy (&tree->nodes[i],
                &tree->nodes[i*2], sizeof (keynode_t));
        else 
            memcpy (&tree->nodes[i],
                &tree->nodes[i*2+1], sizeof (keynode_t));
    }

}

static void keytree_key_down (
        key_tree_t *tree, 
        xkb_keycode_t l, 
        xkb_keycode_t r,
        int i, xkb_keycode_t key) {
    xkb_keycode_t m;

    while (l < r) {
        tree->nodes[i].avail = 1;
        tree->nodes[i].code = key;

        m = (l + r) / 2;
        if (m < key) {
            l = m + 1;
            i = i * 2 + 1;
        } else {
            r = m;
            i = i * 2;
        }
    }
    tree->nodes[i].avail = 1;

}

static void (*key_update_fn[2]) (
        key_tree_t *tree, 
        xkb_keycode_t l, 
        xkb_keycode_t r,
        int i, xkb_keycode_t key) = {
    keytree_key_up, keytree_key_down
};
        

void keytree_update_key (key_tree_t *tree,
        xkb_keycode_t key,
        enum xkb_key_direction d) {

    if (key > tree->max || key < tree->min)
        return;

    if (!(xkb_keymap_key_repeats (tree->keymap, key)))
        return;

    key_update_fn[d] (tree, tree->min, tree->max, 1, key);

}


void keytree_destroy (key_tree_t *tree) {
    xfree (tree->nodes);
    xkb_keymap_unref (tree->keymap);
    xfree (tree);
}
