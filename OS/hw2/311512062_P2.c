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
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <signal.h>
#include <math.h>

int main()
{
    pid_t pid;
    int shmid, n;
    printf("Input the matrix dimension : ");
    scanf("%d", &n);
    printf("\n");
    /* create and attach share memory */
    key_t key = 1111;
    shmid = shmget(key, sizeof(uint32_t) * n * n * 3, IPC_CREAT | 0666);
    uint32_t (*A)[n] = (uint32_t (*)[n])shmat(shmid, NULL, 0);
    uint32_t (*B)[n] = A + n;
    uint32_t (*C)[n] = B + n;
    /* initialize A B */
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            A[i][j] = n * i + j;
            B[i][j] = n * i + j;
        }
    }
    /* process 1 ~ 16 */
    for (int process = 1; process <= 16; process++)
    {
        (process == 1) ? printf("Multiplying matrices using %d process\n", process) : printf("Multiplying matrices using %d processes\n", process);
        /* record start time */
        struct timeval time_start, time_end;
        gettimeofday(&time_start, 0);
        /* row number of each process */
        int cut = (int)ceil((double) n / process);
        for (int i = 0; i < process; i++)
        {
            pid = fork();
            {
                if (pid < 0)
                {
                    /* error occurred */
                    fprintf(stderr, "Fork Failed");
                    exit(-1);
                }
                else if (pid == 0)
                {
                    /* child process : do A * B = C */
                    int start = cut * i;
                    int end = (cut * i + cut < n) ? cut * i + cut : n;
                    for (int i = start; i < end; i++)
                    {
                        for (int j = 0; j < n; j++)
                        {
                            uint32_t sum = 0;
                            for (int k = 0; k < n; k++)
                            {
                                sum += A[i][k] * B[k][j];
                            }
                            C[i][j] = sum;
                        }
                    }
                    exit(0);
                }
            }
        }
        /* parent wait for all process */
        for (int i = 0; i < process; i++)
        {
            wait(NULL);
        }
        /* calculate check sum */
        uint32_t checksum = 0;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                checksum += C[i][j];
            }
        }
        /* record ebd time */
        gettimeofday(&time_end, 0);
        /* calculate elapsed time */
        int sec = time_end.tv_sec - time_start.tv_sec;
        int usec = time_end.tv_usec - time_start.tv_usec; 
        printf("Elapsed time : %f sec, Checksum : %u\n", sec + usec / 1000000.0, checksum);       
    }
    /* detach ans delete share memory */
    shmdt(A);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}