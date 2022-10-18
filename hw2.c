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

//global variable f_pid for use in interrupt handler
pid_t f_pid;
int f_indx;
int g_job_id = 1;

enum job_Status {
    AVAILABLE,
    FOREGROUND, //foreground_running
    BACKGROUND, //background_running
    STOPPED
};     

struct job_Info { 
    pid_t pid;
    enum job_Status status;
    int job_id;
    char cmd[MAX_LINE];
} jobList[MAX_JOB]; //global job list

typedef enum working_Space {
    WP_FOREGROUND,
    WP_BACKGROUND,
    WP_REDIRECT_INPUT,
    WP_REDIRECT_OUTPUT,
    JOB_ID
} working_Space;

void addJob(pid_t pid, enum job_Status status,char* cmdLine) {
    for (int i = 0; i < MAX_JOB; i++) {
        if (jobList[i].status == AVAILABLE) {
            jobList[i].pid = pid;
            jobList[i].status = status;
            jobList[i].job_id = g_job_id;
            strcpy(jobList[i].cmd,cmdLine);
            g_job_id++;
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
    }
}

void pauseCurrentFGJob(pid_t pid) { //pauses the current foreground job
    while (pid) {
        sleep(0);
    }
}

void printBgJobs(){

    static const char *STATUS_STRING[] = {
     "Running", "Stopped"};
    for(int i = 0; i < MAX_JOB; i++){
        if(jobList[i].status == BACKGROUND || jobList[i].status == STOPPED){
            printf("[%d] (%d) %s %s",jobList[i].job_id,jobList[i].pid,STATUS_STRING[jobList[i].status-2],jobList[i].cmd);
        }
    }
}

int getIndxByID(int id){
    for(int i = 0; i < MAX_JOB; i ++){
        if(jobList[i].job_id == id){
            return i;
        }
    }
}

int getIndxByPID(pid_t pid){
     for(int i = 0; i < MAX_JOB; i ++){
        if(jobList[i].pid == pid){
            return i;
        }
    }
}

void changeToForeground(int indx){

}
void eval(char **argv, int argc, working_Space space,char* cmdLine){

    char cwd[MAX_LINE];     // store current working directory path

    // Built-in commands
    if (strcmp(argv[0], "cd") == 0) {
        chdir(argv[1]);
    }
    else if (strcmp(argv[0], "pwd") == 0) {
        printf("%s\n", getcwd(cwd,sizeof(cwd)));
    }
    else if (strcmp(argv[0], "quit") == 0) {
        exit(0);
    }
    else if (strcmp(argv[0], "jobs") == 0) {  // not a built-in command. change to another stage
        printBgJobs();
    }
    else if (strcmp(argv[0], "fg")== 0){ //change job that is in either stopped state or background to fg
        
        if(argv[1][0] == '%'){ //find by job id
            changeToForeground(getIndxByID);
        }
        else{ //find by pid 
            changeToForeground(getIndxByPID);
        }
    }
    else {     //run as a general command
        
        int reap_status;
        pid_t pid;        // child's pid to the parent process      
        //printf("Parent pid is %d \n", pid);
        argv[argc] = NULL;
        if ((pid = fork()) == 0) {      // child process is successfully spawned
            //printf("child pid is %d \n", pid);
            if(execv(argv[0], argv) < 0){     // Negative value means it didn't work - try execv first
               if(execvp(argv[0],argv) < 0){ //try with execvp
                        printf("%s: Command not found.\n",argv[0]);
                        exit(0);
            }
            }
        }
        else if (pid < 0) {
            printf("Spawning child process unsuccessful! \n");
        }
        else {      // parent process
            printf("parent id: %d\n", pid);
            if(space == WP_BACKGROUND){ //create new background job 
                if(strcmp(argv[1], "&") == 0) {
                    printf("New background job.\n");
                    addJob(pid, BACKGROUND,cmdLine);   
                }
            }
            else{
                printf("New foreground job.\n"); //create new foreground job
                addJob(pid, FOREGROUND,cmdLine);
            }
            if (space == WP_FOREGROUND) { //pause if foreground otherwise said for sigchld
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
            jobList[i].pid = 0;
            jobList[i].pid = 0;
            jobList[i].status = AVAILABLE;
        }
    }
}

void interruptHandler(int signalNum) {        
    //printf("current pid that needs to be interrupted: %d\n", f_pid);
    pid_t pid = currentFGJobPID();

    if (pid > 0) {
        kill(-pid, SIGINT);
    } 

}

void stopHandler(int signalNum) {
    pid_t pid = currentFGJobPID();
    if (pid > 0) {
        kill(pid, SIGTSTP);
    } 
}

void sigchdHandler(int signalNum) {
    int status = 0;
    pid_t pid = 0;
    while ((pid = waitpid(currentFGJobPID(), &status, WNOHANG | WUNTRACED)) > 0) {
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
    char cmdLine[MAX_LINE]; //copy of input since input gets changed to strtok
    int argc;               // Number of arguments from the input
    char* argv[MAX_LINE];  // List of arguments. First argument would be the command
    constructJobs(); // fill the job list with empty jobs 
    
    signal(SIGINT, interruptHandler);   // When user type in Ctrl+C, interrupt signal handler will be called
    signal(SIGTSTP, stopHandler);       //deals with stop signal 
    signal(SIGCHLD, sigchdHandler);     // for the bg process, cleans up zombies
    
    while(1) //loop until quit is entered
    {
        fflush(stdin);
        argc = 0;   // reset number of arguments every time getting a new input
        printf("prompt >");
        fgets(input, MAX_LINE, stdin);          // Get user input
        strcpy(cmdLine,input); //make a copy & store in cmdLine
        distributeInput(input, &argc, argv);    // Distribute arguments from user input
        // check input to see if in bg, or on shell or to redicted files
       

        working_Space space = checkInput(&argc, argv);
        if(feof(stdin)){
            exit(0);
        }
        eval(argv, argc, space,cmdLine);     // evaluate the list of arguments
        fflush(stdin);

        
    }

    return(0);

}
