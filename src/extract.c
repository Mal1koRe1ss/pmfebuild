#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>

void extract(FILE* file) {

	char pm[16] = {0};

	int 7z_check = system("7z </dev/null 2>/dev/null");
	if (WEXITSTATUS(7z_check)==127) {
		printf("7Zip not found...\nInstalling...");
		if (access("/etc/redhat-release", F_OK) == 0) pm = "yum";
		if (access("/etc/arch-release", F_OK) == 0) pm = "pacman";
		if (access("/etc/gentoo-release", F_OK) == 0) pm = "emerge";
		if (access("/etc/SuSE-release", F_OK) == 0) pm = "zypper";
		if (access("/etc/debian_version", F_OK) == 0) pm = "apt-get";
		if (access("/etc/alpine-release", F_OK) == 0) pm = "apk";
		printf("\nActive package manager : %s\n", pm);

		if (strcmp(pm, "apt-get") == 0) system("sudo apt-get install p7zip-full");
		if (strcmp(pm, "yum") == 0) system("sudo yum install p7zip");
		if (strcmp(pm, "emerge") == 0) system("sudo emerge --ask app-archp7zip");
		if (strcmp(pm, "zypper") == 0) system("sudo install p7zip");
		if (strcmp(pm, "pacman") == 0) system("sudo pacman -S 7zip");
		if (strcmp(pm, "apk") == 0) system("sudo apk add 7zip");

		break;
	} 

	char cmd[256];
	mkdir("extracted");
	system("mv %s ./extracted", file);

	struct dirent *de;

	DIR *dr = opendir(".");
	if (dr == NULL) { printf("Could not open directory\n"); exit(EXIT_FAILURE); }

	char found_file[256] = {0};
	while ((de = readdir(dr)) != NULL) {
		if (strcmp(de->d_name, "data.tar") == 0 || strstr(de->d_name, ".cpio") != NULL) {
			strcpy(found_file, de->d_name);
			break;
		}  
	}
	closedir(dr);
	
	(strlen(found_file) > 0) ? snprintf(cmd, sizeof(cmd), "tar -xvf \"%s\"", found_file) : snprintf(cmd, sizeof(cmd), "cpio -idmv < \"%s\"", found_file);
       system(cmd);

	printf("Extraction complete!\n");       

}

