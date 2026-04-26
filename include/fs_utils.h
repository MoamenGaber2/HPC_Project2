#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <limits.h>

#include "common.h"

void scan_dir_recursive(const char *dir_path, StringList *out);
int cmp_str_ptr(const void *a, const void *b);
int ensure_dir(const char *path);
int make_parent_dirs(const char *file_path);
const char *base_name(const char *p);

#endif
