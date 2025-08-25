#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

void extract(char* file) {
    char pm[16] = {0};    
    
    int sz_check = system("7z </dev/null 2>/dev/null");
    printf("\e[1;1H\e[2J");
    
    if (WEXITSTATUS(sz_check) == 127) {
        printf("7Zip not found...\nInstalling...");
        
        if (access("/etc/debian_version", F_OK) == 0) strcpy(pm, "apt-get");
        else if (access("/etc/redhat-release", F_OK) == 0) strcpy(pm, "yum");
        else if (access("/etc/arch-release", F_OK) == 0) strcpy(pm, "pacman");
        else if (access("/etc/gentoo-release", F_OK) == 0) strcpy(pm, "emerge");
        else if (access("/etc/SuSE-release", F_OK) == 0) strcpy(pm, "zypper");
        else if (access("/etc/alpine-release", F_OK) == 0) strcpy(pm, "apk");
        else {
            printf("\n[ERR] Unknown package manager!\n");
            exit(EXIT_FAILURE);
        }
        
        printf("\nActive package manager : %s\n", pm);

        if (strcmp(pm, "apt-get") == 0) {
            system("sudo apt-get update && sudo apt-get install -y p7zip-full");
        } else if (strcmp(pm, "yum") == 0) {
            system("sudo yum install -y p7zip");
        } else if (strcmp(pm, "emerge") == 0) {
            system("sudo emerge --ask app-arch/p7zip");
        } else if (strcmp(pm, "zypper") == 0) {
            system("sudo zypper install -y p7zip");
        } else if (strcmp(pm, "pacman") == 0) {
            system("sudo pacman -S --noconfirm p7zip");
        } else if (strcmp(pm, "apk") == 0) {
            system("sudo apk add p7zip");
        }
    } 

    char cmd[512];
    
    system("rm -rf extracted");
    
    if (mkdir("extracted", 0755) == -1) {
        perror("[ERR] Failed to create extracted directory");
        exit(EXIT_FAILURE);
    }
    
    snprintf(cmd, sizeof(cmd), "7z x -o\"extracted\" \"%s\"", file);
    printf("Executing: %s\n", cmd);
    if (system(cmd) != 0) {
        printf("[ERR] Failed to extract package\n");
        exit(EXIT_FAILURE);
    }

    struct dirent *de;
    DIR *dr = opendir("./extracted");
    if (dr == NULL) { 
        printf("[ERR] Could not open extracted directory\n"); 
        exit(EXIT_FAILURE); 
    }

    char found_file[512] = {0};
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, "data.tar") == 0 || 
            strcmp(de->d_name, "data.tar.gz") == 0 ||
            strcmp(de->d_name, "data.tar.xz") == 0 ||
            strcmp(de->d_name, "data.tar.bz2") == 0 ||
            strstr(de->d_name, ".cpio") != NULL) {
            snprintf(found_file, sizeof(found_file), "extracted/%s", de->d_name);
            break;
        }  
    }
    closedir(dr);
    
    if (strlen(found_file) > 0) {
        printf("Found data file: %s\n", found_file);
        
        if (strstr(found_file, "data.tar")) {
            snprintf(cmd, sizeof(cmd), "tar -xf \"%s\" -C ./extracted", found_file);
        } else if (strstr(found_file, ".cpio")) {
            snprintf(cmd, sizeof(cmd), "cd ./extracted && cpio -idmv < \"%s\"", 
                    found_file + strlen("extracted/"));
        }
        
        printf("Executing: %s\n", cmd);
        if (system(cmd) != 0) {
            printf("[ERR] Failed to extract data archive\n");
            exit(EXIT_FAILURE);
        }
        
        remove(found_file);
    } else {
        printf("[WARN] No data archive found in package\n");
    }

    printf("Extraction complete!\n");       
}
