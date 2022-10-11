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
//MAXLINE -> 80 //given in assignment details
// MAX ARGS -> 80 //given in assignment details
//MAXJOB ->5 //given in assignment details


void eval(char*input){
   // char*argv[80]; for later
    char* token;
    int bg; //whether the job runs in the foreground or bg for later
    pid_t pid; //process_id for later 
    char cwd[80]; //should probably make a global for the ints for the max amts
   

    //bg = parseline(input,argv); for later 
    //built_in command functions
    token = strtok(input, " \t\n"); //first token 
    
    if(strcmp(token,"cd") == 0){  
        token = strtok(NULL," \t\n"); //token should now equal the second argument
        chdir(token);
    
    }
    else if(strcmp(token,"pwd")== 0){
        printf("%s\n",getcwd(cwd,sizeof(cwd)));
        
    }
    else if(strcmp(token,"quit")== 0){
        exit(0);
    }
}



int main() {
    //each argument is seperated by a space or tab character
    
    char  input[80];
 

    while(1) //loop until quit is entered
    {
        printf("prompt >");
        fgets(input,80,stdin);  //user input

        if(feof(stdin)){
            exit(0);
        }
        eval(input);

        
    }
   

    
    return(0);

}