#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <ctype.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>

#define h_addr h_addr_list[0]
#define maxEntries (100)
extern int errno;

//prototype for the function which will generate threads based on user command request
int netOpen(const char* pathname, int flags);
int netserverinit(char* hostname);
ssize_t netRead(int filds, void *buf, size_t nbyte);
ssize_t netWrite(int fildes, const void *buf, size_t nbyte);


//global so all functions can access the host name entered by user
char host[100];

//counter for number of entries made
int reqCount = 0;

//declaring a struct which holds data needed for netFile calls
typedef struct reqInfo{
    int fD;
    int sockFD;
    const char* filename;
}reqInfo;

//initializing an array of data entries
reqInfo request[100];
//struct reqInfo *request = malloc( sizeof(reqInfo)*100);
//error function
void error(const char *msg)
{
    perror(msg);
    exit(0);
} 

int main(int argc, char **argv){
    //If the user did not enter two arguments call an error and end the program
    if(argc != 3){
        printf("Error: There are not 2 arguments.\n");
        return -1;
    }
	char buff[100];
	strcpy(host, argv[1]);
	int test = netOpen("/ilab/users/sr1163/test_server/t3.c", 0700);
	int test2 = netOpen("/ilab/users/sr1163/test_server/t2.txt", 0700);
	int test3 = netOpen("/ilab/users/sr1163/test_server/t1.tt", 0700);
	printf("New FD= %d\n", test);
	printf("New FD= %d\n", test2);
	printf("New FD= %d\n", test3);
	ssize_t test4 = netRead(test, buff, 7);
	printf("The Buff no is %s\n", buff);
	ssize_t test5 = netWrite(test2, "Hellooooo", 10);
	printf("Num of bytes read = %d\n", test4);
	printf("Num of bytes read = %d\n", test5);
	int i;
	for (i=0;i<reqCount;i++){
		printf("File name and descrip for Open# %d is %d %s\n", i, request[i].fD, request[i].filename);
	}
	//printf("reCount is %d\n", reqCount);
	return 0;
}

