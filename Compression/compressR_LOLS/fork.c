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
		printf("Past the file access and readibility of the uncompressed file\n");
    }
    else{
        //File doesn't exist
        printf("Error: File doesn't exist or you don't have read access.\n");
        return NULL;
    }
            
	//opening the file
	FILE *fp = fopen(fileName, "r");
	
	printf("File was just opened\n");
	
	//for as long as we can get characters
	char c;
    while((c=fgetc(fp)) != EOF) {
		numberOfCharsInFile++;
	}
	numberOfCharsInFile--;
	  
	printf("Counted numberOfCharsInFile in fork\n");	
    
	//Check if there is at least one character in the file
    if(numberOfCharsInFile == 0){
        printf("The file is empty.\n");
        return NULL;
    }
	printf("Past empty file check in fork\n");
	
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

	//Copies the content
	strcpy(fName, fileName);
	
	//Uses '.' as a delimiter to seperate the file name from the extension to get the correct compressed file name
	const char delim[] = ".";
	
	//index variable for putting tokenized words into 2D array
    char *token = strtok(fName, delim);

    //declaring a double pointer 2D array of type char which will hold the file name and the extension for the uncompressed file name
    char **strArr = (char**)malloc(sizeof(char*));

    //copying the address of the tokenized word into the arrays index
    strArr[0]= token;
     
    //Creating the second token for the extension
    token = strtok(NULL,delim);

	//copying the address of the tokenized word into the arrays index
	strArr[1]= token;
	
	printf("Tokenized in fork\n");
		
	printf("Set starting Points in fork\n");
	
	//setting each struct in the struct array to necessary data used by each thread to compress
	printf("Creating %d structs with specified data each \n", parts);

	//creating number of processes user requests
    pid_t pid;
	printf("Entering fork in fork\n");
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
			printf("Process %d created\n", i);
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