#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* We are permitted to assume 4096 as max char num as input */
#define INPUT_LIMIT 4096
#define COMMAND_LIMIT 10 //rlly 11 (cuz its 10 commands + the program to run)
#define TOKEN_DELIM " \t\n\0" 

char* readInp(void) {

	/* Buffer for shoving our command */
	int shellInpSiz = INPUT_LIMIT;
	char* shellInp = malloc(sizeof(char) * shellInpSiz);
	if(shellInp == NULL) {
		return NULL;
	}
	
	/* Where we listen for input indefinitely until either the user does something; no need for infinite loop here? */
	while(1) {
		printf("\n$$ ");

		/* Attempt to get shell input & handles EOF case */
		if(fgets(shellInp, shellInpSiz, stdin) == NULL) {
			exit(0);
		}
		
		/* handle enter for reset case */
		if(strcmp(shellInp, "\n") ==  0) {
			continue;
		}
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
	
	/* handle exit case, weird if i try to do it in readInp function, hmm*/
	if(strcmp(token, "exit") ==  0) {
		//account for free-ing?
		exit(0);
	}

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

void inputredirect(char **tokens, int index, int *fd) {
	*fd = open(tokens[index+1] , O_RDONLY); 
	if(*fd == -1) {
		perror("bash");
		exit(1); //rather, (maybe it's fine) //set signal to break out of out of the infinite while loop;?
	}
	if(dup2(*fd, STDIN_FILENO) == -1) {
		perror("dup2");
	}
	
	//return *fd;
}

void outputredirect(char **tokens, int index, int *fd, int isAppend) {
	if (isAppend == 1) {
		*fd = open(tokens[index+1], O_WRONLY | O_APPEND);
		if(*fd == -1) {
			perror("bash");
			exit(1); //rather, (maybe it's fine) //set signal to break out of out of the infinite while loop;?
		}

	}
	else {
		*fd = open(tokens[index+1], O_WRONLY | O_TRUNC);
		if(*fd == -1) {
			perror("bash");
			exit(0); //rather, (maybe it's fine) //set signal to break out of out of the infinite while loop;?
		}
		
	}

	dup2(*fd, STDOUT_FILENO);
	if(dup2(*fd, STDOUT_FILENO) == -1) {
		perror("dup2");
	}
	//return *fd;

}

/* function for I/O redirection */
void IOredirect(char **tokens, int symIdx, int tknCtr, int isAppend) {
		__pid_t childpid, exitpid;
	int exitval;
	int fd;
	char **nonsymbol = malloc(sizeof(char*) * COMMAND_LIMIT); //remember to free

	childpid = fork();
	if(childpid < 0) {
		perror("fork");
		exit(1);
	}
	/* child does all the work */
	if(childpid == 0) {	
		//subset tokens: eh no func. needed i feel like
		int j = 0;
		for (int i = 0; i <= tknCtr; i++) { //don't start at 0 start at ; //go till first symbol? or start counter where we left off
			while(strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0) {
				/* input redirect */
				if(strcmp(tokens[i], "<") == 0) {
					inputredirect(tokens,i, &fd);
				}

				/*output redirect*/
				if(strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0) {
					outputredirect(tokens, i, &fd, isAppend);
				}

				i+=2;

				if(i > tknCtr) {
					break;
				}
			}

			if(i > tknCtr) {
				break;
			}

			nonsymbol[j] = strdup(tokens[i]); //filling in nonsymbol[i] you buffon
			j++;
		}
		
		//rlly wanna call exec on ALL previous things all things before the symbol (symbol)
		execvp(nonsymbol[0], nonsymbol); 
		/*fork bomb protection*/
		perror("shell");
		exit(1);
		
		
	} else { /* parent waits for child to be done, then cleans up?*/
		exitpid = wait(&exitval);
		//free nonsymbol & close open file descriptors
	}
}


/* piping functionality */
/* intermediate pipes will have to dup twice, we don't want the same dup set up (ouptut for one pipe because input for other pipe (2 closes too))*/
void piping(char ** tokens, int tknCtrPreSym, int offset, int tknCtr) {
	int pipefd[2];
	__pid_t child1pid, child2pid, exitpid;
	int exitval;
	int fd;
	char **presym = malloc(sizeof(char*) * COMMAND_LIMIT); //remember to free
	char **postsym = malloc(sizeof(char*) * COMMAND_LIMIT); //remember to free

	/* parsing, hopefully working */
	// for (int i = 0; i < tknCtrPreSym; i++) {
	// 	presym[i] = tokens[i];
	// }
	// int j = 0;
	// for (int b = offset + 1; b <= tknCtr; b++) {
	// 	postsym[j] = tokens[b];
	// 	j++;
	// }

	presym[0] = strdup("ls");
	postsym[0] = strdup("sort");
	postsym[1] = strdup("-r");
 	/* pipe sys call */
	pipe(pipefd);

	/* THIS IS BROKEN*/
	/* fork */
	child1pid = fork(); //error check
		/* in child, dup, exec */
		if(child1pid == 0){
			write(1, "inhere", 7);
			close(pipefd[0]); 
			dup2(pipefd[1], 1);
			execvp(presym[0], presym);
		} else { /*the parent should wait, and fork*/
			//wait here?
			child2pid = fork();
			if(child2pid == 0) { /*child 2 will dup,exec*/
			//close
				write(1, "inhere", 7);
				close(pipefd[1]); 	
				dup2(pipefd[0], 0);
				execvp(postsym[0], postsym);
			} else { /*the parent still waits x2*/
				//or wait twice here
				wait(NULL);
				wait(NULL);
			}
		}
}


/* basic functionality */
void basicfunc(char **tokens) {
	__pid_t child_pid, exit_pid;
	int exit_value;

	child_pid = fork();
	//error check
	if(child_pid < 0) {
		perror("fork");
		exit(1);
	}

	/* child does all the work */
	if(child_pid == 0) {
		execvp(tokens[0], tokens);
		/*fork bomb protection*/
		perror("shell");
		exit(1);
		
	} else { /* parent waits for child to be done*/
		exit_pid = wait(&exit_value);
	}	
}


int main(int argc, char *argv[]) {
	while(1) {
		char* shellInp = readInp();
		int tokenCnt = 0;
		char** tokens = parseInp(shellInp, &tokenCnt);
		int i;
		int offset;
		int tknCntPreSym = 0;		

		
		printf("%d\n", tokenCnt);
		for(i = 0; i <= tokenCnt; i++) { /*this loop feels useless, just set flags in parsefunc so we know what ops to do!?*/
						
			if (strcmp(tokens[i], "<") == 0) {
				IOredirect(tokens, i, tokenCnt, 0);
				continue;
			}

			if (strcmp(tokens[i], ">") == 0) {
				printf("output redirect\n");
				IOredirect(tokens, i, tokenCnt, 0);
				continue;
			}
			
			if (strcmp(tokens[i], ">>") == 0) {
				printf("appended output redirect\n");
				IOredirect(tokens, i, tokenCnt, 1);
				continue;
			}

			if(strcmp(tokens[i], "|") == 0) {
				printf("piping");
				piping(tokens,tknCntPreSym, i, tokenCnt);
			}
			// 	// for (int a= 0; a < tknCntPreSym; a++) {
			// 	// 	printf("\n%d\n", a);
			// 	// }
			// 	// int j = 0;
			// 	// offset = i;
			// 	// for (int b = offset + 1; b <= tokenCnt; b++) {
			// 	// 	printf("\n%d\n", j);
			// 	// 	printf("\n%d\n", b);
			// 	// 	j++;
			// 	// }
			// 	/* parsing, hopefully working */
			// 	// for (int i = 0; i < tknCntPreSym; i++) {
			// 	// 	printf("\n%d\n",i);
			// 	// }
			// 	// int j = 0;
			// 	// int offset = i;
			// 	// for (int b = offset + 1; b <= tokenCnt; b++) {
			// 	// 	printf("\n%d\n",j);
			// 	// 	printf("\n%d\n",b);
			// 	// 	j++;
			// 	// }

			// 	tknCntPreSym = 0;
			// 	continue;
			// }	

			//tknCntPreSym++;
		}

		// IOredirect(tokens, i, tokenCnt, 0); /* < */
		// IOredirect(tokens, i, tokenCnt, 0); /* > */
		//IOredirect(tokens, i, tokenCnt, 1); /* >> */

		//basicfunc(tokens); /* only run if there's no symbols */

		
	}	
	return 0;
}


