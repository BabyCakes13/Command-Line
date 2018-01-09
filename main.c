#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

typedef enum{ false, true} boolean;

static volatile int keepRunning = 1;

boolean interpret(char** arguments, char* command);
void handle_help();
void open_interpreter();
void parse_command(char** argument, char* command);
int handle_cd();
int keyPressed = 1;
void handle_yes();
void handle_tee();
void signalHandler(int mysignal);
boolean check_text(char* string);

int main(){

    open_interpreter();

    return 0;

}

boolean interpret(char** commands, char* command){
/*

Work is the status of the program - if the while from open_interpreter() gets false, it terminates.
We use a pipe so we can correctly pass the work from the child to the parent.
child is the new process which handles the interpretation of the command line.
In each case, we write into the pipe the work state, then pass it to the parent.
In parent, we wait for the child process to finish, then we read from the pipe the work state.

*/

    boolean work = true;

    int fd[2];

    if(pipe(fd) < 0){

        exit(-1);
        perror("Error in pipe: ");

    }

    pid_t child;
    child = fork();

    if(child < 0){

        perror("Error in child: ");
        exit(-1);

    }

    if(child == 0){

        close(fd[0]); // close the reading part of the pipe

        if(strcmp(command, "exit") == 0){

            work = false;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);

        }else if(strcmp(command, "help") == 0){

            handle_help();
            work = true;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);


        }else if(strcmp(command, "yes") == 0){

            work = 3;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);


        }else if(strcmp(command, "tee") == 0){

            work = 4;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);

        }else if(execvp(commands[0], commands) != -1){

            work = true;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);

        }else if(strcmp(command, "cd") == 0){
            //TODO

            work = 2;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);

        }else{

            work = true;
            printf("command does not exist\n");
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);

        }

    }else{

        wait(&child);

        boolean work = true;

        close(fd[1]);
        read(fd[0], &work, sizeof(boolean));
        close(fd[0]);

        if(work == 2){

            handle_cd(commands);
            return true;

        }else if(work == 3){

            handle_yes(commands);
            return true;

        }else if(work == 4){

            handle_tee(commands);
            return true;

        }else{

            return work;

        }

        return work;

    }

}

void signalHandler(int mysignal){

    keyPressed = true;

}

void handle_tee(char** commands){

      int i = 1;
      char** textFiles = NULL;
      boolean isTextFile;

      while(commands[i] != NULL){

        isTextFile = check_text(commands[i]);

        printf("%d\n", isTextFile);
        i++;

      }

    if(commands[1] != NULL){

        FILE* newFile = fopen(commands[1], "w");
        printf("success\n");


        printf("\n");

        signal(SIGINT, signalHandler);

        char* text;

        keyPressed = false;

        while(keyPressed == false){

            text = readline("");
            fprintf(newFile,"%s\n", text);
            printf("%s\n", text);

        }

        keyPressed = true;
        fclose(newFile);

    }else{

        printf("There must be a name for the text file.\n");

    }

}

boolean check_text(char* string){

    /*
        This function checks if the arguments read from the command line represent a text file or not.
    */

    int i = 0, j = 0;
    char newString[500], dotTxt[5];

    strcpy(newString, string);

    for(i = strlen(newString) - 4; i < strlen(newString); i++){

        dotTxt[j] = newString[i];
        j++;

    }
    dotTxt[j] = 0;

    if(strcmp(dotTxt, ".txt") == 0){

        return 1;

    }else{

        return 0;

    }

}
void handle_yes(char** commands){

    signal(SIGINT, signalHandler);

    keyPressed = false;

    int i = 1;

    char commandsArray[500];

    while(commands[i] != NULL){

        strcat(commandsArray, commands[i]);
        strcat(commandsArray, " ");
        i++;

    }

    while(keyPressed == false){

        if(strlen(commandsArray) > 0){

             printf("%s\n", commandsArray);

        }else{

            printf("y\n");

        }


    }

    keepRunning = true;

}


int handle_cd(char** commands){

    if((strcmp(commands[1], " ") == 0) || (strcmp(commands[1], "~") == 0) || (strcmp(commands[1], "") == 0 || commands[1] == NULL)){

        return chdir(getenv("HOME"));

    }else{

        if(chdir(commands[1]) != 0){

            perror("Error in chdir(): ");

        }else{

            return chdir(commands[1]);

        }

    }

}
void handle_help(){

    printf("HELP\n");

    printf("Type 'exit' to exit\n");
    printf("Type commands eg: yes this, cat text.txt, ls, etc\n");

}


void parse_command(char** argument, char* command){
/*

This function parses the line given in the arguments, so that execpv will have the separated words in order to work.
The first while will exit when the command has been read completely - when it gets to the terminator of the command.
The second while separates the command into words - when it finds one of \t,\n or space, it terminates the line by adding a terminator, then puts it in arguent.
The argument will have a char array (double poiner char) which will keep all the words in the command separated.
The third while goes to the next word in the command.
When all is finished, all the words in the command line are separated and put into the char array.

*line = line[this]

*/
    while(*command != '\0'){

        while(*command == ' ' || *command == '\t' || *command == '\n'){

            *command++ = '\0';

        }

        *argument++ = command;

        while(*command != '\0' && *command != '\t' && *command != '\n' && *command != ' '){

            command++;

        }
    }


    *argument = '\0';

}
void open_interpreter(){

/*

Command is the written command in the terminal
Arguments is breaking the command in words
While work is true, the program is still running
We read the command using readline, then parse it with the parse_command()
we add history only if there is at least one character written
If that is the case, interpret

*/

    boolean work = true;
    char* command;
    char* arguments[501];
    long size = pathconf(".", _PC_PATH_MAX);
    char* path;

    while(work){

        printf("%s",getcwd(path, (size_t)size));
        command = readline(">");
        parse_command(arguments, command);

        if(strlen(command) >= 1){

            add_history(command);
            work = interpret(arguments, command);

        }

    }

    free(command);

}

