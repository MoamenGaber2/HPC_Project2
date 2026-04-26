#include "fs_utils.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#else
#include <dirent.h>
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#include "string_list.h"

static int ends_with_bin(const char *name) {
    size_t n = strlen(name);
    return n >= 4 && strcmp(name + n - 4, ".bin") == 0;
}

void scan_dir_recursive(const char *dir_path, StringList *out) {
#ifdef _WIN32
    char pattern[PATH_MAX];
    int n = snprintf(pattern, sizeof(pattern), "%s\\*", dir_path);
    if (n < 0 || n >= (int)sizeof(pattern)) {
        return;
    }

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
            continue;
        }

        char full[PATH_MAX];
        int written = snprintf(full, sizeof(full), "%s\\%s", dir_path, fd.cFileName);
        if (written < 0 || written >= (int)sizeof(full)) {
            continue;
        }

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            scan_dir_recursive(full, out);
        } else if (ends_with_bin(fd.cFileName)) {
            list_push(out, full);
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
#else
    DIR *d = opendir(dir_path);
    if (!d) {
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        char full[PATH_MAX];
        int written = snprintf(full, sizeof(full), "%s/%s", dir_path, ent->d_name);
        if (written < 0 || written >= (int)sizeof(full)) {
            continue;
        }
        struct stat st;
        if (stat(full, &st) != 0) {
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            scan_dir_recursive(full, out);
        } else if (S_ISREG(st.st_mode) && ends_with_bin(ent->d_name)) {
            list_push(out, full);
        }
    }
    closedir(d);
#endif
}

int cmp_str_ptr(const void *a, const void *b) {
    const char *sa = *(const char *const *)a;
    const char *sb = *(const char *const *)b;
    return strcmp(sa, sb);
}

int ensure_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
#ifdef _WIN32
        return (st.st_mode & _S_IFDIR) ? 0 : -1;
#else
        return S_ISDIR(st.st_mode) ? 0 : -1;
#endif
    }
    return MKDIR(path);
}

int make_parent_dirs(const char *file_path) {
    char tmp[PATH_MAX];
    size_t n = strlen(file_path);
    if (n >= sizeof(tmp)) {
        return -1;
    }
    memcpy(tmp, file_path, n + 1);
    for (size_t i = 1; i < n; i++) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            char keep = tmp[i];
            tmp[i] = '\0';
            if (strlen(tmp) > 0 && ensure_dir(tmp) != 0 && errno != EEXIST) {
                return -1;
            }
            tmp[i] = keep;
        }
    }
    return 0;
}

const char *base_name(const char *p) {
    const char *s1 = strrchr(p, '/');
    const char *s2 = strrchr(p, '\\');
    const char *s = s1 > s2 ? s1 : s2;
    return s ? s + 1 : p;
}
