#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char * argv[]){
    printf("This is what is being inputted to add: %s\n",argv[1]);
    int n = atoi(argv[1]);
    
   // printf("file working.Input value: %d\n",n);
    printf("%d \n", n+2); // print n+2
    return 0;
}