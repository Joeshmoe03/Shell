#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* We are permitted to assume 4096 as max char num as input */
#define COMMAND_LIMIT 4096
#define TOKEN_DELIM " \t\n"

char* readInp(void) {

	/* Buffer for shoving our command */
	int shellInpSiz = COMMAND_LIMIT;
	char* shellInp = malloc(sizeof(char) * shellInpSiz);
	if(shellInp == NULL) {
		return NULL;
	}
	
	/* Where we listen for input indefinitely until either the user does something */
	while(1) {
		printf("$ ");

		/* Attempt to get shell input */
		fgets(shellInp, shellInpSiz, stdin);
		if(s == NULL) {
			free(shellInp);
			return NULL;
		}

		/* If user specified exit, no need to do anything else */ //TODO: should we do parsing before exit???
		if(strcmp(shellInp, "exit" == 0 || feof(stdin))) {
			free(shellInp);
			exit(0);
		}
		return shellInp;
	}
}

char* parseInp(char* shellInp) {
	
	/* Assumed initial number of tokens that will be parsed. Realloc if necessary. */
	int tokenIdx = 0;
	int tokenNum = 20;
	char* token;
	char** tokens = malloc(sizeof(char*) * tokenNum); 
	if(tokens == NULL) {
		return NULL;
	}
	
	/* It's token time >:D */
	token = strtok(shellInp, TOKEN_DELIM);
	while(token != NULL) {
		//TODO: do something where I put token in tokens and realloc if need be
		
		/* malloc of token is +1 for \0 */
		tokens[tokenIdx] = malloc(sizeof(char) * (strlen(token) + 1));
		//do NULL check
		//strcpy
		token = strtok(NULL, TOKEN_DELIM);
		tokenIdx++;
	}
	return tokens;
}


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
