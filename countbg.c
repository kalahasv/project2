#include <stdio.h>
#include <unistd.h>
int main() {
unsigned int i = 0;
while(1)
{
printf("Counter background: %d\n", i);
i++;
sleep(1);
}
return 0;
}