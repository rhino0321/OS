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
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#define NUM_THREADS 8

typedef enum
{
    undo,
    wait,
    finish
} Status;

typedef struct
{
    int Size;          // job array size
    int *arr;  // job array
    char job_type[10]; // Bubble or Merge
    Status status;     // Undo or Wait or Finish
} JOB;

JOB job_list[16];
int job_queue[16], queue_size = 0;
sem_t job, complete, change_job_queue, change_job_status;

void bubble_sort(int id)
{
    for (int i = 0; i < job_list[id].Size - 1; i++)
    {
        for (int j = 0; j < job_list[id].Size - i - 1; j++)
        {
            if (job_list[id].arr[j] > job_list[id].arr[j + 1])
            {
                int tmp = job_list[id].arr[j];
                job_list[id].arr[j] = job_list[id].arr[j + 1];
                job_list[id].arr[j + 1] = tmp;  
            } 
        }
    }
}

void merge_sort(int id)
{
    job_list[id].Size = job_list[id * 2].Size + job_list[id * 2 + 1].Size;
    job_list[id].arr = malloc(sizeof(int) * (job_list[id].Size));
    int left = 0, right = 0;
    for (int i = 0; i < job_list[id].Size; i++)
    {
        if (left >= job_list[id * 2].Size)
            job_list[id].arr[i] = job_list[id * 2 + 1].arr[right++];
        else if (right >= job_list[id * 2 + 1].Size)
            job_list[id].arr[i] = job_list[id * 2].arr[left++];
        else if (job_list[id * 2].arr[left] <= job_list[id * 2 + 1].arr[right])
            job_list[id].arr[i] = job_list[id * 2].arr[left++];
        else if (job_list[id * 2].arr[left] > job_list[id * 2 + 1].arr[right])
            job_list[id].arr[i] = job_list[id * 2 + 1].arr[right++];
    }
}

void *thread_func()
{
    while (1)
    {
        sem_wait(&job);
        sem_wait(&change_job_queue);
        int id = job_queue[0];
        queue_size--;
        for (int i = 0; i < queue_size; i++)
            job_queue[i] = job_queue[i + 1];
        sem_post(&change_job_queue);
        strcmp(job_list[id].job_type, "Bubble") == 0 ? bubble_sort(id) : merge_sort(id);
        sem_wait(&change_job_status);
        job_list[id].status = finish;
        sem_post(&change_job_status);
        sem_post(&complete);
    }
}

void check_insert(int id)
{
    if (job_list[id].status != undo)
        return;
    if (job_list[id * 2].status == finish && job_list[id * 2 + 1].status == finish)
    {
        job_list[id].status = wait;
        strcpy(job_list[id].job_type, "Merge");
        sem_wait(&change_job_queue);
        job_queue[queue_size++] = id;
        sem_post(&change_job_queue);
        sem_post(&job);
    }
    check_insert(id * 2);
    check_insert(id * 2 + 1);
}

int main()
{
    FILE *create_input_file = fopen("input.txt", "w");
    int inputSize = 100000;
    fprintf(create_input_file, "%d\n", inputSize);
    for (int i = 0; i < inputSize; i++)
        fprintf(create_input_file, "%d ", inputSize - i);
    fclose(create_input_file);
    //int inputSize;
    FILE *input_file = fopen("input.txt", "r");
    fscanf(input_file, "%d", &inputSize);
    int data[inputSize]; 
    for (int i = 0; i < inputSize; i++)
    {
        fscanf(input_file, "%d", &data[i]);
    }
    fclose(input_file);
    for (int num = 1; num <= NUM_THREADS; num++)
    {
        for (int id = 1; id < 16; id++)
        {
            if (id >= 8)
            {
                job_list[id].status = wait;
                job_queue[queue_size++] = id;
            }
            else
                job_list[id].status = undo;
        }
        int divide = (int) floor((double) inputSize / NUM_THREADS);
        int mod = inputSize % NUM_THREADS;
        int index = 0;
        for (int i = 8; i < 8 + NUM_THREADS; i++)
        {
            job_list[i].Size = (mod > 0) ? divide + 1 : divide;
            mod--;
            job_list[i].arr = malloc(sizeof(int) * (job_list[i].Size));
            for (int j = index; j < index + job_list[i].Size; j++)
                job_list[i].arr[j - index] = data[j];
            strcpy(job_list[i].job_type, "Bubble");
            index = index + job_list[i].Size;
        }
        sem_init(&job, 0, 8);
        sem_init(&complete, 0, 0);
        sem_init(&change_job_queue, 0, 1);
        sem_init(&change_job_status, 0, 1);
        pthread_t threads[num];
        for (int i = 0; i < num; i++)
        {
            pthread_create(&threads[i], NULL, thread_func, NULL);
        }
        struct timeval time_start, time_end;
        gettimeofday(&time_start, 0);
        while (job_list[1].status != finish)
        {
            sem_wait(&complete);
            sem_wait(&change_job_status);
            check_insert(1);
            sem_post(&change_job_status);
        }
        char file_name[20];
        sprintf(file_name, "output_%d.txt", num);
        FILE *output_file = fopen(file_name, "w");
        for (int i = 0; i < job_list[1].Size; i++)
        {
            fprintf(output_file, "%d ", job_list[1].arr[i]);
        }
        fclose(output_file);
        sem_destroy(&job);
        sem_destroy(&complete);
        sem_destroy(&change_job_queue);
        sem_destroy(&change_job_status);

        gettimeofday(&time_end, 0);
        int sec = time_end.tv_sec - time_start.tv_sec;
        int usec = time_end.tv_usec - time_start.tv_usec; 
        printf("worker thread #%d, elapsed %f ms\n", num, sec * 1000 + usec / 1000.0);
    }
    return 0;
}
