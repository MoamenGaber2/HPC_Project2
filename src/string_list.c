#include "string_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void list_init(StringList *list) {
    list->items = NULL;
    list->count = 0;
    list->cap = 0;
}

void list_push(StringList *list, const char *s) {
    if (list->count == list->cap) {
        int new_cap = list->cap == 0 ? 256 : list->cap * 2;
        char **new_items = (char **)realloc(list->items, (size_t)new_cap * sizeof(char *));
        if (!new_items) {
            fprintf(stderr, "Out of memory while expanding file list.\n");
            exit(1);
        }
        list->items = new_items;
        list->cap = new_cap;
    }
    size_t n = strlen(s);
    list->items[list->count] = (char *)malloc(n + 1);
    if (!list->items[list->count]) {
        fprintf(stderr, "Out of memory while storing file path.\n");
        exit(1);
    }
    memcpy(list->items[list->count], s, n + 1);
    list->count++;
}

void list_free(StringList *list) {
    for (int i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->cap = 0;
}
