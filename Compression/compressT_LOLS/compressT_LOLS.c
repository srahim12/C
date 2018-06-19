#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<ctype.h>

//Prototyping compressT_LOLS defined by the project and compress for multithread
void *compress(void *arg);
void *compressT_LOLS(char *fileName, int parts);

//data struct which will be used by each thread for compression
typedef struct data{
    int numberOfCharsToRead;
    int threadID;
    int startingPoint;
    int amtOfThreads;
	char *fileName;
	char *extension;
	char *file;
}data;

//Made global to make appending the file name easier. 
int fileNameCounter = 0;
int numberOfCharsInFile = 0;

//main function which inputs filename, and number of threads, then passes the two values into compressLOLS
int main(int argc, char **argv){
    
    //If the user did not enter two arguments call an error and end the program
    if(argc != 3){
        printf("Error: There are not 2 arguments.\n");
        return -1;
    }
    
    //Atoi failure error check
    int parts;
    if((parts = atoi(argv[2])) == 0){
        printf("Error: User didn't enter a number for parts or it was 0.\n");
        return -1;
     }
   
	//Calling function which will check for errors and create multithreads
    compressT_LOLS(argv[1],parts);
    
	return 0;
}

void *compressT_LOLS(char *fileName, int parts){
  
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
        
    numberOfCharsInFile = 0;
    
	//opening the file
	FILE *fp = fopen(fileName, "r");
	
	char c;

    /* for as long as we can get characters... */
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
	//Create an array with each index corresponding to the number of char's that thread needs to read
    int i;
    
	int arrayNumChars[parts];
    for(i = 0; i < parts; i++){
        arrayNumChars[i] =  numberOfCharsInFile / parts ;
    }
    
    // divide up the remainder 
    for (i = 0; i <= (int)(numberOfCharsInFile % parts) - 1 && (int)(numberOfCharsInFile % parts) != 0;i++){
        arrayNumChars[i] += 1;
    }
   
    //array which holds all the starting points for each thread to compress
    int arrayStart[parts];
    
	arrayStart[0] = 0;
   
	
    //setting all the sarting points from which the thread should start executing
    for(i = 1; i < parts; i++){
        arrayStart[i] = arrayStart[i - 1] + arrayNumChars[i - 1];
    }
	
	//creating an array of threads  
    pthread_t arrThreadObj[parts]; 
    
	//declaring an array of structs which will hold all necessary data for the thread to use during its portion of compression
    data arg[parts];
    
	int j;
    
	//setting each struct in the struct array to necessary data used by each thread to compress
	for(j = 0; j < parts; j++){     

        arg[j].threadID = j;
        arg[j].startingPoint = arrayStart[j];
        arg[j].numberOfCharsToRead = arrayNumChars[j];  
        arg[j].amtOfThreads = parts;
		arg[j].fileName = fileName;
		arg[j].extension = strArr[1];
		arg[j].file = strArr[0];
	}
    
    fileNameCounter = parts;
    int curr;
    
	int rc;
               
	printf("***************************************************************************************\n");
    
	//creates threads passing in compress as function and the struct as its argument to use for compression
    for(curr = 0; curr < parts; curr++){    
        rc = pthread_create(&arrThreadObj[curr],NULL, compress, &arg[curr]);
        //Checks if thread is created
        if (rc){
            printf("error: in pthread_create() \n");
            exit(-1);
        }
    }
   
    //joins all the threads to ensure they dont run unpredictably
    for(curr = 0; curr < parts; curr++){
        pthread_join(arrThreadObj[curr], NULL);
    }
	
    return NULL; 
}

void *compress(void *arg){
    //casting arg into a pointer to a data struct 
    data *argReal = (data*) arg;

    FILE *fp =fopen(argReal->fileName,"r");

    int i; 
       
	char stringOfWholeLine[numberOfCharsInFile];
	
	fgets(stringOfWholeLine, numberOfCharsInFile + 1, fp);

	//creating array to store files characters
    char str[argReal->numberOfCharsToRead];
	
	int x=0;  
    	
	//goes through file and stores the characters into str
    for(i = argReal->startingPoint; i <  (argReal->startingPoint + argReal->numberOfCharsToRead); i++){
			str[x] =  stringOfWholeLine[i];
			x++;
	}
	str[x] = '\0';
	    
    //allocating memory in heap to store the compressed string portion the thread compresses
    char outputString[argReal->numberOfCharsToRead+10];
    
	int k;
	
	int letterCount = 1;
	
	outputString[0] = '\0';
	
	//loops through the portion of the file the thread is assigned to compress
    for(k = 0; k < argReal->numberOfCharsToRead; k++){
		if(!isalpha(str[k])){}
		else{
			if (k == strlen(str)-1){
				if(letterCount ==2){
					char cToStr[3];
					cToStr[2] = '\0';
					cToStr[0] = str[k];
					cToStr[1] = str[k];
					strcat(outputString, cToStr);
				}
				else if(letterCount <2){
					char cToStr[2];
					cToStr[1] = '\0';
					cToStr[0] = str[k];
					strcat(outputString, cToStr);
				}
				else if(letterCount>2){
					char cToStr[4], chr[2];
					sprintf(cToStr, "%d", letterCount);
					chr[0] = str[k];
					chr[1] = '\0';
					strcat(outputString, cToStr);
					strcat(outputString, chr);
				}
				break;
			}
			if (str[k] == str[k+1])
				letterCount++;	
			else if(str[k]!= str[k+1] && letterCount == 2){ 
				char cToStr[3];
				cToStr[2] = '\0';
				cToStr[0] = str[k];
				cToStr[1] = str[k];
				strcat(outputString, cToStr);
				letterCount = 1; 
			}    
			else if(str[k]!= str[k+1] && letterCount <2){
				char cToStr[2];
				cToStr[1] = '\0';
				cToStr[0] = str[k];
				strcat(outputString, cToStr);
				letterCount = 1;
			}
			else if(str[k]!= str[k+1] && letterCount >2){
				char cToStr[4], chr[2];
				sprintf(cToStr, "%d", letterCount);
				chr[0] = str[k];
				chr[1] = '\0';
				strcat(outputString, cToStr);
				strcat(outputString, chr);
				letterCount = 1;
			}
		}
    }
	printf("After the compression for thread#%d compressed is: %s\n", argReal->threadID, outputString);
    
    //If the number of threads = 1 we use the naming scheme without the number at the end
   	char *newFileName = (char*) malloc(sizeof(char) *100);
	if(argReal->amtOfThreads == 1){
		char ext[2];
		char l[] = "_LOLS";
		ext[0]= '_';
		ext[1]= '\0';
		strcat(newFileName, argReal->file);
		strcat(newFileName, ext);
		strcat(newFileName, argReal->extension);
		strcat(newFileName, l);
	}
	
	if(argReal->amtOfThreads  > 1){
		char ext[2];
		char l[] = "_LOLS";
		ext[0]= '_';
		ext[1]= '\0';
		char chr[5]; 
		sprintf(chr, "%d",argReal->threadID);
		strcat(newFileName, argReal->file);
		strcat(newFileName, ext);
		strcat(newFileName, argReal->extension);
		strcat(newFileName, l);
		strcat(newFileName, chr);
    }

	FILE *fp2 = fopen(newFileName, "w");
	//printf("Does it get to this point for THREAD#%d\n", argReal->threadID);
	fprintf(fp2,outputString);
	printf("FILE: %s\n", newFileName);
	fclose(fp2);
	
	fclose(fp);
  
    pthread_exit(NULL);
}
