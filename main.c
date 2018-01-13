#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>

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

void display_pointer(char** pointer){

    int i = 0;
    while(pointer[i] != NULL){

        printf("\n%d.%s ", i, pointer[i]);
        i++;

    }

}
boolean interpret(char** commands, char* command){
    /*
    Work is the status of the program - if the while from open_interpreter() gets false, it terminates.
    We use a pipe so we can correctly pass the work from the child to the parent.
    child is the new process which handles the interpretation of the command line.
    In each case, we write into the pipe the work state, then pass it to the parent.
    In parent, we wait for the child process to finish, then we read from the pipe the work state.
    This is used in case there are no pipes involved.
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

        if(strcmp(commands[0], "exit") == 0){

            work = false;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);

        }else if(strcmp(commands[0], "help") == 0){

            handle_help();
            work = true;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);


        }else if(strcmp(commands[0], "yes") == 0){

            work = 3;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);


        }else if(strcmp(commands[0], "tee") == 0){

            work = 4;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);

        }else if(execvp(commands[0], commands) != -1){

            work = true;
            write(fd[1], &work, sizeof(boolean));
            close(fd[1]);

        }else if(strcmp(command, "cd") == 0){

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

    /*
    This function sends signal when needed
    */

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

    /*
    This function handles yes command.
    If yes is the only word in the command line, it prints 'y' until CTRL-C is hit.
    Else, it prints the words from the command line, until CTRL-C is hit
    */

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

    /*
    This function handles cd
    */

    if((strcmp(commands[1], " ") == 0) || (strcmp(commands[1], "~") == 0) || (strcmp(commands[1], "") == 0 || commands[1] == NULL)){

        return chdir(getenv("HOME"));

    }else{

        if(chdir(commands[1]) != 0){

            perror("Error in cd: ");

        }else{

            return chdir(commands[1]);

        }

    }

}

void handle_help(){

    /*
    This function prints information about the program
    */

    printf("HELP\n");

    printf("Type 'exit' to exit\n");
    printf("Type commands eg: yes this, cat text.txt, ls, etc\n");

}

int hasPipe(char* commandLineInput){

    /*
    This function checks wether the command line input has pipes
    It returns the number of pipes in the command line input
    */

    int i = 0;
    int numberPipes = 0;

    for(i = 0; i < 100; i++){

        if(commandLineInput[i] == '|')
            numberPipes++;

    }

    return numberPipes;

}

int hasRedirect(char* commandLineInput){

    int i = 0;
    int numberRedirect = 0;

    for(i = 0; i < 100; i++){

        if(commandLineInput[i] == '<' || commandLineInput[i] == '>')
            numberRedirect++;

    }

    return numberRedirect;

}
int separateCommandsByPipe(char* commandlineInput, char* brokenByPipeCommands[]){

    /*
    The function takes the argument line from the command line, and splits it into separate commands by |.
    It keeps all the commands in a char double pointer
    */

    int i = 0, numberCommands = 0, word = 0, letter = 0;

    brokenByPipeCommands[0] = (char*)malloc(sizeof(char)*100);

    while(commandlineInput[i] != '\0'){

        if((commandlineInput[i] != '|') && (commandlineInput[i+1] != '\0')){

            brokenByPipeCommands[word][letter] = commandlineInput[i];
            letter++;

        }else{

            if(commandlineInput[i+1] == '\0'){

                brokenByPipeCommands[word][letter] = commandlineInput[i];
                letter++;
                brokenByPipeCommands[word][letter] = '\0';
                break;

            }

            brokenByPipeCommands[word][letter] = '\0';

            word++;

            brokenByPipeCommands[word] = (char*)malloc(sizeof(char)*100);

            letter = 0;

            numberCommands++;

        }

        i++;

    }

    numberCommands++;

    return numberCommands;

}

