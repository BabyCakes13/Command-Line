#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

boolean handle_help(){

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

