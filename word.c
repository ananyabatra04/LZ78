#include "word.h"
#include "code.h"
#include <stdlib.h>

Word *word_create(uint8_t *syms, uint32_t len) {
    Word *word = calloc(1, sizeof(Word));
    if (word == NULL) {
        return NULL;
    }
    word->len = len;
    word->syms = (uint8_t *) calloc(word->len, sizeof(uint8_t));
    for (uint32_t i = 0; i < word->len; i++) {
        word->syms[i] = syms[i];
    }

    return word;
}

Word *word_append_sym(Word *w, uint8_t sym) {
    Word *new_word = calloc(1, sizeof(Word));
    new_word->len = w->len + 1;
    new_word->syms = (uint8_t *) calloc(new_word->len, sizeof(uint8_t));
    for (uint32_t i = 0; i < w->len; i++) {
        new_word->syms[i] = w->syms[i];
    }
    new_word->syms[w->len] = sym;
    return new_word;
}

void word_delete(Word *w) {
    if (w != NULL) {
        free(w->syms);
        w->syms = NULL;
        free(w);
    }
}

WordTable *wt_create(void) {
    WordTable *table = (WordTable *) calloc(MAX_CODE, sizeof(Word *));
    if (table == NULL) {
        return table;
    }
    table[EMPTY_CODE] = word_create(NULL, 0);
    return table;
}

void wt_reset(WordTable *wt) {
    for (int i = 0; i < MAX_CODE; i++) {
        if (i != EMPTY_CODE) {
            word_delete(wt[i]);
            wt[i] = NULL;
        }
    }
}

void wt_delete(WordTable *wt) {
    wt_reset(wt);
    word_delete(wt[EMPTY_CODE]);
    wt[EMPTY_CODE] = NULL;
    free(wt);
}
