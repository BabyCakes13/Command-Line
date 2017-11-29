#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

pid_t c; // pipe

void keypressed(int sig){

    printf("Hey there\n"); fflush(stdout);
    kill(c, SIGUSR1);

}

void parenthandler(int sig){

    printf("\nChild receieved SIGURS2");
    kill(getppid(), SIGUSR2);

}

void childhandler(int sig){

    printf("\nParent receieved SIGURS2 from child.");
    exit(0);

}

int main()
{


    c = fork();

    if( c > 0 ){

        signal(SIGUSR2, childhandler);
        signal(SIGINT, keypressed);
        while(1);

    }else{

        signal(SIGUSR1, parenthandler);
        printf("child running\n");
        while(1);

    }
    return 0;
}
