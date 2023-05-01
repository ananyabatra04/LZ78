#include "code.h"
#include "trie.h"
#include <stdlib.h>

TrieNode *trie_node_create(uint16_t index) {
    TrieNode *node = calloc(1, sizeof(TrieNode));
    node->code = index;
    for (int i = 0; i < ALPHABET; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void trie_node_delete(TrieNode *n) {
    if (n != NULL) {
        free(n);
        n = NULL;
    }
}

TrieNode *trie_create(void) {
    TrieNode *root = trie_node_create(EMPTY_CODE);
    if (!root) {
        return NULL;
    }
    return root;
}

void trie_reset(TrieNode *root) {
    for (int i = 0; i < ALPHABET; i++) {
        trie_delete(root->children[i]);
        root->children[i] = NULL;
    }
}

void trie_delete(TrieNode *n) {
    if (n != NULL) {
        for (int i = 0; i < ALPHABET; i++) {
            trie_delete(n->children[i]);
        }
    }
    trie_node_delete(n);
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    return n->children[sym];
}
