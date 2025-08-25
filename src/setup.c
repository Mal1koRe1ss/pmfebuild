#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "extract.h"

int ensure_directory_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("[ERR] mkdir error");
            return -1;
        }
    }
    return 0;
}

int is_same_file(const char* path1, const char* path2) {
    struct stat st1, st2;
    if (stat(path1, &st1) == -1) return 0;
    if (stat(path2, &st2) == -1) return 0;
    return (st1.st_ino == st2.st_ino) && (st1.st_dev == st2.st_dev);
}

int move_recursive(const char* source_root, const char* source_path, const char* target_root) {
    DIR* dp;
    struct dirent* entry;
    char src_full_path[1024], rel_path[1024], target_full_path[1024];

    if (strncmp(source_path, source_root, strlen(source_root)) == 0) {
        snprintf(rel_path, sizeof(rel_path), "%s", source_path + strlen(source_root));
    } else {
        fprintf(stderr, "[ERR] Error: source path not inside source root\n");
        return -1;
    }

    snprintf(target_full_path, sizeof(target_full_path), "%s%s", target_root, rel_path);

    struct stat statbuf;
    if (lstat(source_path, &statbuf) == -1) {
        perror("[ERR] lstat failed");
        return -1;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        if (strcmp(".", rel_path) == 0 || strcmp("..", rel_path) == 0 || strlen(rel_path) == 0) {
        } else {
            if (ensure_directory_exists(target_full_path) != 0) {
                return -1;
            }
        }
        if ((dp = opendir(source_path)) == NULL) {
            perror("[ERR] opendir failed");
            return -1;
        }
        while ((entry = readdir(dp)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(src_full_path, sizeof(src_full_path), "%s/%s", source_path, entry->d_name);
            if (move_recursive(source_root, src_full_path, target_root) != 0) {
                closedir(dp);
                return -1;
            }
        }
        closedir(dp);
        if (strcmp(source_root, source_path) != 0) {
            if (rmdir(source_path) != 0) { 
            }
        }
    } else {
        char target_dir[1024];
        strncpy(target_dir, target_full_path, sizeof(target_dir));
        char* last_slash = strrchr(target_dir, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
            ensure_directory_exists(target_dir);
        }

        if (is_same_file(source_path, target_full_path)) {
            fprintf(stderr, "[ERR] Skipping move: same file %s\n", source_path);
            return 0;
        }

        if (rename(source_path, target_full_path) != 0) {
            perror("[ERR] rename failed");
            return -1;
        }
        printf("Moved: %s -> %s\n", source_path, target_full_path);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char* source_folder = "./extracted";

    const char* root_dir = "/";

    extract(argv[1]);

    if (move_recursive(source_folder, source_folder, root_dir) != 0) {
        fprintf(stderr, "[ERR] Error occurred during move operation.\n");
        return EXIT_FAILURE;
    }

    printf("[SUC] Done moving files and directories.\n");
    return EXIT_SUCCESS;
}

