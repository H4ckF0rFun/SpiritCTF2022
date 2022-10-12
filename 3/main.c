#include<stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int strcmp(const char*s1,const char*s2){
	while(s1[0] == s2[0]){
		if(s1[0] == 0)
			return 0;
		s1++,s2++;
	}
	return s1[0] - s2[0];
}

char pass[0x100];

void __getpass(char*pass){
	FILE*fp = fopen("/pwn/pass","r");
	if(fp == NULL){
		puts("could not open pass");
		exit(0);
	}
	fgets(pass,0x100,fp);
	fclose(fp);
}


void initIO(){
	setbuf(stdin,NULL);
	setbuf(stdout,NULL);
	setbuf(stderr,NULL);
}

int main(){
	char Username[16];
	char Password[16];

	char pad[16] = {1};
	
	initIO();
	__getpass(pass);

	printf("Username:");
	Username[read(0,Username,16)] = 0;

	printf("Password:");
	Password[read(0,Password,16)] = 0;

	if(!strcmp(Username,"r00t") && !strcmp(Password,pass)){
		pad[0] = 0;
	}
	if(!pad[0]){
		puts("Login Success!");
		execve("/bin/bash",0,0);
	}
	puts("Login failed.");
	return 0;
}
