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
			exit(EXIT_FAILURE);
		}
		/* User did CTRL-D */
		else {
			free(shellInp);
			exit(EXIT_SUCCESS);
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
	
		/* Malloc of token is +1 for \0 */
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

/* Error Check Functions! */
void forkErrChk(int childPid, char **tokens, int tokenCnt, char *shellInp, char **commands) {

	if(childPid < 0) {
		/* Free allocs */
		for(int tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			free(tokens[tokenIdx]);
		}
		free(shellInp);
		free(tokens);
		free(commands);

	/* Error message & exit */
		perror("fork");
		exit(EXIT_FAILURE);
	}
}

void execErrChk(char **tokens, int tokenCnt, char *shellInp, char **commands) {

	/* Free allocs */
	for(int tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
		free(tokens[tokenIdx]);
	}
	free(shellInp);
	free(tokens);
	free(commands);

	/* Error message & exit */
	perror("mysh");
	exit(EXIT_FAILURE);
}

void waitErrChk(int exitPid, char **tokens, int tokenCnt, char *shellInp, char **commands) {
	if(exitPid < 0) {

		/* Free allocs */
		for(int tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			free(tokens[tokenIdx]);
		}
		free(shellInp);
		free(tokens);
		free(commands);

		/* Error message & exit */
		perror("wait");
		exit(EXIT_FAILURE);
	}
}

void dup2ErrChk(int newFd, char **tokens, int tokenCnt, char *shellInp, char **commands) {
	if(newFd < 0) {

		/* Free allocs */
		for(int tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			free(tokens[tokenIdx]);
		}
		free(shellInp);
		free(tokens);
		free(commands);

		/* Error message & exit */
		perror("dup2");
		exit(EXIT_FAILURE);
	}
}

void pipeWithErrChk(int retVal, char **tokens, int tokenCnt, char *shellInp, char **commands) {
	if(retVal < 0) {

		/* Free allocs */
		for(int tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			free(tokens[tokenIdx]);
		}
		free(shellInp);
		free(tokens);
		free(commands);

		/* Error message & exit */
		perror("pipe");
		exit(EXIT_FAILURE);
	}
}

void openErrChk(int fd, char **tokens, int tokenCnt, char *shellInp, char **commands) {
	if(fd < 0) {

		/* Free allocs */
		for(int tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			free(tokens[tokenIdx]);
		}
		free(shellInp);
		free(tokens);
		free(commands);

		/* Error message & exit */
		perror("mysh");
		exit(EXIT_FAILURE);
	}
}

void closeWithErrChk(int closeRetVal, char **tokens, int tokenCnt, char *shellInp, char **commands) {
	if(closeRetVal < 0) {

		/* Free allocs */
		for(int tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			free(tokens[tokenIdx]);
		}
		free(shellInp);
		free(tokens);
		free(commands);

		/* Error message & exit */
		perror("mysh");
		exit(EXIT_FAILURE);
	}
}

void forkPipe(int fdin, int fdout, char **tokens, int tokenCnt, char *shellInp, char **commands) {
	__pid_t childPid, exitPid;
	int exitVal;
	int newFd;

	childPid = fork();
	forkErrChk(childPid, tokens, tokenCnt, shellInp, commands);
	if(childPid == 0) {
		if(fdin != STDIN_FILENO) {
			newFd = dup2(fdin, STDIN_FILENO);
			dup2ErrChk(newFd, tokens, tokenCnt, shellInp, commands);
			close(fdin);
		}
		if(fdout != STDOUT_FILENO) {
			newFd = dup2(fdout, STDOUT_FILENO);
			dup2ErrChk(newFd, tokens, tokenCnt, shellInp, commands);
			close(fdout);
		}
		execvp(commands[0], commands);
		execErrChk(tokens, tokenCnt, shellInp, commands);
	} 
	exitPid = wait(&exitVal);
	waitErrChk(exitPid, tokens, tokenCnt, shellInp, commands);
}

int redirectIn(char* file, char **tokens, int tokenCnt, char *shellInp, char **commands) {

	int fdin = open(file, O_RDONLY);
	openErrChk(fdin, tokens, tokenCnt, shellInp, commands);
	
	return fdin;
}

int redirectOut(char* file, int isAppend, char **tokens, int tokenCnt, char *shellInp, char **commands) {
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
	openErrChk(fdout, tokens, tokenCnt, shellInp, commands);

	return fdout;
}

int main(int argc, char *argv[]) {
	
	/* Raw input */
	char* shellInp;

	/* Parsed buffer tracking vars */
	int tokenCnt;
	int tokenIdx;
	char** tokens;
	
	/* Buffer to hold things being executed at given time */
	char** commands;
	int commandsIdx;
	
	/* File involved in redirection */
	char* file;

	/* Fd for our piping and redirection purposes */
	int fd[2];
	int fdin;
	int fdout;

	while(1) {	

		/* Read and parse input after reset of tracking vars */
		tokenCnt = 0;
		tokenIdx = 0;
		shellInp = readInp();

		/* Rerun fgets again cuz user didn't do anything with \n */
		if(strcmp(shellInp, "\n") == 0) {
			free(shellInp);
			continue;
		}

		/* Parse user input */
		tokens = parseInp(shellInp, &tokenCnt);
		if(tokens == NULL) {
			break;
		}
	
		/* Prep for iterating over tokens and placing in buffer to execute, ready for piping */
		commandsIdx = 0;
		commands = malloc(sizeof(char*) * (COMMAND_LIMIT + 1));
		if(commands == NULL) {
			break;
		}

		/* Our initial piping ends */
		fdin = STDIN_FILENO;
		fdout = STDOUT_FILENO;

		/* User specifies to exit, so exit after freeing */
		if(strcmp(tokens[0], "exit") == 0) {
			for(tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
				free(tokens[tokenIdx]);
			}
			free(shellInp);
			free(tokens);
			free(commands);
			exit(EXIT_SUCCESS);
		}
	
		/* Get corresponding tokens and do the right things on those tokens between pipes/redirects */
		for(tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			file = tokens[tokenIdx + 1];
			if(strcmp(tokens[tokenIdx], "<") == 0) {
				commands[commandsIdx] = NULL;
				fdin = redirectIn(file, tokens, tokenCnt, shellInp, commands);
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], ">") == 0) {
				commands[commandsIdx] = NULL;
				fdout = redirectOut(file, 0, tokens, tokenCnt, shellInp, commands);
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], ">>") == 0) {
				commands[commandsIdx] = NULL;
				fdout = redirectOut(file, 1, tokens, tokenCnt, shellInp, commands);
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], "|") == 0) {
				commands[commandsIdx] = NULL;
				pipeWithErrChk(pipe(fd), tokens, tokenCnt, shellInp, commands);
				forkPipe(fdin, fd[1], tokens, tokenCnt, shellInp, commands);
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
		forkPipe(fdin, fdout, tokens, tokenCnt, shellInp, commands); //TODO: ERROR HANDLING WITH FREE

		/* Close opened file descriptors */
		if(fdout != STDOUT_FILENO) {
			closeWithErrChk(close(fdout), tokens, tokenCnt, shellInp, commands);
		}	
		if(fdin != STDIN_FILENO) {
			closeWithErrChk(close(fdout), tokens, tokenCnt, shellInp, commands);
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
	exit(EXIT_FAILURE);
}

