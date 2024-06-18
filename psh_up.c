#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAXARGS 20 
#define ARGLEN  100

int execute(char *arglist[]);
char *makestring(char *buf);

int main()
{
    char *arglist[MAXARGS+1];
    int numargs;
    char argbuf[ARGLEN];

    numargs = 0;
    while (numargs < MAXARGS)
    {
        printf("Arg[%d]? ", numargs);
        if (fgets(argbuf, ARGLEN, stdin) && *argbuf != '\n')
        {
            arglist[numargs++] = makestring(argbuf);
        }
        else
        {
            if (numargs > 0)
            {
                arglist[numargs] = NULL;
                execute(arglist);
                numargs = 0;
            }
        }
    }
    return 0;
}

int execute(char *arglist[])
{
    int pid, exitstatus;
    int pipe_fd[2];
    int input_fd = -1, output_fd = -1;
    int i, j;
    int has_pipe = 0;

    for (i = 0; arglist[i] != NULL; i++)
    {
        if (strcmp(arglist[i], ">") == 0)
        {
            if (arglist[i+1] != NULL)
            {
                output_fd = open(arglist[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
                if (output_fd == -1)
                {
                    perror("open failed");
                    exit(1);
                }
                arglist[i] = NULL;  // truncate the arglist
                break;
            }
        }
        else if (strcmp(arglist[i], "|") == 0)
        {
            has_pipe = 1;
            break;
        }
    }

    if (has_pipe)
    {
        if (pipe(pipe_fd) == -1)
        {
            perror("pipe failed");
            exit(1);
        }
    }

    pid = fork();
    switch (pid)
    {
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            if (output_fd != -1)
            {
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }

            if (has_pipe)
            {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[0]);
                close(pipe_fd[1]);

                arglist[i] = NULL;  // truncate the arglist for the first command
            }

            execvp(arglist[0], arglist);
            perror("execvp failed");
            exit(1);
        default:
            if (has_pipe)
            {
                int pid2 = fork();
                switch (pid2)
                {
                    case -1:
                        perror("fork failed");
                        exit(1);
                    case 0:
                        dup2(pipe_fd[0], STDIN_FILENO);
                        close(pipe_fd[0]);
                        close(pipe_fd[1]);

                        execvp(arglist[i+1], &arglist[i+1]);
                        perror("execvp failed");
                        exit(1);
                    default:
                        close(pipe_fd[0]);
                        close(pipe_fd[1]);
                        waitpid(pid2, &exitstatus, 0);
                }
            }
            while (wait(&exitstatus) != pid);
            printf("child exited with status %d, %d\n",
            exitstatus>>8, exitstatus & 0377);
    }

    if (output_fd != -1)
    {
        close(output_fd);
    }

    return 0;
}

char *makestring(char *buf)
{
    char *cp;

    buf[strlen(buf)-1] = '\0';
    cp = malloc(strlen(buf) + 1);
    if (cp == NULL)
    {
        fprintf(stderr, "no memory\n");
        exit(1);
    }
    strcpy(cp, buf);
    return cp;
}
