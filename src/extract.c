#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

void extract(char* file) {

	char pm[16];

	int sz_check = system("7z </dev/null 2>/dev/null");
	printf("\e[1;1H\e[2J");
	if (WEXITSTATUS(sz_check)==127) {
		printf("7Zip not found...\nInstalling...");
		if (access("/etc/redhat-release", F_OK) == 0) strcpy(pm, "yum");
		if (access("/etc/arch-release", F_OK) == 0) strcpy(pm, "pacman");
		if (access("/etc/gentoo-release", F_OK) == 0) strcpy(pm, "emerge");
		if (access("/etc/SuSE-release", F_OK) == 0) strcpy(pm,"zypper");
		if (access("/etc/debian_version", F_OK) == 0) strcpy(pm, "apt-get");
		if (access("/etc/alpine-release", F_OK) == 0) strcpy(pm, "apk");
		printf("\nActive package manager : %s\n", pm);

		if (strcmp(pm, "apt-get") == 0) system("sudo apt-get install p7zip-full");
		if (strcmp(pm, "yum") == 0) system("sudo yum install p7zip");
		if (strcmp(pm, "emerge") == 0) system("sudo emerge --ask app-archp7zip");
		if (strcmp(pm, "zypper") == 0) system("sudo install p7zip");
		if (strcmp(pm, "pacman") == 0) system("sudo pacman -S 7zip");
		if (strcmp(pm, "apk") == 0) system("sudo apk add 7zip");
	} 

	char cmd[256];
	mkdir("extracted", 0755);
	snprintf(cmd, sizeof(cmd), "7z x -o\"extracted\" %s", file);
	system(cmd);

	struct dirent *de;

	DIR *dr = opendir("./extracted");
	if (dr == NULL) { printf("Could not open directory\n"); exit(EXIT_FAILURE); }

	char found_file[256] = {0};
	while ((de = readdir(dr)) != NULL) {
		if (strcmp(de->d_name, "data.tar") == 0 || strstr(de->d_name, ".cpio") != NULL) {
			snprintf(found_file, sizeof(found_file), "extracted/%s", de->d_name);
			break;
		}  
	}
	closedir(dr);
	
	if (strlen(found_file) > 0) {
		if (strstr(found_file, "data.tar")) {
			snprintf(cmd, sizeof(cmd), "tar -xvf %s -C ./extracted", found_file);
		} else {
			snprintf(cmd, sizeof(cmd), "cpio -D ./extracted -idmv < \"%s\"", found_file);
		}
		system(cmd);
	}

	printf("Extraction complete!\n");       

}

