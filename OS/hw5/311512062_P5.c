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
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>

struct LFU
{
    int page;
    struct LFU *next;
};

typedef struct
{
    int ref_count;
    int seq;
} LFU_hash;

struct LRU
{
    int page;
    struct LRU *prev;
    struct LRU *next;
};

typedef struct
{
    bool h;
    struct LRU* node;
} LRU_hash;

int main(int argc, char *argv[])
{
    char *fp = argv[1];
    int frame_array[4] = {64, 128, 256, 512};
    int sec, usec;
    printf("LFU policy:\n");
    printf("Frame\tHit\t\tMiss\t\tPage fault ratio\n");
    struct timeval start, end;
    gettimeofday(&start, 0);
    for (int i = 0; i < 4; i++)
    {
        FILE *file = fopen(fp, "r");
        char buffer[100];
        int frame = frame_array[i];
        LFU_hash *hash = malloc(sizeof(LFU_hash) * 2000000);
        for (int j = 0; j < 2000000; j++)
            hash[j].ref_count = 0;
        int frame_used = 0;
        int sequence = 0;
        int hit = 0;
        int miss = 0;
        struct LFU* head = malloc(sizeof(struct LFU*));
        struct LFU* tail = malloc(sizeof(struct LFU*));
        head->next = NULL;
        while (fgets(buffer, sizeof(buffer), file) != NULL)
        {
            sequence++;
            buffer[strlen(buffer) - 1] = '\0';
            int num = atoi(buffer);
            if (hash[num].ref_count > 0)
            {
                hash[num].ref_count++;
                hit++;
                hash[num].seq = sequence;
                continue;
            }
            if (frame_used < frame)
            {
                struct LFU* new_node = malloc(sizeof(struct LFU*));
                new_node->page = num;
                new_node->next = head->next;
                head->next = new_node;
                hash[num].ref_count++;
                hash[num].seq = sequence;
                frame_used++;
                miss++;
            }
            else
            {
                struct LFU* current = head->next;
                struct LFU* target = head->next;
                while (current != NULL)
                {
                    if (hash[current->page].ref_count < hash[target->page].ref_count)
                    {
                        target = current;
                    }
                    else if (hash[current->page].ref_count == hash[target->page].ref_count && hash[current->page].seq < hash[target->page].seq)
                    {
                        target = current;
                    }
                    current = current->next;
                }
                hash[target->page].ref_count = 0;
                target->page = num;
                hash[target->page].ref_count = 1;
                hash[target->page].seq = sequence;
                miss++;
            }
        }
        fclose(file);
        free(hash);
        struct LFU* current = head;
        struct LFU* tmp;
        while (current != NULL)
        {
            tmp = current;
            current = current->next;
            free(tmp);
        }
        printf("%d\t%d\t\t%d\t\t%.10f\n", frame_array[i], hit, miss, ((double) miss) / (hit + miss));
    }
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("Total elapsed time %.4f sec\n\n", sec + usec / 1000000.0);
    // LRU
    printf("LRU policy:\n");
    printf("Frame\tHit\t\tMiss\t\tPage fault ratio\n");
    gettimeofday(&start, 0);
    for (int i = 0; i < 4; i++)
    {
        FILE *file = fopen(fp, "r");
        char buffer[100];
        int frame = frame_array[i];
        int frame_used = 0;
        int hit = 0;
        int miss = 0;
        LRU_hash *hash = malloc(sizeof(LRU_hash) * 2000000);
        for (int j = 0; j < 2000000; j++)
            hash[j].h = false;
        struct LRU* head = malloc(sizeof(struct LRU*));
        struct LRU* tail = malloc(sizeof(struct LRU*));
        head->next = tail;
        head->prev = NULL;
        tail->next = NULL;
        tail->prev = head;
        while (fgets(buffer, sizeof(buffer), file) != NULL)
        {
            buffer[strlen(buffer) - 1] = '\0';
            int num = atoi(buffer);
            if (hash[num].h == true)
            {
                struct LRU* MRU = hash[num].node;
                MRU->prev->next = MRU->next;
                MRU->next->prev = MRU->prev;
                MRU->prev = head;
                MRU->next = head->next;
                head->next->prev = MRU;
                head->next = MRU;
                hit++;
                continue;
            }
            if (frame_used < frame)
            {
                frame_used++;
            }
            else
            {
                struct LRU* remove = tail->prev;
                remove->prev->next = tail;
                tail->prev = remove->prev;
                hash[remove->page].h = false;
                hash[remove->page].node = NULL;
                free(remove);
            }
            struct LRU *new_node = malloc(sizeof(struct LRU*));
            new_node->page = num;
            new_node->prev = head;
            new_node->next = head->next;
            head->next->prev = new_node;
            head->next = new_node;
            hash[num].h = true;
            hash[num].node = new_node;
            miss++;
        }
        fclose(file);
        free(hash);
        struct LRU* current = head;
        struct LRU* tmp;
        while (current != NULL)
        {
            tmp = current;
            current = current->next;
            free(tmp);
        }
        printf("%d\t%d\t\t%d\t\t%.10f\n", frame_array[i], hit, miss, ((double) miss) / (hit + miss));
    }
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("Total elapsed time %.4f sec\n", sec + usec / 1000000.0);
    return 0;
}