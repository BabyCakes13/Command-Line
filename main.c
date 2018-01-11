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

    /*
    This function handles the tee command.
    First, it check which of the words in the command line contain .txt, because there can be garbage values which are not taken in consideration.
    It counts how many files there are, so it knows how many files it will create.
    It also checks wether the tee command has -a parameter, for appending.
    If there are no files, it just lets the user write and the text will be simply outputed in command line.
    Else, we keep all the text files in an array, later opening the first file, taking the input from the command line, and close it. Then open and close the others also.
    When the C^ is pressed, we will have the written text in all files opened with tee.
    */
    char* textFiles[500];
    int numberFiles = 0;
    boolean isTextFile;
    boolean hasParameter = 0;

    int i = 1;
    while(commands[i] != NULL){

        isTextFile = check_text(commands[i]);

        if(isTextFile == 1)
            numberFiles++;

        if(strcmp(commands[i], "-a") == 0)
            hasParameter = 1;

        i++;

    }

    if(numberFiles != 0){

        for(i = 0; i < numberFiles; i++){

            textFiles[i] = (char*)malloc(sizeof(char)*300);

        }

        i = 0;
        int j = 0;
        while(commands[i] != NULL){

            isTextFile = check_text(commands[i]);

            if(isTextFile == 1){

                strcpy(textFiles[j], commands[i]);
                j++;

            }

            i++;

        }

        FILE* newFile;

        if(hasParameter == 0){

            newFile = fopen(textFiles[0], "w");

        }else{

            newFile = fopen(textFiles[0], "a");

        }


        signal(SIGINT, signalHandler);

        char* text;

        keyPressed = false;

        while(keyPressed == false){

            text = readline("");
            fprintf(newFile, "%s\n", text);
            printf("%s\n", text);

        }

        fclose(newFile);

        if(numberFiles > 1){

            FILE* newFile2;


            char letter;

            for(i = 1; i < numberFiles; i++){

                if(hasParameter == 0){

                     newFile2 = fopen(textFiles[i], "w");

                }else{

                     newFile2 = fopen(textFiles[i], "a");

                }

                newFile = fopen(textFiles[0], "r");

                letter = fgetc(newFile);

                while(letter != EOF){

                    fputc(letter, newFile2);
                    letter = fgetc(newFile);

                }

                fclose(newFile2);

            }


        }

    }else{

        signal(SIGINT, signalHandler);

        keyPressed = false;

        char* text;

        while(keyPressed == false){

            text = readline("");
            printf("%s\n", text);

        }

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

int hasPipe(char* commandLineInput){

    /*
    This function checks wether the command line input has pipes
    */
    int i = 0;
    int numberPipes = 0;

    for(i = 0; i < 100; i++){

        if(commandLineInput[i] == '|')
            numberPipes++;

    }

    return numberPipes;

}
void break_arguments(char* commandlineInput, char* brokenByPipeCommands[]){

    /*
    The function takes the argument line from the command line, and splits it into separate commands by |.
    It keeps all the commands in a char double pointer
    */

    int i, numberCommands = 0;
    for(i = 0; i < 100; i++){

        brokenByPipeCommands[i] = (char*)malloc(sizeof(char)*100);

    }

    int word = 0, letter = 0;
    i = 0;

    brokenByPipeCommands[word][letter] = commandlineInput[i];

    while(commandlineInput[i] != '\0'){

        if((commandlineInput[i] != '|') && (commandlineInput[i+1] != '\0')){

            //printf("In if: word: %d; letter %d; i: %d; charachter: %c\n", word, letter, i, commandlineInput[i]);
            brokenByPipeCommands[word][letter] = commandlineInput[i];
            letter++;

        }else{

            //printf("In else: word: %d; letter %d; i: %d; charachter: %c\n", word, letter, i, commandlineInput[i]);
            if(commandlineInput[i+1] == '\0'){

                brokenByPipeCommands[word][letter] = commandlineInput[i];
                letter++;

            }
            brokenByPipeCommands[word][letter] = '\0';

            word++;
            letter = 0;

            numberCommands++;

        }

        i++;

    }

    for(i = 0; i < numberCommands; i++){

        printf("%s ", brokenByPipeCommands[i]);

    }

}

void interpretPipe(char* commandLine){

    pid_t pid;

    pid = fork();

    if(pid == -1){

        perror("Failed to create process.\n");
        exit(1);

    }

    if(pid == 0){



    }

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

        int numberPipes = hasPipe(command);
        printf("Command %s has pipes: %d\n",command, numberPipes);

        if(numberPipes == 0){

            parse_command(arguments, command);

            if(strlen(command) >= 1){

                add_history(command);
                work = interpret(arguments, command);

            }

        }else{

            char* brokenByPipeCommands[100];

            break_arguments(command, brokenByPipeCommands);

            work = interpretPipes(brokenByPipeCommands);

        }

    }

    free(command);

}

