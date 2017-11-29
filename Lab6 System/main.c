#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void ctrlCpressed(int sig);

int keypressed = 0;

int main()
{
    signal(SIGINT, ctrlCpressed); // when we press ctrl + C, a SIGINT is transmitted, so that's why the function is stopping

    printf("looping...");

    while(1){

        if(keypressed)
            break;

    }

    printf("\nnow you see it ;)\n");

    return 0;
}

void ctrlCpressed(int sig){

    keypressed = 1;

}
