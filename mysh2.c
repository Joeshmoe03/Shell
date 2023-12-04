#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

/* We are permitted to assume 4096 as max char num as input */
#define INPUT_LIMIT 4096
#define COMMAND_LIMIT 10
#define TOKEN_DELIM " \t\n\0" 

char* readInp(void) {
	
	/* Buffer for shoving our command */
	int shellInpSiz = INPUT_LIMIT;
	char* shellInp = malloc(sizeof(char) * shellInpSiz);
	if(shellInp == NULL) {
		return NULL;
	}
	printf("\n$ ");

	/* Attempt to get shell input & handles failure case: if at eof we could get null, and if error we also get null, thus we need errno */
	if(fgets(shellInp, shellInpSiz, stdin) == NULL) {
	
		/* A problem occured with fgets */
		if(errno != 0) {
			free(shellInp);
			exit(1);
		}
		/* User did CTRL-D */
		else {
			free(shellInp);
			exit(0);
		}
	}
	return shellInp;
}

char** parseInp(char* shellInp, int* tokenCntAddr) {
	
	/* Assumed initial number of tokens that will be parsed. Realloc if necessary. */
	int tokenIdx = 0;
	int tokenSiz = 20;
	int tmpTokenSiz;
	char* token;
	char** tmpTokens;
	char** tokens = malloc(sizeof(char*) * tokenSiz); 
	if(tokens == NULL) {
		return NULL;
	}
	
	/* It's token time >:D */
	token = strtok(shellInp, TOKEN_DELIM);
	while(token != NULL) {
		*tokenCntAddr = tokenIdx;

		/* Realloc tokens if necessary */
		if(tokenIdx > tokenSiz) {
			tmpTokenSiz = tokenSiz * 2;
			tmpTokens = realloc(tokens, sizeof(char*) * tmpTokenSiz);
			if(tmpTokens == NULL) {
				for(tokenIdx = 0; tokenIdx <= tokenSiz; tokenIdx++) {
					free(tokens[tokenIdx]);
				}
				free(tokens);
				return NULL;
			}
			tokens = tmpTokens;
			tokenSiz = tmpTokenSiz;
		}
	
		/* malloc of token is +1 for \0 */
		tokens[tokenIdx] = malloc(sizeof(char) * (strlen(token) + 1));
		
		/* If the malloc fails, free everything and return */
		if(tokens[tokenIdx] == NULL) {
			for(tokenIdx = 0; tokenIdx <= tokenSiz; tokenIdx++) {
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

int forkPipe(int fdin, int fdout, char** commands) {
	int exitStatus;
	pid_t pid;

	if((pid = fork()) == 0) {
		if(fdin != STDIN_FILENO) {
			dup2(fdin, STDIN_FILENO);
			close(fdin);
		}
		if(fdout != STDOUT_FILENO) {
			dup2(fdout, STDOUT_FILENO);
			close(fdout);
		}
		if (execvp(commands[0], commands) == -1) {
			return -1;
		}
	} 
	else if (pid < 0) {
		return -1;
	} 
	else {
		waitpid(pid, &exitStatus, 0);
	}
	return pid;
}

int redirectIn(char* file) {
	int fdin = open(file, O_RDONLY);
	
	/* Check that opening file succeeded else throw error */
	if(fdin == -1) {
		perror("open");
		return -1;
	}
	return fdin;
}

int redirectOut(char* file, int isAppend) {
	int fdout;
	
	/* User specifies to append rather than trunc */
	if(isAppend == 1) {
		fdout = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666); 
	} 
	
	/* Truncate otherwise */
	else {
		fdout = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	}
	
	/* Check that open succeeded else throw error */
	if(fdout == -1) {
		perror("open");
		return -1;
	}
	return fdout;
}

int main(int argc, char *argv[]) {
	int tokenCnt;
	int tokenIdx;
	char* shellInp;
	char** tokens;
	char** commands;
	int commandsIdx;
	char* file;
	int fd[2];
	int fdin;
	int fdout;

	while(1) {	

		/* Read and parse input */
		tokenCnt = 0;
		tokenIdx = 0;
		shellInp = readInp();
		if(strcmp(shellInp, "\n") == 0) {
			free(shellInp);
			continue;
		}
		tokens = parseInp(shellInp, &tokenCnt);
		if(tokens == NULL) {
			break;
		}
	
		/* Prep for iterating over tokens and placing in buffer to execute, ready for piping */
		commands = malloc(sizeof(char*) * (COMMAND_LIMIT + 1));
		if(commands == NULL) {
			break;
		}
		commandsIdx = 0;
		fdin = STDIN_FILENO;
		fdout = STDOUT_FILENO;

		/* User specifies to exit so exit after freeing */
		if(strcmp(tokens[0], "exit") == 0) {
			for(tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
				free(tokens[tokenIdx]);
			}
			free(shellInp);
			free(tokens);
			free(commands);
			exit(0);
		}
	
		/* Get corresponding tokens and do the right things on those tokens between pipes/redirects */
		for(tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			file = tokens[tokenIdx + 1];
			if(strcmp(tokens[tokenIdx], "<") == 0) {
				commands[commandsIdx] = NULL; //TODO: ERROR HANDLING WITH FREE
				fdin = redirectIn(file);
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], ">") == 0) {
				commands[commandsIdx] = NULL;
				fdout = redirectOut(file, 0); //TODO: ERROR HANDLING WITH FREE
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], ">>") == 0) {
				commands[commandsIdx] = NULL;
				fdout = redirectOut(file, 1); //TODO: ERROR HANDLING WITH FREE
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], "|") == 0) {
				commands[commandsIdx] = NULL;
				pipe(fd);
				forkPipe(fdin, fd[1], commands); //TODO: ERROR HANDLING WITH FREE
				close(fd[1]);
				fdin = fd[0];
				commandsIdx = 0;
			}
			else {
				commands[commandsIdx] = tokens[tokenIdx];
				commandsIdx++;
			}
		}
		commands[commandsIdx] = NULL;
		forkPipe(fdin, fdout, commands); //TODO: ERROR HANDLING WITH FREE
		if(fdout != STDOUT_FILENO) {
			close(fdout);
		}	
		if(fdin != STDIN_FILENO) {
			close(fdin);
		}

		/* Free at the end */	
		for(tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			free(tokens[tokenIdx]);
		}
		free(shellInp);
		free(tokens);
		free(commands);
	}

	/* If we break out of loop we go here and indicate failure */
	for(tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
		free(tokens[tokenIdx]);
	}
	free(shellInp);
	free(tokens);
	free(commands);
	exit(1);
}
