#include <stdio.h>
#include <unistd.h>
int main() {
unsigned int i = 0;
for(i = 0; i < 20; i++)
{
    if( i == 10)  {
        printf("I'm still running <3");
    }
    sleep(5);
}
return 0;
}