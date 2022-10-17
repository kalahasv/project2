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
#include <errno.h>

#define MAX_LINE 80     //given in assignment details
#define MAX_ARGC 80     //given in assignment details
#define MAX_JOB 5       //given in assignment details

//we need to use a global variable for the signal handler since it can't have any parameters passed to it
pid_t f_pid;
int f_indx;

enum job_Status {
    AVAILABLE,
    FOREGROUND,
    BACKGROUND,
    STOPPED
} jobList2[MAX_JOB];     // global struct variable in order to assists signal handler functions

struct job_Info {
    pid_t pid;
    enum job_Status status;
    int job_id;
    //char cmd[MAX_LINE];
};

typedef enum working_Space {
    WP_BACKGROUND,
    WP_REDIRECT_INPUT,
    WP_REDIRECT_OUTPUT,
    JOB_ID
} working_Space;

void addJob(struct job_Info *jobList, pid_t pid, enum job_Status status) {
    for (int i = 0; i < MAX_JOB; i++) {
        if (jobList[i].status == AVAILABLE) {
            jobList[i].pid = pid;
            jobList[i].status = status;
            jobList[i].job_id++;
            break;
        }
    }
    return;
}

void constructJobs(struct job_Info *jobList) {   // Construct a default list of jobs
    for (int i = 0; i < MAX_JOB; i++) {
        jobList[i].pid = 0;
        jobList[i].status = AVAILABLE;
        jobList[i].job_id = 0;
        //jobList[i].cmd[0] = '\0';
    }
}

void sigchdHandler( int signalNum){
    printf("The sigchild handler actually got called");
    int stat;
    wait(&stat);
}
void interruptHandler(int signalNum) {        // PAUSED !!!
    // use kill() to send signal to a certain job
    // need to know which is the current foreground job
    printf("current pid that needs to be interrupted: %d\n", f_pid);
    
    if (f_pid > 0) {
        kill(-f_pid, SIGINT);
        printf("kill(%d) error! \n",SIGINT);   // shouldnt be printed
        exit(1);
    } 
    

}
void eval(struct job_Info *jobList, char **argv, int argc, working_Space space){

    //for (int i = 0; i < argc; i++) {
    //    printf("argv[%d] = %s\n", i, argv[i]);
   // }
   
    char cwd[MAX_LINE];     // store current working directory path
    // First arugment in the argv[] is the command
    // Other arguments could be a path or executable programs or ampersand

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
    else if (strcmp(argv[0], "jobs") == 0) {  // not a built-in command. change to another stage
        // do something
    }
    // else if (strcmp(argv[1], "&") == 0) {
    //    printf("Should run this as a background process\n");
        //can run multiple background processes at once 
        //can still add it to the jobList though for the sake for keeping track

    // }

    else {     //run as a different process
        
        pid_t pid = fork();        // child's pid to the parent process      
        printf("Parent pid is %d \n", pid);
        //char* envvar = argv[0];
        //int errcode;
        
        
        if (pid == 0) {      // child process is successfully spawned
            printf("child pid is %d \n", pid);
            
            if(space == WP_BACKGROUND){ //need to get parent pid or use another method to check 
                 //if it's  a background job  
                  printf("This is a background job.\n");
                  printf("Argument going into execv: %s\n",argv[0]);
                  fflush(stdin);
                  fflush(stdout);
                  argv[1] = NULL;
                  if(execv(argv[0], argv) < 0){     // Negative value -> ERROR HERE
                     //printf("Return value: %d\n",execv(argv[0],argv));
                     printf("%s: Command not found.\n",argv[0]);
                     exit(0);
                  }   
                   
            }
            
            else{
                printf("This is a foreground process.\n");
                printf("Current f_pid:%d\n",f_pid);
                int reap_status;
                waitpid(pid,&reap_status,0); //calling wait after a child process is formed -> only if this is a foreground job
                
                fflush(stdin);
                fflush(stdout);
                // first try execvp to execute ./hello 
                printf("First argument: %s\n", argv[0]);
                if (strstr("./", argv[0]) != NULL || strcmp("ls", argv[0]) == 0){       // does not seem to be working
                    printf("EXECVP Functionality not working at this time.");
                }
                else {
                    //printf("Cannot execute command \n");
                    if(execv(argv[0], argv) < 0){     // Negative value -> ERROR HERE
                        printf("%s: Command not found.\n",argv[0]);
                        exit(0);
                    }

                }
               
                printf("Reap status: %d\n",reap_status);
               
            }
           // exit(EXIT_SUCCESS);

            
            printf("Finished conditionals.\n");
            
           

            // since execv need to works with both /bin/ls and ls 
            // ... 
        }

        else if (pid < 0) {
            printf("Spawning child process unsuccessful! \n");
        }

        else {      // parent process
            // create new job and add the foreground job into the job list 
            if(space == WP_BACKGROUND){
                //signal(SIGCHLD,sigchdHandler); // for the bg process
                if(strcmp(argv[1], "&") == 0) {
                    printf("New background job.\n");
                    addJob(jobList, pid, BACKGROUND);   //bg job
                }
              
            }
            else{
                addJob(jobList, pid, FOREGROUND);
            }
        }   
        
        //KEEP THIS FOR FOREGROUND PROCESSES ONLY
        int status; //locations where waitpid stores status
        if(waitpid(pid, &status, 0) < 0){
           //errcode = errno;
           printf("waitfg: waitpid error\n");
           exit(EXIT_FAILURE);
        }
        
        
    }

}

