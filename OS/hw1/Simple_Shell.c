/*
Student No.: 311512062
Student Name: Jason Hsu
Email: jason21.hsu@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not 
supposed to be posted to a public server, such as a
public Github repository or a public web page.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include <ctype.h>

pid_t pid;

void handler(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main()
{
    signal(SIGCHLD, handler);
    while (1)
    {
        printf(">");
        bool Wait; // & wait
        bool in = false; // "<""
        bool out = false; // ">"
        bool Pipe = false; // "|"
        char input[100];
        fgets(input, 100, stdin);
        input[strlen(input) - 1] = '\0'; //prevent read \n
        char *tok[20];
        int tok_used = 0;
        char *token = strtok(input, " ");
        while (token != NULL)
        {
            tok[tok_used] = malloc(sizeof(char) * strlen(token));
            strcpy(tok[tok_used], token);
            if (strcmp(tok[tok_used], ">") == 0)
                out = true;
            else if (strcmp(tok[tok_used], "|") == 0)
                Pipe = true;
            tok_used++;
            token = strtok(NULL," ");
        }
        if (strcmp(tok[tok_used - 1], "&") == 0)
        {
            Wait = false;
            tok[tok_used - 1] = NULL;
        }
        else
        {
            Wait = true;
            tok[tok_used] = NULL;
        }
        pid = fork();
        if (pid < 0)
        {
            /* error occurred */
            fprintf(stderr, "Fork Failed");
            exit(-1);
        }
        else if (pid == 0)
        {
            /* child process */
            if (out == true) //redirect std output
            {
                /* cut tok */
                int i = 1;
                while (strtok(tok[i], ">") != 0)
                    i++;
                tok[i] = NULL;
                i++;
                /* tok[0] ....| tok[i] ... NULL*/
                pid_t out_pid;
                out_pid = fork();
                if (out_pid == 0)
                {
                    /* change stdout_fileno to txt */
                    /* out before | result to txt*/
                    int write_fd = open(tok[i], O_CREAT | O_WRONLY | O_TRUNC, 0666);
                    dup2(write_fd, STDOUT_FILENO);
                    execvp(tok[0], tok);
                    exit(0);
                }
                else
                {
                    wait(NULL);
                }
            }
            else if (Pipe == true) // redirect pipelines
            {
                /* cut tok */
                int i = 1;
                while (strtok(tok[i], "|") != 0)
                    i++;
                tok[i] = NULL;
                i++;
                /* tok[0] ....| tok[i] ... NULL*/
                /* create pipe_fd pipe_fd[0] for read pipe_fd[1] for write */
                int pipe_fd[2];
                pipe(pipe_fd);
                pid_t pipe_pid;
                pipe_pid = fork();
                if (pipe_pid == 0)
                {
                    /* put execution before | in pipe_fd[1] dup2 is used to change IN|OUT file descripstor */
                    /* first close fd[0], then use fd[1] */
                    close(pipe_fd[0]);
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    close(pipe_fd[1]);
                    execvp(tok[0], tok);
                    exit(0);
                }
                else
                {
                    /* execution after | */
                    wait(NULL);
                    close(pipe_fd[1]);
                    /*STDIN_FILENO*/
                    dup2(pipe_fd[0], STDIN_FILENO);
                    close(pipe_fd[0]);
                    execvp(tok[i], tok);
                }
            }
            else  //common shell
                execvp(tok[0], tok);
            exit(0);
        }
        else
        {
            /* parent process */
            /* parent will wait for the child to complete */
            if (Wait)
                wait(NULL);
        }
    }
    return 0;
}