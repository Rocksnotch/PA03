/**************************/
/* Name: William Youse    */
/* Program: Extended Shell*/
/* Purpose: Extend shell  */
/* project from PA02      */
/**************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include "linkedList.h"

#define MAX_CHAR 1024

//Housekeeping, some local variables that are constantly referenced to

char *currentDir;
char input[MAX_CHAR];
char tempInput[MAX_CHAR];
char backup[MAX_CHAR];
char commandBackup[MAX_CHAR];
char delim[2] = " ";
char *token;
char *command;
char *file;
int argCount = 0;
int redirect = 0;
char *redir_out, *redir_outerr, *redir_outapp, *redir_outerrapp, *redir_in;
char *ipc_next, *ipc_nexterr;
int bgprocess;

int zombies[50];

pid_t cpid, lastCpid;
int stat;

char **args;
char **emptyArgs;
char **envp;

int exitShell = 1;

//End of local variables

//internal commands

void printEnv() { //prints all env variables
    printList();
}

void setEnv() { //create a new env variable
    if (argCount > 3 || argCount == 1) {
        fprintf(stderr, "Invalid argument usage!\n");
        return;
    }

    struct localEnv *temp = find(args[1]);

    if (args[2] == NULL && argCount > 1) {

        if (temp == NULL) {
            insertFirst(args[1], "");
        } else {
            strcpy(temp->envData, "");
        }
    }

    if (args[3] == NULL && argCount > 2) {

        if (temp == NULL) {
            insertFirst(args[1], args[2]);
        } else {
            strcpy(temp->envData, args[2]);
        }
    }
}

void which() { //finds where file is in user path
    char path[MAX_CHAR];
    char pathBackup[MAX_CHAR];
    char pathDelim[2] = ":";
    char *tempPath;


    strncpy(path, find("PATH")->envData, sizeof(find("PATH")->envData));

    token = strtok(path, pathDelim);


    if ((strncmp(args[1], "-a", 2) == 0) && (args[2] != NULL)) {
        while(token != NULL) { //peeking thru all the PATHs
            strncpy(pathBackup, token, MAX_CHAR);

            strcat(pathBackup, "/");
            strcat(pathBackup, args[2]);
            
            if (access(pathBackup, F_OK) == 0) {
                printf("%s\n", pathBackup);
            }

            token = strtok(NULL, pathDelim);
        }
    } else if ((strncmp(args[1], "-a", 2) == 0) && (args[2] == NULL)){
        fprintf(stderr, "which: no file after parameter.\n");
        return;
    } else if (args[1] != NULL) {
        while(token != NULL) { //peeking thru all the PATHs
            strncpy(pathBackup, token, MAX_CHAR);

            strcat(pathBackup, "/");
            strcat(pathBackup, args[1]);
            
            if (access(pathBackup, F_OK) == 0) {
                printf("%s\n", pathBackup);
                return;
            }

            token = strtok(NULL, pathDelim);
        }
    } else {
        return;
    }
}

void listDir() { //no arg, list files in current dir. with arguments, list ALL directories
    DIR *d;
    struct dirent *dir;
    if (args[1] == NULL) {
        d = opendir(".");
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
  } else {
        for (int i = 1; i < argCount; i++) {
            printf("%s:\n", args[i]);
            d = opendir(args[i]);
            while ((dir = readdir(d)) != NULL) {
                printf("%s\n", dir->d_name);
            }
            closedir(d);
        }
    }
    printf("\n");
}

void printPwd() { //print shells pwd
    char tempCWD[MAX_CHAR];
    getcwd(tempCWD, MAX_CHAR);
    printf("The current directory is: %s\n", tempCWD);
}

void killCommand() { //tries to kill PID sent to it
    int tempSig;
    pid_t tempPid;
    

    if (args[1] == NULL) {
        printf("kill: invalid usage.\nCorrect usage: kill [arguments] <pid>...\n\n");
    } else {
        if (args[2] == NULL) {  
            tempPid = atoi(args[1]);
            if (kill(tempPid, SIGTERM) == 0) {
                printf("PID killed successfully!\n");
            } else {
                printf("PID killed unsuccessfully.\n");
            }
        } else {
            tempPid = atoi(args[2]);
            token = strtok(args[1], "-");
            tempSig = atoi(token);

            if (kill(tempPid, tempSig) == 0) {
                printf("PID killed successfully!\n");
            } else {
                printf("PID killed unsuccessfully.\n");
            }
        }
    }
}

void printPid() {
    printf("PID of shell is: %d\n", (int)getpid());
}

void changeDir() {
    if (args[1] == NULL) {
                chdir(find("HOME")->envData);
    } else {
        if (chdir(args[1]) == -1) {
            printf("cd: path '%s' not found.\n", args[1]);
        }
    }
}

void helpCmd() { //simple help command, lets the user know what they can run
    printf("\n\n-------HELP MENU-------\n");
    printf("List of availible commands:\nhelp\ncd\nexit\nkill\npid\npwd\nprintenv\nsetenv\nlist\nwhich\nAnd all availible UNIX commands.\n\n");
}

void zombieCleanupCrew() { //crew that, at call, makes sure ALL zombies are dealt with (or your money back)
    for (int i = 0; i <= 50; i++) {
        if (zombies[i] != 0) {
            if (waitpid(zombies[i], &stat, WNOHANG)) {
                zombies[i] = 0;
            }
        }
    }
}

void fgCommand() { //bring a background program to the foreground
    int status;
    int tempCpid = 0;

    if (args[1] == NULL) {
        if (lastCpid != 0) {
            printf("Bringing PID %d to foreground and waiting for finish.\n", lastCpid);
            waitpid(lastCpid, &status, WUNTRACED);
        } else {
            return;
        }
    } else {
        for (int i = 0; i <= 50; i++) {
            if (zombies[i] == atoi(args[1])) {
                tempCpid = zombies[i];
                zombies[i] = 0;
                break;
            }
        }
        if (tempCpid == 0) {
            printf("fg: no cpid with that ID found.\n");
            return;
        } else {
            printf("Bringing PID %d to foreground and waiting for finish.\n", tempCpid);
            waitpid(tempCpid, &status, WUNTRACED);
        }
    }
}

//!!!END OF INTERNAL COMMANDS!!!

//This section defines rest of working commands (parser, etc)

void parser() {
    
    //check if there is anything to do with the redirect here
    redir_out = strstr(input, ">");
    redir_outerr = strstr(input, ">&");
    redir_outapp = strstr(input, ">>");
    redir_outerrapp = strstr(input, ">>&");
    redir_in = strstr(input, "<");

    if (redir_outerrapp != NULL) {
        redirect = 1;
    } else if (redir_outapp != NULL) {
        redirect = 1;
    } else if (redir_outerr != NULL) {
        redirect = 1;
    } else if (redir_out != NULL) {
        redirect = 1;
    } else if (redir_in != NULL) {
        redirect = 1;
    }

    //check if there is anything to do with IPC

    ipc_next = strstr(input, "|");
    ipc_nexterr = strstr(input, "|&");

    //check if this will be a background process or not

    char tempBgCheck = input[strlen(input) - 2];
    char bgSym = '&';
    if (tempBgCheck == bgSym) {
        bgprocess = 1;
    }

    //Done! Lets parse the rest...

    input[strnlen(input, MAX_CHAR) - 1] = 0;
    strncpy(backup, input, sizeof(input));

    token = strtok(input, delim);

    if (redirect) { //if we are redirecting, we are going to handle the args a bit differently
        while (token != NULL) {
            if ((strcmp(token, "<") == 0) || (strcmp(token, ">>&") == 0) || (strcmp(token, ">>") == 0) || (strcmp(token, ">&") == 0) || (strcmp(token, ">") == 0) || (strcmp(token, "|") == 0) || (strcmp(token, "|&") == 0)) {
                break; //basically, if the token actually is the redirect symbol/IPC then we want to end the count here...
            } else {
                argCount++; //...otherwise, continue to count
                token = strtok(NULL, delim);
            }
        }

        strncpy(input, backup, sizeof(backup)); //bring the backup back into input

        token = strtok(input, delim);

        while(token != NULL) {
            file = token;
            token = strtok(NULL, delim);
        }

        token = strtok(backup, delim);
        command = token;

        args = malloc((argCount) * sizeof(char *)); //size of each reference

        for (int i = 0; i <= argCount; i++) {
            args[i] = malloc(MAX_CHAR * sizeof(char)); //size of the data IN reference
        }

        for (int j = 0; j <= argCount; j++) {
            if (j == argCount) {
                args[j] = NULL; //if its the end of the count, null terminated!
            } else {
                strcpy(args[j], token);
                token = strtok(NULL, delim);
            }
        }

    } else {
        while(token != NULL) { //using this to grab how many tokens there actually are
                argCount++;
                token = strtok(NULL, delim);
        }
    
        token = strtok(backup, delim);
        command = token;

        args = malloc((argCount) * sizeof(char *)); //size of each reference

        for (int i = 0; i <= argCount; i++) {
            args[i] = malloc(MAX_CHAR * sizeof(char)); //size of the data IN reference
        }

        for (int j = 0; j <= argCount; j++) {
            if (j == argCount) {
                args[j] = NULL; //if its the end of the count, null terminated!
            } else {
                strcpy(args[j], token);
                token = strtok(NULL, delim);
            }
        }

        if (bgprocess == 1) {
            args[argCount - 1] = NULL;
            free(args[argCount]);
            argCount = argCount - 1;
        }
    }

    //lets turn the env variables into an array of pointers to strings...

    struct localEnv *current = returnHead();
    char tempEnv[MAX_CHAR];

    envp = malloc(length() * sizeof(char)); //size of each reference

    for (int i = 0; i <= length(); i++) {
        envp[i] = malloc(MAX_CHAR * sizeof(char));
    }

    for (int j = 0; j < length(); j++) {
        strcpy(tempEnv, current->envName);
        strcat(tempEnv, "=");
        strcat(tempEnv, current->envData);
        strcpy(envp[j], tempEnv);

        current = current->next;
    }

    envp[length()] = NULL;

    //Done with env! Lets check to see if it is a program we can call internally...

    zombieCleanupCrew(); //secret zombie cleanup crew!

    if (strcmp(command, "setenv") == 0) {
        setEnv();
    } else if (strcmp(command, "printenv") == 0) {
        printEnv();
    } else if ((strcmp(command, "exit") == 0) || (strcmp(command, "logout") == 0) || (strcmp(command, "quit") == 0)) {
        printf("Exiting shell, goodbye!\n");
        exitShell = 0;
    } else if (strcmp(command, "pwd") == 0) {
        printPwd();
    } else if (strcmp(command, "cd") == 0) {
        changeDir();
    } else if (strcmp(command, "kill") == 0) {
        killCommand();
    } else if (strcmp(command, "list") == 0) {
        listDir();
    } else if (strcmp(command, "which") == 0) {
        which();
    } else if (strcmp(command, "pid") == 0) {
        printPid();
    } else if (strcmp(command, "help") == 0) {
        helpCmd();
    } else if (strcmp(command, "fg") == 0) {
        fgCommand();
    } else if ((strncmp(command, "/", 1) == 0) || (strncmp(command, "./", 2) == 0) || (strncmp(command, "../", 3) == 0)) {
        
        if (access(command, F_OK && X_OK) == 0) {
            int status;

            if ((cpid = fork()) == 0) { //child process
                    if (redir_outerrapp != NULL) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                        close(1);
                        close(2);
                        dup(fd);
                        dup(fd);

                        execve(command, args, envp);

                        exit(0);
                    } else if (redir_outapp != NULL) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                        close(1);
                        dup(fd);

                        execve(command, args, envp);

                        exit(0);
                    } else if (redir_outerr != NULL) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        close(1);
                        close(2);
                        dup(fd);
                        dup(fd);

                        execve(command, args, envp);

                        exit(0);
                    } else if (redir_out != NULL) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        close(1);
                        dup(fd);

                        execve(command, args, envp);

                        exit(0);
                    } else if (redir_in != NULL) {
                        int fd = open(file, O_RDONLY | O_CREAT, 0644);
                        close(0);
                        dup(fd);

                        execve(command, args, envp);

                        exit(0);
                    } else {
                        execve(command, args, envp);
                        exit(0);
                    }

                    

                } else { //parent process

                    if (bgprocess == 1) {
                        if (waitpid(cpid, &status, WNOHANG) == 0) {
                            lastCpid = cpid;
                            for (int i = 0; i <= 50; i++) {
                                if (zombies[i] == 0) {
                                    zombies[i] = cpid;
                                    break;
                                }
                            }
                        }
                        sleep(1);
                    } else {
                        waitpid(cpid, &status, WUNTRACED);
                    }
                }
        }
    } else { //basically checking if this command exists in the PATH

        char path[MAX_CHAR];
        char pathBackup[MAX_CHAR];
        char pathDelim[2] = ":";
        char *tempPath;

        strncpy(path, find("PATH")->envData, sizeof(find("PATH")->envData));

        token = strtok(path, pathDelim);

        while(token != NULL) { //peeking thru all the PATHs
            strncpy(pathBackup, token, MAX_CHAR);

            strcat(pathBackup, "/");
            strcat(pathBackup, args[0]);
            
            if (access(pathBackup, F_OK && X_OK) == 0) { //if the command is valid, found in the path, and can be executed
                strcpy(commandBackup, pathBackup);
                int status;

                if ((cpid = fork()) == 0) { //child process
                    
                    if (redir_outerrapp != NULL) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                        close(1);
                        close(2);
                        dup(fd);
                        dup(fd);

                        execve(commandBackup, args, envp);

                        exit(0);
                    } else if (redir_outapp != NULL) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                        close(1);
                        dup(fd);

                        execve(commandBackup, args, envp);

                        exit(0);
                    } else if (redir_outerr != NULL) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        close(1);
                        close(2);
                        dup(fd);
                        dup(fd);

                        execve(commandBackup, args, envp);

                        exit(0);
                    } else if (redir_out != NULL) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        close(1);
                        dup(fd);

                        execve(commandBackup, args, envp);

                        exit(0);
                    } else if (redir_in != NULL) {
                        int fd = open(file, O_RDONLY | O_CREAT, 0644);
                        close(0);
                        dup(fd);

                        execve(commandBackup, args, envp);

                        exit(0);
                    } else {
                        execve(commandBackup, args, envp);
                        exit(0);
                    }
                } else { //parent process

                    if (bgprocess == 1) {
                        if (waitpid(cpid, &status, WNOHANG) == 0) {
                            lastCpid = cpid;
                            for (int i = 0; i <= 50; i++) {
                                if (zombies[i] == 0) {
                                    zombies[i] = cpid;
                                    break;
                                }
                            }
                        }
                        sleep(1);
                    } else {
                        waitpid(cpid, &status, WUNTRACED);
                    }
                }
                break;
            }

            token = strtok(NULL, pathDelim);
        }
    }

    //cleanup at the end, lets set everything back to the way it was
    redir_out = redir_outerr = redir_outapp = redir_outerrapp = redir_in = 0;
    ipc_next = ipc_nexterr = 0;
    bgprocess = 0;
    argCount = 0;

    for (int k = 0; k < argCount; k++) { //delete all the args
            free(args[k]);
    }

    free(args);
}

//!!!END OF BULK COMMANDS!!!
 
int main() {
    
    //initial setup of local environment
    insertFirst("USER", getenv("USER"));
    insertFirst("HOME", getenv("HOME"));
    insertFirst("PATH", getenv("PATH"));
    insertFirst("DISPLAY", getenv("DISPLAY"));

    chdir(find("HOME")->envData);

    printf("---Welcome to William's Shell!---\n");
    printf("---(Try typing 'help' to see availiable commands)---\n");
    printf("---NOT ALL FUNCTIONS MAY WORK : USE AT OWN RISK---\n\n\n");


    while (exitShell) {

    printf("%s@%s> ", find("USER")->envData, getcwd(currentDir, MAX_CHAR));
    fgets(input, MAX_CHAR, stdin);
         
        if (input[0] == '\n') {
                //dont do anything if input is empty
            } else {
                parser();
            }
    }

    //Housekeeping, freeing memory
    deleteAll(); //freeing local environment memory
    zombieCleanupCrew();
    return 0;
}