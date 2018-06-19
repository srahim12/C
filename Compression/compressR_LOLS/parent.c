#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (int argc, char *argv[]) {
	/*if(argc != 3){
        printf("Error: There are not 2 arguments.\n");
        return -1;
    }
	int i;
	pid_t pid;
	//printf("Entering fork in fork\n");
    for (i = 0; i < 4; i++) {
        pid = fork();
        if (pid == -1) {
             //error handling here, if needed 
			perror("Creating Process Error");
            return 0;
        }
        if (pid == 0) {
			printf("Process created\n");
			char *args[] = {"./worker",processID, startingPoint, numCharsToRead,
															amtOfProcess, fileN, extension, file, charsInF,"Hello", NULL};
			execv("./worker", args);
			exit(1);
	        return 0;
		}
	}
	
	for (i = 0; i < 4; i++) {
		wait(NULL);
	}*/
	char *lol = "Ben hates his ex";
	char *args[] = {"./worker", lol,"Hello", NULL};
	execv(args[0], args);
	return 0;
}