void distributeInput(char* input, int* argc, char** argv) {
    char* token;        
    const char* delims = " \t\n";
    token = strtok(input, delims);      // first token is the command
    while (token != NULL) {             // getting next arguments in to argv
        // printf("here is some tokens: %s\n", token);
        argv[(*argc)++] = token;
        token = strtok(NULL, delims);
    }
}

pid_t currentFGJobPID(struct job_Info *jobList) {
    for (int i = 0; i < MAX_JOB; i++) {
        if (jobList[i].status == FOREGROUND) {
            return jobList[i].pid;
        }
    }
    return 0;
}

int getCurrentFGJobIndex(struct job_Info *jobList) {
    for (int i = 0; i < MAX_JOB; i++) {
        if (jobList[i].status == FOREGROUND) {
            return i;
        }
    }
    return -1;
}



working_Space checkInput(int* argc, char **argv) {
    working_Space space;
    for (int i = 0; i < *argc; i++) {
        
        if (strcmp(argv[i], "&") == 0) {     // Background space
            space = WP_BACKGROUND;
        }  
        //else if () {    // redirect input
            // do something
        //}
        //else if () {      // reidrect output
            //
        //}
        //else if () {      // use to displace stuff by job ID

        //}
        
    }
    return space;
}


int main() {
     
    char input[MAX_LINE];  // Input from user. Each argument is seperated by a space or tab character 
    int argc;               // Number of arguments from the input
    char* argv[MAX_LINE];  // List of arguments. First argument would be the command
      
    struct job_Info jobList[MAX_JOB];      // list of jobs Should have a constructor for job list 
    constructJobs(jobList);
    // Signals
    signal(SIGINT, interruptHandler);   // When user type in Ctrl+C, interrupt signal handler will be called
        
    f_pid = currentFGJobPID(jobList); //get PID
    
   // f_indx = getCurrentFGJobIndex(jobList); //gets index in jobList of the current FGJob
   
   
    
    while(1) //loop until quit is entered
    {
        fflush(stdin);
        argc = 0;   // reset number of arguments every time getting a new input
        printf("prompt >");
        
        fgets(input, MAX_LINE, stdin);          // Get user input
        distributeInput(input, &argc, argv);    // Distribute arguments from user input
        // check input to see if in bg, or on shell or to redicted files
        working_Space space = checkInput(&argc, argv);


        if(feof(stdin)){
            exit(0);
        }
        
        eval(jobList, argv, argc, space);     // evaluate the list of arguments
        f_pid = currentFGJobPID(jobList); //get current FG PID every time 
        //f_indx = getCurrentFGJobIndex(jobList); //get current index of FG Job every time
        fflush(stdin);

        
    }

    sleep(10);
    return(0);

}
