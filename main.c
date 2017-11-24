#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

typedef enum{ false, true } boolean;

boolean interpret(char* );
char* read_line();
boolean handle_help();
void open_interpreter();

int main(){
    open_interpreter();
    return 0;
}

boolean interpret(char* command){
    if(strncmp(command, "exit", strlen(command) -1 ) == 0){
        return false;
    }else{
        if(strncmp(command, "help", strlen(command) - 1) == 0){
            return handle_help();
        }else{
            printf("command not found\n");
            return true;
        }
    }
}

//pune hei la inceputul programului si dupa continua normal, daca apas help imi da wrror in execl
boolean handle_help(){

    pid_t child = fork();

    if(child < 0){

        printf("Error: fork() failed to create.\n");
        exit(-1);

    }

    if(child == 0){

        printf("(In child)\n");

        int help = execl("home/babycakes/Documents/Command Line Interpreter/Handle Help/help", "help", "-1", NULL);

        if(help < 0){

            perror("Error in e");

        }

    }else{

        printf("(In parent)\n");

        if(wait(0) == -1){

            perror("Error in wait: ");

        }

    }

    return true;

}

void open_interpreter(){

    boolean work = true;
    char* command;

    while(work){

        command = read_line();
        work = interpret(command);

    }
}

char* read_line(){

    char* command;

    command = readline(">");
    add_history(command);

    return command;

}

void open(){

}

