#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<ctype.h>
#include <sys/types.h>
#include <sys/wait.h> 

//declaring a prototype file for the process compress function
void *compressR_LOLS(char *fileName, int parts);

//keeing the count of characters in the file global so it can be accessed throughout the code
int numberOfCharsInFile = 0;

//main function takes two arguments for this program: filename and the number of processes to be created  
int main (int argc, char *argv[]) {
	
	//if the argument count is anything but three, display an error message
	if(argc != 3){
        printf("Error: There are not 2 arguments.\n");
        return -1;
    }
    printf("Hellooooooooooo\n");
    //Atoi failure error checks if the file number entered it 0
    int parts;
    if((parts = atoi(argv[2])) == 0){
        printf("Error: User didn't enter a number for parts or it was 0.\n");
        return -1;
     }
     
	//Calling function which will check for errors and create multi processes
    compressR_LOLS(argv[1],parts);
    return 0;
}

//multi process function which will check for errors and fork processes inputed 
void *compressR_LOLS(char *fileName, int parts){
  	
	//Checks if the number of threads user wants is equal to or less than 0.
	if(parts <= 0){
        printf("Error: Parts <= 0.\n");
        return NULL;
    }
   
    //Check file has read access & exists in current directory
    if((access(fileName, F_OK)  && access(fileName, R_OK))!= -1){
        //File exists
    }
    else{
        //File doesn't exist
        printf("Error: File doesn't exist or you don't have read access.\n");
        return NULL;
    }
            
	//opening the file
	FILE *fp = fopen(fileName, "r");
		
	//for as long as we can get characters
	char c;
    while((c=fgetc(fp)) != EOF) {
		numberOfCharsInFile++;
	}
	numberOfCharsInFile--;
	  
	//Check if there is at least one character in the file
    if(numberOfCharsInFile == 0){
        printf("The file is empty.\n");
        return NULL;
    }
		
	//Checks if the the number of files to be compressed into is less than the number of characters in the file
    if(parts > numberOfCharsInFile){
        printf("The number of files to compress the original string exceeds the length of the original string.\n");
        return NULL;
    }
	
	//Assigning number of characters each thread should read into an array
	int i;
	int arrayNumChars[parts];
    for(i = 0; i < parts; i++){
        arrayNumChars[i] =  numberOfCharsInFile / parts ;
    }
    
    //divide up the remainder 
    for (i = 0; i <= (int)(numberOfCharsInFile % parts) - 1 && (int)(numberOfCharsInFile % parts) != 0;i++){
        arrayNumChars[i] += 1;
    }
   
    //array which holds all the starting points for each thread to compress
    int arrayStart[parts];
	
	//setting the first index starting point to zero since first thread starts compressing at 0-th character
	arrayStart[0] = 0;
   	
    //setting all the sarting points from which the thread should start executing
    for(i = 1; i < parts; i++){
        arrayStart[i] = arrayStart[i - 1] + arrayNumChars[i - 1];
    }
    
	//creating a file with length of fileName
	char fName[strlen(fileName)];
	
	strcpy(fName, fileName);
	
	const char delim[] = ".";
	
	//index variable for putting tokenized words into 2D array
    char *token;
	
	token = strtok(fName, delim);
	
	int strIndex = 0;

    //declaring a double pointer 2D array of type char
    char **strArr;

    //making the array strArr have a return type of type char**
    strArr = (char**)malloc(sizeof(char*));

    //allocating space in memory for the first tokenized word in the 2D array
    strArr[strIndex] = (char*)malloc(sizeof(char)*strlen(token));

    //copying the address of the tokenized word into the arrays index
    strArr[strIndex]=token;
     
    //iterating the token to go to the next word
    token = strtok(NULL,delim);
	while(token!=NULL ){
        //increment through the 2D array
        strIndex++;

        //allocating space in memory for the next tokenized word in the 2D array
        strArr[strIndex]=(char*)malloc(sizeof(char)*strlen(token));

        //copying the address of the tokenized word into the arrays index
        strArr[strIndex]=token;
        
        token = strtok(NULL,delim);
    }


	//creating number of processes user requests
    pid_t pid;

    for (i = 0; i < parts; i++) {
		
		//creates the process
        pid = fork();
		
		//if pid returns -1, process is not created properly
        if (pid == -1) {
            /* error handling here, if needed */
			perror("Creating Process Error");
            return NULL;
        }
		
		//pid returns 0 meaning process is created sucessfully
        if (pid ==0 ) {
			
			//creating all necessary data types each worker process will use
			char processID[4];
			sprintf(processID, "%d", i);
			char startingPoint[4];
			sprintf(startingPoint, "%d", arrayStart[i]); 
			char numCharsToRead[20];
			sprintf(numCharsToRead, "%d", arrayNumChars[i]);
			char amtOfProcess[4];
			sprintf(amtOfProcess, "%d", parts);
			char fileN[10];
			strcpy(fileN, fileName);
			char extension[10];
			strcpy(extension, strArr[1]);
			char file[10];
			strcpy(file, strArr[0]);
			char charsInF[4];
			sprintf(charsInF, "%d", numberOfCharsInFile);
			
			//creating an array of strings which includes the bytecode worker file, and all data types which are converted into char*
			char *args[] = {"./compressR_worker_LOLS",processID, startingPoint, numCharsToRead,
															amtOfProcess, fileN, extension, file, charsInF, NULL};
			
			//calling the worker file, and passing all the data types along with it
			execvp(args[0], args);
			
			//returns null after each process is complete
	        return NULL;
        }
    }
	 
	//waiting for each child worker process to finish
	for (i = 0; i < parts; i++) {
		wait(NULL);
	}
	return NULL;
}