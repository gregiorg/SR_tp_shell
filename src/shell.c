/*
 * 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include "csapp.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

void sigchldHandler(int sig)
{
		int status;
    waitpid(-1, &status, WNOHANG|WUNTRACED);
    return;
}

int main()
{
	Signal(SIGCHLD, sigchldHandler);

	while (1) {
		struct cmdline *l;

		printf("shell> ");
		l = readcmd();

		/* Display each command of the pipe */
		// int i, j;
		// for (i = 0; l->seq[i] != 0; i++) {
		// 	char **cmd = l->seq[i];
		// 	printf("seq[%d]: ", i);
		// 	for (j = 0; cmd[j] != 0; j++) {
		// 		printf("%s ", cmd[j]);
		// 	}
		// 	printf("\n");
		// }

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->seq[0] != 0) {

			if (l->err) {
				/* Syntax error, read another command */
				printf("error: %s\n", l->err);
				continue;
			}

			if (strcmp(l->seq[0][0], "quit") == 0) { // Quitting sequence
				printf("Quitting the shell ...\n");
				exit(0);
			} else {

				// Counting the number of commands
				int nbCmd = 0;
				while (l->seq[nbCmd]) {
					nbCmd++;
				}

				int** pipes = Malloc(sizeof(int*)*(nbCmd-1)); // Allocating memory for pipes
				for(int i = 0; i < nbCmd-1; i++) {
					pipes[i] = Malloc(sizeof(int)*2);
				}

				pid_t* pids = Malloc(sizeof(pid_t)*nbCmd);	// Allocating memory for pids
				int inDesc, outDesc;

				// Creating nbCmd -1 pipes
				for (int i = 0; i < nbCmd-1; i++) {
					pipe(pipes[i]);
				}


				for (int i = 0; i < nbCmd; i++) {
					pids[i] = Fork();

					if (!pids[i]) { // child process
						if (i == 0) { // first command

								if (l->in) { // change input if needed
									if ((inDesc = Open(l->in, O_RDONLY, 0)) == -1) { // check if file can be opened
										perror(l->in); // print error if file can't be opened
									}
									Dup2(inDesc, STDIN); // replace standard input
								}

								// compute pipes[0] only if pipes[0] exists (ergo : more than one cmd)
								if (nbCmd > 1) {
									close(pipes[i][0]); // close output
									Dup2(pipes[i][1], STDOUT); // replace standard output
								}
						}

						if (i == nbCmd-1) { // last command

								if (l->out) {// change output if needed
									mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // permisions if file needs to be created
									if ((outDesc = Open(l->out, O_CREAT | O_WRONLY, mode)) == -1) { // check if file can be opened or creates file
										perror(l->in); // print error if file can't be opened
									}
									Dup2(outDesc, STDOUT); // replace standard output
								}

								// compute pipes[nbCmd -1] only if pipes[nbCmd-1] exists (ergo : more than one cmd)
								if (nbCmd > 1) {
									close(pipes[i-1][1]); // close input
									Dup2(pipes[i-1][0], STDIN); // replace standard input
								}
						}

						if (i != 0 && i != nbCmd-1) { // middle commands
							// compute pipes[i-1] : input
							close(pipes[i-1][1]); // close pipe input
							Dup2(pipes[i-1][0], STDIN); // replace standard input by pipe output

							// compute pipes[i] : output
							close(pipes[i][0]); // close pipe output
							Dup2(pipes[i][1], STDOUT); // replace standard output by pipe input
						}

						// exectutes the command[i]
						if (execvp(l->seq[i][0], l->seq[i]) == -1) {
							perror(l->seq[i][0]); // in case of error
							exit(-1);
						}
					}
					if (nbCmd > 1 && i < nbCmd-1) {
						close(pipes[i][1]);  // parent needs to close the pipes input on his side as well
					}
					if (!l->async) { // need to be synchrone
						int status;
						waitpid(pids[i], &status, 0); // parent waits for current child process to finish
					}
				}
			}
		}
	}
}
