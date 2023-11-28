#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define COMMAND_LIMIT 4096

int main(int argc, char *argv[]) {


	/* function for error checking */

	/* function for tokenizing? */

	/* infinite loop b/cuz the shell is always listening (1984 style)*/
	while(1){
		/* print statement for our shell */
		printf("This is our shell: ");

		/* get what they give us using fgets & */
		char shellinput[COMMAND_LIMIT]; 
		fgets(shellinput, COMMAND_LIMIT, stdin); //TODO: error check needed? or check that COMMAND_LIMIT is not exceeed

		/* replace \n character of input with null byte */
		int cmdlen = strlen(shellinput);
		shellinput[cmdlen-1] = '\0';

		/* exit if desired */
		if(strcmp(shellinput, "exit") == 0 || feof(stdin)) {
			exit(0);
		}

		/* let's parse with tokenize */
		// while(strtok()) {
		// 	printf("in dev...");
		// }
	}


	/* Fork, exec, wait, dup */

	/* What we pass to exec() */

	/* use strtok() to parse argv*/
	/* First argv is the program */
	/* The rest are command line args to pass to the programs*/
	return 0;
}