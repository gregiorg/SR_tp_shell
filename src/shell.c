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

int main()
{
	while (1) {
		struct cmdline *l;

		printf("shell> ");
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

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

		if (strcmp(l->seq[0][0], "quit") == 0) { // Quitting sequence
			printf("Quitting the shell ...\n");
			printf("Hope you'll come back soon master ... UwU\n");
			exit(0);
		} else {

			pid_t pid;
			if ((pid = Fork()) == 0) { // child process
				int inDesc, outDesc;

				if (l->in) { // change input if needed
					if ((inDesc = Open(l->in, O_RDONLY, 0)) == -1) { // check if file can be opened
						perror(l->in); // print error if file can't be opened
					}
					Dup2(inDesc, STDIN); // replace standard input
				}

				if (l->out) {// change output if needed
					mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // permisions if file needs to be created
					if ((outDesc = Open(l->out, O_CREAT | O_WRONLY, mode)) == -1) { // check if file can be opened or creates file
						perror(l->in); // print error if file can't be opened
					}
					Dup2(outDesc, STDOUT); // replace standard output
				}

				if (execvp(l->seq[0][0], l->seq[0]) == -1) { // exectutes the command
					perror(l->seq[0][0]); // in case of error
					exit(-1);
				}

			} else { // parent process
				Wait(0); // wait (what did you expect)
			}
		}
	}
}
