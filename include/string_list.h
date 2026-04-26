#ifndef STRING_LIST_H
#define STRING_LIST_H

#include "common.h"

void list_init(StringList *list);
void list_push(StringList *list, const char *s);
void list_free(StringList *list);

#endif
