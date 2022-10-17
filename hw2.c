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
    }
}

void sigchdHandler( int signalNum){
    printf("The sigchild handler actually got called.\n");
    pid_t child_pid;
    int status;

    while((child_pid = waitpid(-1,&status,WNOHANG|WUNTRACED)) > 0){
        if(WIFSTOPPED(status)){ //crtl + z
            //to do 
        }
        else if (WIFSIGNALED(status) || WIFEXITED(status)){ //crtl + c, and then normal
            //todo
        }
        else{
            fprintf(stdout, "%s/n","waitpid error");
        }
    }
}
void interruptHandler(int signalNum) {        
    printf("current pid that needs to be interrupted: %d\n", f_pid);
    if (f_pid > 0) {
        kill(f_pid, SIGINT);
        //printf("kill(%d) error! \n",SIGINT);   // shouldnt be printed
       // exit(1);
    } 
}

void eval(struct job_Info *jobList, char **argv, int argc, working_Space space){
    char cwd[MAX_LINE];     // store current working directory path

    //built-in commands
    if (strcmp(argv[0], "cd") == 0) {
        chdir(argv[1]);
    }
    else if (strcmp(argv[0], "pwd") == 0) {
        printf("%s\n", getcwd(cwd,sizeof(cwd)));
    }
    else if (strcmp(argv[0], "quit") == 0) {
        exit(0);
    }
    else {    //general process
        pid_t pid = fork();        // child's pid to the parent process      
       // printf("Parent pid is %d \n", pid);

        if (pid == 0) {      // child process is successfully spawned
           // printf("child pid is %d \n", pid);     
           
               // printf("Current f_pid:%d\n",f_pid);
                fflush(stdin);
                fflush(stdout);
                argv[1] = NULL;
                //printf("First argument: %s\n", argv[0]);
                if (strstr("./", argv[0]) != NULL || strcmp("ls", argv[0]) == 0){       // TO-DO
                   // printf("EXECVP Functionality not working at this time.");
                   execvp(argv[0],argv);
                }
                else {
                    if(execv(argv[0], argv) < 0)
                    {     
                        printf("%s: Command not found.\n",argv[0]);
                        exit(0);
                    }
                }       
            
        }
        else if (pid < 0) {
            printf("Spawning child process unsuccessful! \n");
        }
        else {      // parent process
        
            if(space == WP_BACKGROUND){ //background process
                signal(SIGCHLD,sigchdHandler); // signal to receive if background process -> never detects change in status for some reason
                if(strcmp(argv[1], "&") == 0) {
                    printf("New background job.\n");
                    addJob(jobList, pid, BACKGROUND);   //bg job
                }    
            }
            else{
                int reap_status;
                pause();//calling waitpid after a child process is formed //call pause instead 
                addJob(jobList, pid, FOREGROUND);
            }
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
    constructJobs(jobList);                //add jobs to a list of jobs
    signal(SIGINT, interruptHandler);   // When user type in Ctrl+C, interrupt signal handler will be called
    f_pid = currentFGJobPID(jobList); //get foreground PID
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
        f_pid = currentFGJobPID(jobList); //get current FG PID every time for signal 
        f_indx = getCurrentFGJobIndex(jobList); //get current index of FG Job every time
        fflush(stdin);

    }

    
    return(0);

}
