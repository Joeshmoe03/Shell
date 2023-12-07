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

/* Goes off and forks new process for execution of a command, parent waits on forked child */
int execCommand(int fdin, int fdout, char **commands) {
	__pid_t childPid;

	childPid = fork();
	if(childPid < 0) {
		perror("fork");
		return -1;
	}

	/* Execution of child is under this condition */
	if(childPid == 0) {
	
		/* Input comes from fd specified by previous program and stdin if no piping/first executed.
 		 * Input goes from fdin to stdin and closes fdin */
		if(fdin != STDIN_FILENO) {
			if(dup2(fdin, STDIN_FILENO) < 0) {
				perror("dup2");
				exit(EXIT_FAILURE);
			}
			close(fdin);
		}

		/* Output comes from fd specified by previous program and stdout if no piping/first executed.
 		 * Out goes from fdout to stdout and closes fdout */
		if(fdout != STDOUT_FILENO) {
			if(dup2(fdout, STDOUT_FILENO) < 0) {
				perror("dup2");
				exit(EXIT_FAILURE);
			}
			close(fdout);
		}

		/* If exec (attempts to execute commands) in child fails, we MUST exit... We don't want to run rest of main... */
		if(execvp(commands[0], commands) < 0) {
			perror("mysh");
			exit(EXIT_FAILURE); 
		}
	} 
	return 0;
}

int redirectIn(char* file) {
	int fdin = open(file, O_RDONLY);
	if (fdin < 0) {
		perror("mysh");
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
	if (fdout < 0) {
		perror("mysh");
		return -1;
	}
	return fdout;
}

/* Free allocs & exit */
void freeAll(char **tokens, int tokenCnt, char *shellInp, char **commands) {
	for(int tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
		free(tokens[tokenIdx]);
	}
	free(shellInp);
	free(tokens);
	free(commands);
}

/* Close all fd */
void closeAll(int fdin, int fdout) {
	if(fdin != STDIN_FILENO) {
		if(close(fdin) < 0) {
			perror("close");
		}
	}
	if(fdout != STDOUT_FILENO) {
		if(close(fdout) < 0) {
			perror("close");
		}
	}
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
	int fdin = STDIN_FILENO;
	int fdout = STDOUT_FILENO;

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

		/* User specifies to exit, so exit after freeing */
		if(strcmp(tokens[0], "exit") == 0) {
			freeAll(tokens, tokenCnt, shellInp, commands);
			exit(EXIT_SUCCESS);
		}

		/* Our initial piping ends */
		fdin = STDIN_FILENO;
		fdout = STDOUT_FILENO;

		/* Info needed for waiting on children processes */
		__pid_t exitPid; 
		int exitVal;
		int pipeCtr = 0;
	
		/* Get corresponding tokens and do the right things on those tokens between pipes/redirects */
		for(tokenIdx = 0; tokenIdx <= tokenCnt; tokenIdx++) {
			file = tokens[tokenIdx + 1];
			if(strcmp(tokens[tokenIdx], "<") == 0) {
				commands[commandsIdx] = NULL;
				fdin = redirectIn(file);

				/* If the user attempts a redirect on a file w/o permissions for ex., don't just throw them out of shell by exit. Try again */
				if(fdin < 0) {
					break;
				}
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], ">") == 0) {
				commands[commandsIdx] = NULL;
				fdout = redirectOut(file, 0);

				/* If the user attempts a redirect on a file w/o permissions, don't just throw them out of shell by exit. Try again */
				if(fdout < 0) {
					break;
				}
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], ">>") == 0) {
				commands[commandsIdx] = NULL;
				fdout = redirectOut(file, 1);

				/* If the user attempts a redirect on a file w/o permissions, don't just throw them out of shell by exit. Try again */
				if(fdout < 0) {
					break;
				}
				tokenIdx++;
			}
			else if(strcmp(tokens[tokenIdx], "|") == 0) {
				pipeCtr++;
				commands[commandsIdx] = NULL;

				/* Similar error handle rationale, but due to resource constraints. */
				if (pipe(fd) < 0) {
					perror("pipe");
					break;
				}
			
				/* Attempts to fork process and exec on pipe. Like real shell, fork failure should not boot us w/ exit. Closes fd after, 
 				 * so no process left behind */
				if (execCommand(fdin, fd[1], commands) < 0) {
					break;
				}
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
		
		/* If some redirection failed, don't execCommand. Otherwise execCommand */
		if(fdin >= 0 && fdout >= 0) {
			execCommand(fdin, fdout, commands);
		}
	
		/* Loop for waiting on child processes */
		for(int p = 0; p <= pipeCtr; p++) {
			exitPid = wait(&exitVal);
			if (exitPid < 0) {
				perror("wait");
				break;
			}
		}	
		
		/* Free at the end or if broke out of inner for loop */
		closeAll(fdin, fdout);
		freeAll(tokens, tokenCnt, shellInp, commands);
	}

	/* If we break out of while(1) loop, we go here and indicate failure */
	closeAll(fdin, fdout);
	freeAll(tokens, tokenCnt, shellInp, commands);
	exit(EXIT_FAILURE);
}
