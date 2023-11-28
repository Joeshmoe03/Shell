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

char** parseInp(char* shellInp, int* countAddr) {
	
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
	*countAddr = tokenIdx;
	return tokens;
}


int main(int argc, char *argv[]) {
	char* s = readInp();
	int count = 0;
	char** parsed_s = parseInp(s, &count);
	int i;

	for(i = 0; i < count; i++) {
		printf("%s\n", parsed_s[i]);
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
