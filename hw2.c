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
};     // global struct variable in order to assists signal handler functions

struct job_Info {
    pid_t pid;
    enum job_Status status;
    int job_id;
    //char cmd[MAX_LINE];
} jobList[MAX_JOB];

typedef enum working_Space {
    WP_FOREGROUND,
    WP_BACKGROUND,
    WP_REDIRECT_INPUT,
    WP_REDIRECT_OUTPUT,
    JOB_ID
} working_Space;

void addJob(pid_t pid, enum job_Status status) {
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

void constructJobs() {   // Construct a default list of jobs

    for (int i = 0; i < MAX_JOB; i++) {
        jobList[i].pid = 0;
        jobList[i].status = AVAILABLE;
        jobList[i].job_id = 0;
        //jobList[i].cmd[0] = '\0';
    }
}

void pauseCurrentFGJob(pid_t pid) {
    while (pid) {
        sleep(0);
    }
}


void eval(char **argv, int argc, working_Space space){

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


    else {     //run as a different process
        
        int reap_status;
        pid_t pid;        // child's pid to the parent process      
        //printf("Parent pid is %d \n", pid);
        
        if ((pid = fork()) == 0) {      // child process is successfully spawned
            printf("child pid is %d \n", pid);
            

            if(execv(argv[0], argv) < 0){     // Negative value -> ERROR HERE
                        printf("%s: Command not found.\n",argv[0]);
                        exit(0);
            }

            exit(EXIT_SUCCESS);

            
            printf("Finished conditionals.\n");
            // since execv need to works with both /bin/ls and ls 
            // ... 
        }

        else if (pid < 0) {
            printf("Spawning child process unsuccessful! \n");
        }

        else {      // parent process
            // create new job and add the foreground job into the job list
            printf("parent id: %d\n", pid);
            if(space == WP_BACKGROUND){
                if(strcmp(argv[1], "&") == 0) {
                    printf("New background job.\n");
                    addJob(pid, BACKGROUND);   //bg job
                }
               
            }
            else{
                printf("New foreground job.\n");
                addJob(pid, FOREGROUND);
                //int status; //locations where waitpid stores status
                //if(waitpid(pid, &status, 0) < 0){
                //printf("waitfg: waitpid error\n");
                //exit(EXIT_FAILURE);
                //}
            }

            if (space == WP_FOREGROUND) {
                //pauseCurrentFGJob(pid);
                pause();
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

pid_t currentFGJobPID() {
    pid_t pid = 0;
    for (int i = 0; i < MAX_JOB; i++) {
        if (jobList[i].status == FOREGROUND) {
            pid = jobList[i].pid;
        }
    }
    return pid;
}

int getCurrentFGJobIndex(struct job_Info *jobList) {
    for (int i = 0; i < MAX_JOB; i++) {
        if (jobList[i].status == FOREGROUND) {
            return i;
        }
    }
    return -1;
}

void changeJobStatus(pid_t pid, enum job_Status newStatus) {

    for (int i = 0; i < MAX_JOB; i++) {
        if (jobList[i].pid == pid) {
            jobList[i].status = newStatus;
        }
    }
}

void deleteJob(pid_t pid) {
    for (int i = 0; i < MAX_JOB; i++) {
        if (jobList[i].pid == pid) {
            jobList[i].pid = 0
            jobList[i].jid = 0;
            jobList[i].status = AVAILABLE;
        }
    }
}

void interruptHandler(int signalNum) {        // PAUSED !!!
    // use kill() to send signal to a certain job
    // need to know which is the current foreground job
    //printf("current pid that needs to be interrupted: %d\n", f_pid);
    pid_t pid = currentFGJobPID(jobList);

    if (pid > 0) {
        //printf("ctrl C on %d\n", pid);
        kill(-pid, SIGINT);
    } 

}

void stopHandler(int signalNum) {

    pid_t pid = currentFGJobPID(jobList);

    if (pid > 0) {
        kill(pid, SIGTSTP);
    } 
}

void sigchdHandler(int signalNum) {
    int status = 0;
    pid_t pid = 0;

    while ((pid = waitpid(currentFGJobPID(jobList), &status, WNOHANG | WUNTRACED)) > 0) {
        printf("child pid in sigchd: %d\n", pid);
        if (WIFSIGNALED(status)) {      // child process has terminated by a signal 
            printf("interrupt is sent\n");
            interruptHandler(SIGINT);
        }
        else if (WIFSTOPPED(status)) {  // child process has been stopped by delivery of a signal 
            stopHandler(SIGTSTP);
            changeJobStatus(pid, STOPPED);
        }
        else if (WIFEXITED(status)) {   // child process has been terminated normally
            // for terminated jobs, need to manually delete them
            deleteJob(pid);
        }
    }
}


working_Space checkInput(int* argc, char **argv) {
    working_Space space = WP_FOREGROUND;
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
      
    //struct job_Info jobList[MAX_JOB];      // list of jobs Should have a constructor for job list 
    constructJobs(jobList);
    // Signals
    //f_pid = currentFGJobPID(jobList); //get PID
    
   // f_indx = getCurrentFGJobIndex(jobList); //gets index in jobList of the current FGJob
    signal(SIGINT, interruptHandler);   // When user type in Ctrl+C, interrupt signal handler will be called
    signal(SIGTSTP, stopHandler);
    signal(SIGCHLD, sigchdHandler);     // for the bg process
    
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
        
        eval(argv, argc, space);     // evaluate the list of arguments
        //f_pid = currentFGJobPID(jobList); //get current FG PID every time 
        //f_indx = getCurrentFGJobIndex(jobList); //get current index of FG Job every time
        fflush(stdin);

        
    }
   

    
    return(0);

}
