#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>

#include "extract.h"

int ensure_directory_exists(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            perror("[ERR] mkdir error");
            return -1;
        }
        printf("Created directory: %s\n", path);
    }
    return 0;
}

int is_same_file(const char* path1, const char* path2) {
    struct stat st1, st2;
    if (stat(path1, &st1) == -1) return 0;
    if (stat(path2, &st2) == -1) return 0;
    return (st1.st_ino == st2.st_ino) && (st1.st_dev == st2.st_dev);
}

int create_directory_recursive(const char* path) {
    char tmp[1024];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) == -1 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, 0755) == -1 && errno != EEXIST) {
        return -1;
    }
    
    return 0;
}

int move_recursive(const char* source_root, const char* source_path, const char* target_root) {
    DIR* dp;
    struct dirent* entry;
    char src_full_path[1024], rel_path[1024], target_full_path[1024];

    if (strncmp(source_path, source_root, strlen(source_root)) == 0) {
        const char* rel_start = source_path + strlen(source_root);
        if (*rel_start == '/') rel_start++;
        snprintf(rel_path, sizeof(rel_path), "%s", rel_start);
    } else {
        fprintf(stderr, "[ERR] Error: source path not inside source root\n");
        return -1;
    }

    if (strlen(rel_path) == 0 || strcmp(rel_path, ".") == 0) {
        rel_path[0] = '\0';
    }

    if (strlen(rel_path) > 0) {
        snprintf(target_full_path, sizeof(target_full_path), "%s/%s", target_root, rel_path);
    } else {
        snprintf(target_full_path, sizeof(target_full_path), "%s", target_root);
    }

    struct stat statbuf;
    if (lstat(source_path, &statbuf) == -1) {
        perror("[ERR] lstat failed");
        return -1;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        if (strlen(rel_path) > 0 && strcmp(rel_path, ".") != 0 && strcmp(rel_path, "..") != 0) {
            if (create_directory_recursive(target_full_path) != 0) {
                fprintf(stderr, "[ERR] Failed to create directory: %s\n", target_full_path);
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
                printf("[WARN] Could not remove directory: %s\n", source_path);
            }
        }
    } else {
        char target_dir[1024];
        strncpy(target_dir, target_full_path, sizeof(target_dir) - 1);
        target_dir[sizeof(target_dir) - 1] = '\0';
        
        char* last_slash = strrchr(target_dir, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
            if (create_directory_recursive(target_dir) != 0) {
                fprintf(stderr, "[ERR] Failed to create parent directory: %s\n", target_dir);
                return -1;
            }
        }

        if (is_same_file(source_path, target_full_path)) {
            printf("[WARN] Skipping move: same file %s\n", source_path);
            return 0;
        }

        if (access(target_full_path, F_OK) == 0) {
            printf("[WARN] Target file exists, backing up: %s\n", target_full_path);
            char backup[1024];
            snprintf(backup, sizeof(backup), "%s.backup", target_full_path);
            rename(target_full_path, backup);
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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <package_file.deb|package_file.rpm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (access(argv[1], F_OK) == -1) {
        fprintf(stderr, "[ERR] File does not exist: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (getuid() != 0) {
        fprintf(stderr, "[WARN] Not running as root. File installation may fail.\n");
        printf("Consider running with sudo: sudo %s %s\n", argv[0], argv[1]);
    }

    const char* source_folder = "./extracted";
    const char* root_dir = "/";

    printf("[INFO] Extracting package: %s\n", argv[1]);
    extract(argv[1]);

    printf("[INFO] Installing files to system...\n");
    if (move_recursive(source_folder, source_folder, root_dir) != 0) {
        fprintf(stderr, "[ERR] Error occurred during installation.\n");
        return EXIT_FAILURE;
    }

    system("rm -rf extracted");

    printf("[SUCCESS] Package installation completed.\n");
    return EXIT_SUCCESS;
}
