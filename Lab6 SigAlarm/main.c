#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int runs = 0;
unsigned int alarm(int seconds);

void alarmhandler(int sig){

    printf("Alarm triggered!\n");
    runs++; // each time this function is called, the runs increases
    alarm(5); // 5 seconds alarm

}

int main()
{
    signal(SIGALRM, alarmhandler); // signal an alarm
    alarm(5); // alarm waits 5 seconds

    while(1){

        if(runs == 3) break; // if the alarm has been triggered three times, it terminates

    }

    printf("\nDone.");

    return 0;
}
