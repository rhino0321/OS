#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <openssl/sha.h>
#include <unistd.h>

#define BUFFER_SIZE 8192
#define HASH_SIZE SHA_DIGEST_LENGTH

struct file_hash
{
    char path[PATH_MAX];
    unsigned char hash[HASH_SIZE];
    struct file_hash *prev;
    struct file_hash *next;
};

struct file_hash *tail;

struct file_hash* init()
{
    struct file_hash *head = malloc(sizeof(struct file_hash));
    head->prev = NULL;
    head->next = NULL;
    tail = head;
    return head;
}

void sha1(char *path)
{
    FILE *file = fopen(path, "rb");
    SHA_CTX sha1;
    SHA1_Init(&sha1);

    unsigned char buf[BUFFER_SIZE];
    size_t len;
    while ((len = fread(buf, 1, sizeof(buf), file)))
    {
        SHA1_Update(&sha1, buf, len);
    }

    SHA1_Final(tail->hash, &sha1);
    strcpy(tail->path, path);

    struct file_hash *node = malloc(sizeof(struct file_hash));
    node->prev = tail;
    tail->next = node;
    node->next = NULL;
    tail = node;

    fclose(file);
}

void construct_file_hash(char *path)
{
    DIR *dir = opendir(path);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        //document d_type -> DT_REG
        //directory d_type -> DT_DIR
        if (entry->d_type == DT_REG)
        {
            char entry_path[PATH_MAX];
            snprintf(entry_path, PATH_MAX, "%s/%s", path, entry->d_name);
            sha1(entry_path);
        }
        else if (entry->d_type == DT_DIR)
        {
            char entry_path[PATH_MAX];
            snprintf(entry_path, PATH_MAX, "%s/%s", path, entry->d_name);
            construct_file_hash(entry_path);
        }
    }
}

void hardlink_unlink(struct file_hash *head)
{
    struct file_hash *cur = head;
    while (cur->next != NULL)
    {
        struct file_hash *compare = cur->next;
        while (compare != NULL)
        {
            if (memcmp(cur->hash, compare->hash, HASH_SIZE) == 0)
            {
                link(cur->path, compare->path);
                unlink(compare->path);

                struct file_hash *tmp = compare;
                compare->prev->next = compare->next;
                compare->next->prev = compare->prev;
                free(tmp);
            }
            compare = compare->next;
        }
        cur = cur->next;
    }
}

void free_node(struct file_hash *head)
{
    struct file_hash *cur = head;
    while (cur != NULL)
    {
        struct file_hash *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
}

int main(int argc, char *argv[])
{
    char *path = argv[1];
    struct file_hash *head = init();
    construct_file_hash(path);
    hardlink_unlink(head);
    free_node(head);
    return 0;
}