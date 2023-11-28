#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define INPUT_LIMIT 4096
#define COMMAND_LIMIT 11

int main(int argc, char *argv[]) {
	
	/* function for error checking */

	/* function for tokenizing? */

	/* infinite loop b/cuz the shell is always listening (1984 style)*/
	while(1){
		/* print statement for our shell */
		printf("\nThis is our shell: ");

		/* get what they give us using fgets & */
		char shellinput[INPUT_LIMIT];
		fgets(shellinput, INPUT_LIMIT, stdin); //TODO: error check needed? or check that COMMAND_LIMIT is not exceeed

		/* replace \n character of input with null byte */
		int cmdlen = strlen(shellinput);
		shellinput[cmdlen-1] = '\0';

		/* exit if desired */
		if(strcmp(shellinput, "exit") == 0 || feof(stdin)) {
			exit(0);
		}		
		
		/* let's parse with tokenize */

		/* create "array" of strings for nonsymbol; will be combined later with symbols for desired behavior */
		char **nonsymbol = malloc(INPUT_LIMIT * sizeof(char*)); //should it be 10?
		
		/* populate it */
		nonsymbol[0] = strdup(strtok(shellinput, " "));
		printf("%s", nonsymbol[0]);

		int i = 1;
		char *subtoken;
		while ((subtoken = strtok(NULL, " ")) != NULL) { //Could also do wiht while loop till strktok() returns NULL 
			nonsymbol[i] = subtoken;
			i++;
			/* will have to add a LOT of checks for symbols and add logic for how identifying these symbols will affect the nonsymbol "array"  */
		}

		/* let's build shell functionality for basic programs */
		
		/* fork */
		pid_t child_pid, exit_pid;
		int exit_value;

		child_pid = fork();

		/* child will do all the work*/
		if(child_pid == 0) {
			execvp(nonsymbol[0], nonsymbol);
		} else { /* parent will wait for child to be done */
			exit_pid = wait(&exit_value);
		}
	}
	return 0;
}