int separateCommandsBySpaceSimple(char* commands, char** brokenCommands){

    /*
    This function separates each simple command into separate commands for execvp.
    It returns the number of words it has.
    It adds a NULL for execvp
    commands - has ls / cat text.txt (not separated big commands)
    brokenCommands - has ls / {cat},{text.txt} (separate commands)
    */

    int i = 0, word = 0;

    char* token = strtok(commands, " ");

    brokenCommands[0] = (char*)malloc(sizeof(char)*100);

    while(token != NULL){

        strcpy(brokenCommands[word], token);

        word++;

        brokenCommands[word] = (char*)malloc(sizeof(char)*100);

        token = strtok(NULL, " ");

    }

    brokenCommands[word] = (char*)malloc(sizeof(char)*100);
    brokenCommands[word] = NULL;

    return word;

}

void add_redirect(int nrsimpleCommands,char** simpleCommands){

    int i = 0, maiMic, maiMare;

    while(i < nrsimpleCommands){

        if(strcmp(simpleCommands[i], ">") == 0){

            i++;
            maiMare = creat(simpleCommands[i], 0644);
            dup2(maiMare, STDOUT_FILENO);
            close(maiMare);

        }else if(strcmp(simpleCommands[i], "<") == 0){

            i++;
            maiMic = open(simpleCommands[i], O_RDONLY);
            dup2(maiMic, STDIN_FILENO);
            close(maiMic);

        }

        i++;

    }

}

void add_redirect_check(int nrsimpleCommands, char** simpleCommands, char* simpleCommandsHolder[]){

    int i = 0;

    while(i < nrsimpleCommands){

        if(strcmp(simpleCommands[i], "<") == 0 || strcmp(simpleCommands[i], ">") == 0)
            break;

        simpleCommandsHolder[i] = simpleCommands[i];

        i++;

    }

    simpleCommandsHolder[i] = NULL;

}

boolean interpretPipes(char** commandLine, int number){

    /*
    This function handles the case when there are pipes in the command line.
    It takes as argument the separated big commands from the input (separated by | )
    Then it creates a process united by a pipe for each separated command get gotten by separation, except the last one, which doesn't need another pipe.
    The parent just waits for the child to finish.
    The main child handles each big command - it creates another process for each, then runs execvp on it, putting the result in the write part of the pipe, so the other process
    will be able to retrieve the information by the read part.
    char** commandLine - each command separated by pipes.
    char* brokenSpace[100] - each baby command separated by spaces for execvp
    number - number of pipes written in input
    */

    int i = 0;

    char* brokenSpace[1000];

    pid_t id = fork();

    if(id < 0){

        perror("\nFailed to create fork.");
        exit(1);

    }else{

        if(id > 0){

            wait(&id);
            return true;

        }else{

            int i;

            for(i = 0; i < number; i++){

                int pip[2];

                if(pipe(pip) < 0){

                    perror("\nFailed to create pipe");
                    exit(2);

                }else{

                    if(fork() == 0){

                        dup2(pip[1], 1);
                        close(pip[0]);

                        int numberSimpleCommands = separateCommandsBySpaceSimple(commandLine[i], brokenSpace);

                        char* redirectCommands[numberSimpleCommands + 1];

                        add_redirect(numberSimpleCommands, brokenSpace); // open the file after <

                        add_redirect_check(numberSimpleCommands, brokenSpace, redirectCommands); // execute what is before <

                        if(execvp(redirectCommands[0], redirectCommands) < 0){

                            perror("\nFailed to execvp");
                            exit(3);

                        }

                    }

                    dup2(pip[0], 0);

                    close(pip[1]);

                    wait(&id);

                }

            }

            int numberSimpleCommands = separateCommandsBySpaceSimple(commandLine[i], brokenSpace);

            char* redirectCommands[numberSimpleCommands + 1];

            add_redirect(numberSimpleCommands, brokenSpace);

            add_redirect_check(numberSimpleCommands, brokenSpace, redirectCommands);

            execvp(redirectCommands[0], redirectCommands);

        }

    }

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
        int numberRedirect = hasRedirect(command);

        if(numberPipes == 0 && numberRedirect == 0){

           separateCommandsBySpaceSimple(command, arguments);

            if(strlen(command) >= 1){

                add_history(command);

                work = interpret(arguments, command);

            }

        }else{

            add_history(command);

            char* brokenByPipeCommands[100];

            int numberBigArguments = separateCommandsByPipe(command, brokenByPipeCommands);

            work = interpretPipes(brokenByPipeCommands, numberPipes);

        }

    }

    free(command);

}
