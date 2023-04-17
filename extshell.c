#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include "linkedList.h"

#define MAX_CHAR 255

//Housekeeping, some local variables that are constantly referenced to

char *currentDir;
char input[MAX_CHAR];

//End of local variables


//This section defines all internal commands

void pidCommand() { //prints the shells PID
    printf("PID of shell is: %d\n", (int)getpid());
}

void pwdCommand() {
    char *tempCWD;
    getcwd(tempCWD, MAX_CHAR);
    printf("The current directory is: %s\n", tempCWD);
}

//!!!END OF INTERNAL COMMANDS!!!

//This section defines rest of working commands (parser, etc)

void parser() {

   //check if there is anything to do with the pipes here
   char *redir_out = strstr(input, ">");
   char *redir_outerr = strstr(input, ">&");
   char *redir_outapp = strstr(input, ">>");
   char *redir_outerrapp = strstr(input, ">>&");
   char *redir_in = strstr(input, "<");

   if (redir_out) {

   } else if (redir_outerr) {

   } else if (redir_outapp) {

   } else if (redir_outerrapp) {

   } else if (redir_in) {
      
   }

}

//!!!END OF BULK COMMANDS!!!

void main() {
    
   //initial setup of local environment
   insertFirst("USER", getenv("USER"));
   insertFirst("HOME", getenv("HOME"));
   insertFirst("PATH", getenv("PATH"));

   chdir(find("HOME")->envData);

   printf("%s@%s > ", find("USER")->envData, getcwd(currentDir, MAX_CHAR));
   fgets(input, MAX_CHAR, stdin);

    if (input[0] == '\n') {
            //dont do anything if input is empty
        } else {
        
        parser();
        }

    //Housekeeping, freeing memory
    deleteAll(); //freeing local environment memory
}