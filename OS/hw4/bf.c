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
#include <stddef.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#define pool_size 20000
struct block
{
    size_t size;
    int free;
    struct block *prev;
    struct block *next;
};

struct block* memory_pool = NULL;

char* malloc(size_t alloc_size)
{
    if (alloc_size == 0)
    {
        int max = 0;
        struct block* start = (struct block*)((char*)memory_pool);
        while (start != NULL)
        {
            if (start->free == 1)
            {
                max = (start->size > max) ? start->size : max;
            }
            start = start->next;
        }
        char message[50];
        sprintf(message, "Max Free Chunk Size = %d\n", max);
        write(1, message, strlen(message));
        munmap(memory_pool, pool_size);
        return NULL;
    }
    if (memory_pool == NULL)
    {
        memory_pool = mmap(NULL, 20000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        memory_pool->size = 19968;
        memory_pool->free = 1;
        memory_pool->prev = NULL;
        memory_pool->next = NULL;
    }   
    struct block* start = memory_pool;
    size_t min = INT_MAX;
    struct block* max_block;
    size_t tmp = alloc_size, count = 1;
    while (tmp > 32)
    {
        tmp -= 32;
        count++;
    }
    while (start != NULL)
    {
        if (start->free == 1 && start->size >= 32 * count && start->size < min)
        {
            min = start->size;
            max_block = start;
        }
        start = start->next;
    }
    if (max_block->size - 32 * count < 32)
    {
        max_block->size = 32 * count;
        max_block->free = 0;
    }
    else
    {
        struct block* next_header = (struct block*)((char *)max_block + 32 + 32 * count);
        next_header->size = (max_block->size - 32 * count - 32);
        next_header->free = 1;
        next_header->prev = max_block;
        next_header->next = max_block->next;
        max_block->size = 32 * count;
        max_block->free = 0;
        max_block->next = next_header;
    }
    return (char *)max_block + 32;
}

void free(void *f)
{ 
    struct block *free_loc = (struct block*)((char *)f - 32);
    free_loc->free = 1;
    while (free_loc->prev != NULL && free_loc->prev->free == 1)
    {
        free_loc->prev->size += (32 + free_loc->size);
        free_loc->prev->next = free_loc->next;
        if (free_loc->next != NULL)
            free_loc->next->prev = free_loc->prev;
        free_loc = free_loc->prev;
    }
    while (free_loc->next !=  NULL && free_loc->next->free == 1)
    {
        free_loc->size +=(32 + free_loc->next->size);
        free_loc->next = free_loc->next->next;
        if (free_loc->next->next != NULL)
            free_loc->next->next->prev = free_loc;
    }
}
