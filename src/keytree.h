
#ifndef _SELF_KEYTREE_H
#define _SELF_KEYTREE_H

#include <xkbcommon/xkbcommon.h>

typedef struct _key_tree key_tree_t;

key_tree_t* keytree_create (struct xkb_keymap *keymap);

xkb_keycode_t keytree_get_top_key (key_tree_t *tree);

void keytree_update_key (key_tree_t *tree,
        xkb_keycode_t key,
        enum xkb_key_direction d);

void keytree_destroy (key_tree_t *tree);


#endif
