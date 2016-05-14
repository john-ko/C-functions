
#define MAXARGS 128
#define	MAXLINE 8192

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

void sigchld_handler(int sig) {
	int status;
	//printf("running sigchld_handler\n");
	while (waitpid((pid_t)(-1), &status, WNOHANG) > 0) {
		
	}

}

void unix_error(char *msg) /* Unix-style error */
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

void eval(char *cmdline)
{
	char *argv[MAXARGS]; /* Argument list execve() */
	char buf[MAXLINE]; /* Holds modified command line */
	int bg; /* Should the job run in bg or fg? */
	pid_t pid; /* Process id */

	strcpy(buf, cmdline);
	bg = parseline(buf, argv);
	if (argv[0] == NULL)
		return; /* Ignore empty lines */

	if (!builtin_command(argv)) {
		if ((pid = fork()) == 0) { /* Child runs user job */
			if (execvp(argv[0], argv) < 0) {
				//printf("%s: Command not found.\n", argv[0]);
				exit(0);
			}
		}

		/* Parent waits for foreground job to terminate */
		if (!bg ) {
			
			int status;
			if (waitpid(pid, &status, 0) == -1) {
				//unix_error("");
			}
		}

	}
	return;
}

 /* parseline - Parse the command line and build the argv array */
 int parseline(char *buf, char **argv)
{
	char *delim; /* Points to first space delimiter */
	int argc; /* Number of args */
	int bg; /* Background job? */

	buf[strlen(buf) - 1] = ' '; /* Replace trailing ’\n’ with space */
	while (*buf && ((*buf == ' ') || (*buf == '\t'))) /* Ignore leading spaces */
		buf++;

	/* Build the argv list */
	argc = 0;
	while ((delim = strchr(buf, ' '))) {
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' ')) /* Ignore spaces */
			buf++;
		 
	}
	argv[argc] = NULL;
	
	if (argc == 0) /* Ignore blank line */
		return 1;

	/* Should the job run in the background? */
	if ((bg = (*argv[argc - 1] == '&')) != 0)
		argv[--argc] = NULL;
	else if ((bg = (argv[argc-1][strlen(argv[argc-1]) - 1] == '&')) != 0) {
		argv[argc-1][strlen(argv[argc-1]) - 1] = '\0';
	}

	return bg;
}

 /* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv)
{
	if (!strcmp(argv[0], "quit")) {
		/////////// QUIT HERE /////////////////

		///KILL THE CHILDREN ////////

		exit(0);
	}
	if (!strcmp(argv[0], "&")) /* Ignore singleton & */
		return 1;
	return 0; /* Not a builtin command */
}

int main()
{
	char cmdline[MAXLINE]; /* Command line */

	if (signal(SIGCHLD, sigchld_handler) == SIG_ERR)
		printf("err");
	while (1) {
		/* Read */
		printf(" prompt > ");
		fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin))
			exit(0);

		/* Evaluate */
		eval(cmdline);

	}
}
