//hello test
//need to accept command line arguments 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LINE 80     //given in assignment details
#define MAX_ARGC 80     //given in assignment details
#define MAX_JOB 5       //given in assignment details

struct job_Info {
    pid_t pid;
    int job_id;
    char cmd[MAX_LINE];
};

void eval(char **argv){


    char cwd[MAX_LINE];
    // First arugment in the argv[] is the command
    // Other arguments could be a path, executable 
    // Built-in commands
    if (strcmp(argv[0], "cd") == 0) {
        //printf("in cd, path is: %s", argv[1]);
        chdir(argv[1]);
    }
    else if (strcmp(argv[0], "pwd") == 0) {
        printf("%s\n", getcwd(cwd,sizeof(cwd)));
    }
    else if (strcmp(argv[0], "quit") == 0) {
        exit(0);
    }
    else if (strcmp(argv[0], "jobs") == 0) {
        // do something
    }
    else if (strcmp(argv[0], "fg") == 0) {
        // do something
    }
    else if (strcmp(argv[0], "bg") == 0) {
        // do something
    }
    else {      // not a built-in command. change to another stage
        // do something
    }

}

void distributeInput(char* input, int* argc, char** argv) {
    char* token;        
    const char* delims = " \t\n";
    token = strtok(input, delims);      // first token is the command
    while (token != NULL) {             // getting next arguments in to argv
        printf("tokens: %s\n", token);
        argv[(*argc)++] = token;
        token = strtok(NULL, delims);
    }
}


int main() {
     
    char input[MAX_LINE];  // Input from user. Each argument is seperated by a space or tab character 
    int argc;               // Number of arguments from the input
    char* argv[MAX_LINE];  // List of arguments. First argument would be the command
      
 

    while(1) //loop until quit is entered
    {
        fflush(stdin);
        argc = 0;   // reset number of arguments every time getting a new input
        printf("prompt >");
        
        fgets(input, MAX_LINE, stdin);          // Get user input
        distributeInput(input, &argc, argv);    // Distribute arguments from user input

        if(feof(stdin)){
            exit(0);
        }
        eval(argv);     // evaluate the list of arguments
        fflush(stdin);

        
    }
   

    
    return(0);

}
