#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* We are permitted to assume 4096 as max char num as input */
#define COMMAND_LIMIT 4096
#define TOKEN_DELIM " \t\n\0"

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
		if(shellInp == NULL) {
			free(shellInp);
			return NULL;
		}
		return shellInp;
	}
}

char** parseInp(char* shellInp, int* tokenCntAddr) {
	
	/* Assumed initial number of tokens that will be parsed. Realloc if necessary. */
	int tokenIdx = 0;
	int tmpTokenNum;
	int tokenNum = 20;
	char* token;
	char** tmpTokens;
	char** tokens = malloc(sizeof(char*) * tokenNum); 
	if(tokens == NULL) {
		return NULL;
	}
	
	/* It's token time >:D */
	token = strtok(shellInp, TOKEN_DELIM);
	while(token != NULL) {
		*tokenCntAddr = tokenIdx;

		/* Realloc tokens if necessary */
		if(tokenIdx > tokenNum) {
			tmpTokenNum = tokenNum * 2;
			tmpTokens = realloc(tokens, sizeof(char*) * tmpTokenNum);
			if(tmpTokens == NULL) {
				for(tokenIdx = 0; tokenIdx <= tokenNum; tokenIdx++) {
					free(tokens[tokenIdx]);
				}
				free(tokens);
				return NULL;
			}
			tokens = tmpTokens;
			tokenNum = tmpTokenNum;
		}
	
		/* malloc of token is +1 for \0 */
		tokens[tokenIdx] = malloc(sizeof(char) * (strlen(token) + 1));
		
		/* If the malloc fails, free everything and return */
		if(tokens[tokenIdx] == NULL) {
			for(tokenIdx = 0; tokenIdx <= tokenNum; tokenIdx++) {
				free(tokens[tokenIdx]);
			}
			free(tokens);
			return NULL;
		}
		strcpy(tokens[tokenIdx], token);
		token = strtok(NULL, TOKEN_DELIM);
		tokenIdx++;
	}
	return tokens;
}

/* function for input redirection */
void inputredirect(char **tokens) {
	__pid_t child_pid, exit_pid;
	int exit_value;

	child_pid = fork();

	/* child does all the work */
	if(child_pid == 0) {
		/*if multiple redirects (<) call program on each one */
		/* if multiple things to redirect, it ignores the first redirect? */
		
	} else { /* parent waits for child to be done*/
		exit_pid = wait(&exit_value);
	}	
}



/* basic functionality */
void basicfunc(char **tokens) {
			__pid_t child_pid, exit_pid;
	int exit_value;

	child_pid = fork();

	/* child does all the work */
	if(child_pid == 0) {
		execvp(tokens[0], tokens);
		
	} else { /* parent waits for child to be done*/
		exit_pid = wait(&exit_value);
	}	
}


int main(int argc, char *argv[]) {
	char* shellInp = readInp();
	int tokenCnt = 0;
	char** tokens = parseInp(shellInp, &tokenCnt);
	int i;

		basicfunc(tokens);

	for(i = 0; i <= tokenCnt; i++) {
		//if token == "|"  -> do thing with all previous tokens since last thing done?
		//if token == ">>" ->
		//if token == ">"  ->
		//if token == "<"  ->
		//exec the shell again
														//printf("%s\n", tokens[i]);
	}	

	/* function for error checking */

	/* function for tokenizing? */

	/* infinite loop b/cuz the shell is always listening (1984 style)*/
	//while(1){
	//	/* print statement for our shell */
	//	printf("This is our shell: ");

	//	/* get what they give us using fgets & */
	//	char shellinput[COMMAND_LIMIT]; 
	//	fgets(shellinput, COMMAND_LIMIT, stdin); //TODO: error check needed? or check that COMMAND_LIMIT is not exceeed

	//	/* replace \n character of input with null byte */
	//	int cmdlen = strlen(shellinput);
	//	shellinput[cmdlen-1] = '\0';

	//	/* exit if desired */
	//	if(strcmp(shellinput, "exit") == 0 || feof(stdin)) {
	//		exit(0);
	//	}

	//	/* let's parse with tokenize */
	//	// while(strtok()) {
	//	// 	printf("in dev...");
	//	// }
	//}


	///* Fork, exec, wait, dup */

	///* What we pass to exec() */

	///* use strtok() to parse argv*/
	///* First argv is the program */
	///* The rest are command line args to pass to the programs*/
	return 0;

}
