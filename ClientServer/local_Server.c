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
 int reqCount = 0;
 
extern int errno;
int fd;
//error function
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *wThread(void* nSock);


typedef struct reqSer{
    int fD;
    char *fileName;
}reqSer;

reqSer oData[100]; 

int main(int argc, char *argv[])
{   
	//socket return variable
	int sockfd;
	
	//incoming socket 
	int newsockfd;
	
	//port number
	int portno;
	
	socklen_t client_len;
    	
	//initializing struct for server info for file and client server
	struct sockaddr_in serv, client;
    	
	int n;
    
	//checking if port number is provided
	if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    
	//creating file server socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
	//was socket successfully connected?
	if (sockfd < 0) 
       error("ERROR opening socket");
   
    //zeroing out parametr values in the struct
    bzero((char *) &serv, sizeof(serv));
    
	//setting port number
	portno = atoi(argv[1]);
    
	//contains a code for the address family. It should always be set to the symbolic constant AF_INET.
	serv.sin_family = AF_INET;
	
	//this field contains the IP address of the host
    serv.sin_addr.s_addr = INADDR_ANY;
	
	//setting the server port number
    serv.sin_port = htons(portno);
	
	//binds a socket to the current host
    if (bind(sockfd, (struct sockaddr *) &serv,
             sizeof(serv)) < 0) 
             error("ERROR on binding");
    
	//listening for any socket connection from a client
	listen(sockfd,5);
   
	//blocks process until client connects to the server
    client_len = sizeof(client);
 
	while ((newsockfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t*)&client_len))) {
		int *newsock = malloc(sizeof(int));
		*newsock = newsockfd;
		pthread_t thread;
		int rc =pthread_create(&thread, NULL, wThread, newsock);
		
		if (rc){
            printf("error: in pthread_create() \n");
            exit(-1);
        }
		pthread_detach(thread);
		 
	}
	if (newsockfd <0){
		error("Error on accept");
	}
	return 0;
}

void* wThread(void* newsock){
	
	//buffer to hold sent command from client
    char buffer1[256];
	
	//buffer to hold file descriptor from client
    char buffer2[10];
	
	char buff[256];
	bzero(buff,256);
	char bSize[10];
	int nSock = *((int *)newsock);
	
	//first reading the the instruction from client
	int n,m;
	n = read(nSock,buffer1,255);
	//printf("Read the message %s\n", buffer1);
	const char delim[] = ",";
	char *token;
	token = strtok(buffer1, delim);
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
	
	//printf("Got past tokenizing part\n");
	//printf("First token is %s\n", strArr[0]);
	//if the client requests an open 
	if (*strArr[0] == 'o'){

		printf("Entering %s\n", strArr[0]);
		int fd;
		char fileName[100];
		bzero(fileName, 100);
		strcpy(fileName,basename(strArr[1]));
		int openFlags;
		mode_t filePerms;
		openFlags = O_CREAT | O_RDWR;
		filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
		fd = open(fileName, openFlags, filePerms);
		oData[reqCount].fileName = fileName;
		//printf("File name in open is %s\n",fileName);
		oData[reqCount].fD = fd;
		
		
		if(fd <0){
			char cToStr[10];
			sprintf(cToStr,"%d %s %d" ,errno,",",fd);
			m = write(nSock,cToStr,sizeof(cToStr));
		
		}
		else{
			char cToStr[100];
			sprintf(cToStr,"%d %s %d" ,0,",",fd);
			m = write(nSock,cToStr,sizeof(cToStr));
			
		}
		free(newsock);
		reqCount++;
		return(0);
	}
	
	//does the client request a read
	else if (*strArr[0] == 'r'){
		printf("Entering %s\n", strArr[0]);
		int fd = atoi(strArr[1]);
		//printf("File Descriptor is %d\n", fd);
		size_t bytes = (size_t)(atoi(strArr[3]));
		printf("Number of bytes is %d\n", bytes);
		char buf[300];
		char fil[100];
		sprintf(fil, "%s", strArr[2]);
		printf("File is %s\n",strArr[2]);
		int openFlags = O_CREAT | O_RDWR;
		int k,m; 
		//k = open(fil, openFlags);
		m = read(fd,buf,bytes);
		printf("Number of bytes actually read is %d with total opens %d\n", m, reqCount);
		//printf("%s\n", buf);
		char outBuf[300];
		if (m<0){
			sprintf(outBuf,"%d %s %s %s %d" ,errno,",", buf,",", m);
			m = write(nSock,outBuf,sizeof(outBuf));
		}
		//printf("File content is %s\n", buf);
		else{
			printf("File content is %s %d\n", buf, m);
			sprintf(outBuf,"%d %s %s %s %d" ,0,",", buf,",", m);
			m = write(nSock,outBuf,sizeof(outBuf));
		}
				
		//clearing the socket for other connections
		free(newsock);
		return 0;
		
	}
	//sprintf(buffer,"%s %s %d %s %d %s %s", "w",".", fD, ".", byte, ".", buff);
	else if( *strArr[0] == 'w'){
		int m, n;
		int fd = atoi(strArr[1]);
		size_t bytes = (size_t)(atoi(strArr[2]));
		char buf[300];
		strcpy(buf, strArr[3]);
		m = write(fd, buf, bytes);
		
		char outBuf[300];
		if (m<0){
			sprintf(outBuf,"%d %s &d" ,errno,",", m);
			m = write(nSock,outBuf,sizeof(outBuf));
		}
		else{
			sprintf(outBuf,"%d %s &d" ,0,",", m);
			m = write(nSock,outBuf,sizeof(outBuf));
		}

		//clearing the socket for other connections
		free(newsock);
		return 0;
		
	}

}