int netserverinit(char * hostname){
	
	struct hostent *server;
	server = gethostbyname(hostname);
    
	//does the server not exist?
	if (server == NULL) {
		printf("Host: %s does not exist\n", host);
		return -1;
	}
	//socket return variable
	int sockfd;
	
	//port number return variable
	int portno;

	//used for writing messages to the file server
	int n;
	
	//structs needed to keep track of ip addresses and server hostname verification
	struct sockaddr_in serv_addr;
    //struct hostent *server;
	
	//buffer which will hold the command which is to be sent to the file server
    char buffer[1];
	
	//buffer that will hold the pathname to be sent to the file server
	char buffer2[256];
	
	//port number is set to 8557
    portno = 8557;
	
	//create a new socket FD
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
		error("ERROR opening socket");
	
	//port number is set to 8192
	portno = 8558;
	
	//setting the servers hostname
	server = gethostbyname(host);	
	
	//zero out the fields within the serv_addr struct
	bzero((char *) &serv_addr, sizeof(serv_addr));
    
	//contains a code for the address family. It should always be set to the symbolic constant AF_INET.
	serv_addr.sin_family = AF_INET;
	
	
	bcopy((char *)server->h_addr, 
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
    
	//setting the server address port number
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
	
	return sockfd;
}

int netOpen(const char* pathname, int flags){
	
	int n;
	
	//buffer which will hold the command which is to be sent to the file server
    char buffer[300];
	printf("Hot into netopen\n");
	
	//holds the pathname
	char path[100];
	char * p = (char*)pathname; 
	strcpy(path, p);
	
	//socket return variable
	int sockfd;
	
	sockfd=  netserverinit(host);
	
	sprintf(buffer, "%s %s %s", "o",",", path);
	n = write(sockfd,buffer,sizeof(buffer));
	
	//printf("Wrote to server\n");
	
	//pathname not sent to the socket?
	if (n < 0) 
		error("ERROR writing from socket");
		
	//holds the error sent from fileserver
	char inBuf[256];
	
	//reads the error from fileserver
	n = read(sockfd,inBuf,256);
	//printf("Read into buf\n");	
	//was error properly sent?
	if (n < 0) 
		error("ERROR writing from socket");
		
	const char delim[] = ",";
	char *token;
	token = strtok(inBuf, delim);
	int strIndex = 0;
    char **strArr;
    strArr = (char**)malloc(sizeof(char*));
    strArr[strIndex] = (char*)malloc(sizeof(char)*strlen(token));
    strArr[strIndex]=token;
    token = strtok(NULL,delim);
	while(token!=NULL ){
        strIndex++;
        strArr[strIndex]=(char*)malloc(sizeof(char)*strlen(token));
        strArr[strIndex]=token;
        token = strtok(NULL,delim);
    }
	//printf("Tokenized\n");
	int ern;
	errno = atoi(strArr[0]);
	int fD = atoi(strArr[1]);
	char *fileN = malloc(100);
	bzero(fileN,100);
	sprintf(fileN, "%s", basename(path));
	//printf("Fils from fileN are %s\n", fileN);
	request[reqCount].fD = fD;
	request[reqCount].filename = fileN;
	//puts("works here 2");
	reqCount++;
	//if there is an error
	if (errno !=0){
		if (ern == ENOENT) 
			printf ("%s does not exist errno %d\n", path, ern);
			
		else if (errno == EACCES) 
			printf ("%s is not accessible errno %d\n", path, ern);
			
		else if (errno == EROFS)
			printf ("%s is not writable (read-only filesystem) errno %d\n", path, ern);
		return -1;
	}
	else{
		return fD;
	}	
}


ssize_t netRead(int filds, void *buf, size_t nbyte){
	//socket return variable
	int sockfd, n;
	
	sockfd=  netserverinit(host);
	
	int fD = (int)filds;
	char fil[100];
	int bSize = sizeof((char*)(buf));
	if (fD ==-1)
		return -1;
	int i;
	for (i=0;i<reqCount;i++){
		//printf("File name and descrip for Open# %d is %d %s\n", i, request[i].fD, request[i].filename);
		if (fD = request[i].fD){
			sprintf(fil, "%s", request[i].filename);
			break;
		}
			
	}
	printf("File to be read is %s\n", fil);
	int byte = (int)(nbyte);
	
	printf("Byte size from client size is %d\n", byte);
	
	//buffer which will hold the command which is to be sent to the file server
    char buffer[300];
	
	sprintf(buffer, "%s %s %d %s %s %s %d", "r",",", fD, ",", fil, ",", byte);
	n = write(sockfd,buffer,sizeof(buffer));
	
	//pathname not sent to the socket?
	if (n < 0) 
		error("ERROR writing from socket");
	
	//holds the error sent from fileserver
	char inBuf[256];
	
	//reads the error from fileserver
	n = read(sockfd,inBuf,256);
		
	const char delim[] = ",";
	char *token;
	token = strtok(inBuf, delim);
	int strIndex = 0;
    char **strArr;
    strArr = (char**)malloc(sizeof(char*));
    strArr[strIndex] = (char*)malloc(sizeof(char)*strlen(token));
    strArr[strIndex]=token;
    token = strtok(NULL,delim);
	while(token!=NULL ){
        strIndex++;
        strArr[strIndex]=(char*)malloc(sizeof(char)*strlen(token));
        strArr[strIndex]=token;
        token = strtok(NULL,delim);
    }
	int ern;
	errno = atoi(strArr[0]);
	size_t nBytes = (size_t)(atoi(strArr[2]));
	if (errno !=0){
		if (errno == ETIMEDOUT) 
			printf ("File times out %d\n", errno);
			
		else if (errno == EBADF) 
			printf ("EBADF error %d\n", errno);
			
		else if (errno == ECONNRESET)
			printf ("ECONNRESET %d\n",errno);
		return nBytes;
	}
	else{
		strcpy(buf, strArr[1]);
		return nBytes;
	}	

}

ssize_t netWrite(int fildes, const void *buf, size_t nbyte){
	int sockfd, n;
	
	sockfd=  netserverinit(host);
	
	int fD = (int)fildes;
	
	char * buff = (char*)buf;
	
	char buffer[300];
	
	int byte = (int)(nbyte);
	
	sprintf(buffer, "%s %s %d %s %d %s %s", "w",",", fD, ",", byte, ",", buff);
	n = write(sockfd,buffer,sizeof(buffer));
	
	//pathname not sent to the socket?
	if (n < 0) 
		error("ERROR writing from socket");
	char inBuf[300];
	
	n = read(sockfd,inBuf, sizeof(inBuf));
	//sprintf(outBuf,"%d %s &d" ,errno,".", m);
	
	const char delim[] = ",";
	char *token;
	token = strtok(inBuf, delim);
	int strIndex = 0;
    char **strArr;
    strArr = (char**)malloc(sizeof(char*));
    strArr[strIndex] = (char*)malloc(sizeof(char)*strlen(token));
    strArr[strIndex]=token;
    token = strtok(NULL,delim);
	while(token!=NULL ){
        strIndex++;
        strArr[strIndex]=(char*)malloc(sizeof(char)*strlen(token));
        strArr[strIndex]=token;
        token = strtok(NULL,delim);
    }
	size_t nBytes = (size_t)(atoi(strArr[1]));
	errno = atoi(strArr[0]);
	if (errno !=0){
		if (errno == ETIMEDOUT) 
			printf ("File times out %d\n", errno);
			
		else if (errno == EBADF) 
			printf ("EBADF error %d\n", errno);
			
		else if (errno == ECONNRESET)
			printf ("ECONNRESET %d\n",errno);
		return nBytes;
	}
	else{
		return nBytes;
	}	
	
}
/*
int netclose(int fd){ // connects to server of this fd to attempt to close remote file
    bzero(buffer,256);
    sprintf(buffer, "c,%d", fd);
    // printf("%s\n", buffer);
    n = write(sockFD,buffer,strlen(buffer));
    if (n < 0) {
         error("ERROR writing to socket");
    }
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0){
         error("ERROR reading from socket");
    }
    char * token = strtok(buffer, ",");
    if (!strcmp(token,"error")){
        errno = atoi(strtok(NULL, ","));
        error("netclose did not work");
        return -1; //close failed
    }
    return 0; //close success
}*/
