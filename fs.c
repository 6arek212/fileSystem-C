#include "fs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
// #include <unistd.h>

struct superblock sb;
struct inode *inodes;
struct disk_block *dbs;

struct myopenfile *opened_files[MAX_FILES];

int allocate_file(char *name)
{
    // find an empty inode
    int in = find_empty_inode();
    int block = find_empty_block();

    inodes[in].first_block = block;
    dbs[block].next_block_num = -2;

    strcpy(inodes[in].name, name);
    inodes[in].size = BLOCK_SIZE;
    inodes[in].actual_size = 0;

    return in;
}

void create_fs()
{
    sb.num_inodes = INODE_NUM;
    sb.num_blocks = BLOCKS_NUM;
    sb.size_blocks = sizeof(struct disk_block);

    inodes = (struct inode *)malloc(sizeof(struct inode) * sb.num_inodes);

    for (size_t i = 0; i < sb.num_inodes; i++)
    {
        inodes[i].first_block = -1;
        inodes[i].size = 0;
        inodes[i].actual_size = 0;
        // strcpy(inodes[i].name, "emptyfi");
    }

    dbs = (struct disk_block *)malloc(sizeof(struct disk_block) * sb.num_blocks);

    for (size_t i = 0; i < sb.num_blocks; i++)
    {
        dbs[i].next_block_num = -1;
        dbs[i].data[BLOCK_SIZE - 1] = '\0';
    }

    allocate_file('/');
}

void mount_fs()
{
    FILE *file = fopen("fs_data", "r");
    // superblock
    fread(&sb, sizeof(struct superblock), 1, file);

    inodes = (struct inode *)malloc(sizeof(struct inode) * sb.num_inodes);
    dbs = (struct disk_block *)malloc(sizeof(struct disk_block) * sb.num_blocks);

    // inodes
    fread(inodes, sizeof(struct inode), sb.num_inodes, file);

    // blocks
    fread(dbs, sizeof(struct disk_block), sb.num_blocks, file);

    fclose(file);
}

void sync_fs()
{
    FILE *file = fopen("fs_data", "w+");

    // superblock
    fwrite(&sb, sizeof(struct superblock), 1, file);

    // inodes
    fwrite(inodes, sizeof(struct inode), sb.num_inodes, file);

    // blocks
    fwrite(dbs, sizeof(struct disk_block), sb.num_blocks, file);

    fclose(file);
}

int find_empty_block()
{
    for (size_t i = 0; i < sb.num_blocks; i++)
    {
        if (dbs[i].next_block_num == -1)
        {
            return i;
        }
    }
    return -1;
}

int find_empty_inode()
{
    for (size_t i = 0; i < sb.num_inodes; i++)
    {
        if (inodes[i].first_block == -1)
        {
            return i;
        }
    }
    return -1;
}

void print_fs()
{
    printf("Super block info :\n");
    printf("Inodes number: %d\n", sb.num_inodes);
    printf("Blocks number: %d\n", sb.num_blocks);
    printf("Block size: %d\n", sb.size_blocks);

    for (size_t i = 0; i < sb.num_inodes; i++)
    {
        printf("\tsize: %d block: %d name: %s actual size : %d\n", inodes[i].size, inodes[i].first_block, inodes[i].name, inodes[i].actual_size);
    }

    for (int i = 0; i < sb.num_blocks; i++)
    {
        printf("\tblock num: %d  next block: %d  len: %d\n", i, dbs[i].next_block_num, strlen(dbs[i].data));
    }
}

void shrink_file(int b_num)
{
    int n = dbs[b_num].next_block_num;
    if (n >= 0)
    {
        shrink_file(n);
    }

    dbs[b_num].next_block_num = -1;
}

// add / delete blocks
void set_filesize(int fd, int size)
{
    int num = size / BLOCK_SIZE;
    if (size % BLOCK_SIZE > 0)
    {
        num++;
    }
    inodes[fd].size = num * BLOCK_SIZE;

    int b_num = inodes[fd].first_block;
    num--;
    while (num > 0)
    {
        int next_num = dbs[b_num].next_block_num;

        if (next_num == -2)
        {
            int empty = find_empty_block();
            dbs[b_num].next_block_num = empty;
            dbs[empty].next_block_num = -2;
        }
        b_num = dbs[b_num].next_block_num;
        num--;
    }

    shrink_file(b_num);
    dbs[b_num].next_block_num = -2;
}

int get_block(int b_num, int offset)
{
    int togo = offset;
    int bn = inodes[b_num].first_block;

    while (togo > 0)
    {
        bn = dbs[bn].next_block_num;
        togo--;
    }
    return bn;
}

void write_byte(int fd, int pos, char *data)
{
    if (pos >= inodes[fd].size)
    {
        set_filesize(fd, inodes[fd].size + 1);
    }

    int relative_block = pos / BLOCK_SIZE;

    int b_num = get_block(fd, relative_block);

    int offset = pos % BLOCK_SIZE;

    dbs[b_num].data[offset] = *data;

    inodes[fd].actual_size++;
}

int get_free_opened_file_index()
{
    for (size_t i = 0; i < MAX_FILES; i++)
    {
        if (!opened_files[i])
        {
            return i;
        }
    }
    return -1;
}

struct inode *get_inode_from_path(char *path)
{
    if (strlen(path) == 0)
    {
        return -1;
    }

    if (path[0] != '/')
    {
        return -1;
    }

    path++;

    struct inode *in = &inodes[0];
    char *p;
    char buf[sizeof(struct inode)];

    while (*path)
    {
        p = strstr(path, "/");
        *p = '\0';

        while (myread(i, buf, sizeof(struct inode)) > 0)
        {
            if ()
            {
                /* code */
            }
        }
    }
}

int myopen(char *path, int flags)
{

    int index = get_free_opened_file_index();

    opened_files[index] = (struct myopenfile *)malloc(sizeof(struct myopenfile));
    opened_files[index]->offset = 0;
    opened_files[index]->inode = 0;
    opened_files[index]->current_block = inodes[0].first_block;
    return index;
}

int myclose(int ofd)
{
    if (ofd > MAX_FILES || ofd < 0 || !opened_files[ofd])
    {
        return -1;
    }
    free(opened_files[ofd]);
    opened_files[ofd] = NULL;
    return 0;
}

void move_cursur(struct myopenfile *op)
{
    if (op->offset + 1 == BLOCK_SIZE)
    {
        op->current_block = dbs[op->current_block].next_block_num;
    }
    op->offset = (op->offset + 1) % BLOCK_SIZE;
}

size_t myread(int ofd, void *buf, size_t count)
{
    struct myopenfile *p = opened_files[ofd];
    int actual_size = inodes[p->inode].actual_size;

    int max_read_bytes = actual_size - (BLOCK_SIZE * p->current_block + p->offset);

    printf("%d %d\n", p->current_block, p->offset);
    int c = max_read_bytes > count ? count : max_read_bytes;

    char *b = (char *)buf;

    for (size_t i = 0; i < c; i++)
    {
        *b = dbs[p->current_block].data[p->offset];
        move_cursur(p);
        b++;
    }

    return c;
}

size_t mywrite(int ofd, const void *buf, size_t count)
{
    struct myopenfile *p = opened_files[ofd];
    int size = inodes[p->inode].size - inodes[p->inode].actual_size;

    if (count > size)
    {
        int num_of_blockes_needed = count / BLOCK_SIZE;
        if (count % BLOCK_SIZE > 0)
        {
            num_of_blockes_needed++;
        }
        set_filesize(p->inode, inodes[p->inode].size + num_of_blockes_needed * BLOCK_SIZE);
    }

    char *b = (char *)buf;
    for (size_t i = 0; i < count; i++)
    {
        write_byte(p->inode, p->current_block * BLOCK_SIZE + p->offset, b);
        move_cursur(p);
        b++;
    }

    return count;
}

off_t mylseek(int ofd, off_t offset, int whence)
{
    struct myopenfile *p = opened_files[ofd];

    if (whence == SEEK_SET)
    {
        if (offset >= inodes[p->inode].size)
        {
            p->current_block = inodes[p->inode].size / BLOCK_SIZE;
            p->offset = BLOCK_SIZE - 1;
        }

        p->current_block = offset / BLOCK_SIZE;
        p->offset = offset % BLOCK_SIZE;
    }
    else if (whence == SEEK_CUR)
    {
        if (offset + p->current_block * BLOCK_SIZE + p->offset >= inodes[p->inode].size)
        {
            p->current_block = inodes[p->inode].size / BLOCK_SIZE;
            p->offset = BLOCK_SIZE - 1;
        }
        printf("------%d\n", offset);

        int b = offset / BLOCK_SIZE;
        p->current_block = b;
        p->offset = offset % BLOCK_SIZE;
    }
    else if (whence == SEEK_END)
    {
        p->current_block = inodes[p->inode].size / BLOCK_SIZE;
        p->offset = BLOCK_SIZE;
    }
    return offset;
}

int main(int argc, char const *argv[])
{
    create_fs();
    // mount_fs();
    // set_filesize(0, 1000);
    char ch;
    char buf[4000] = "hello world !";
    int fd = myopen();

    mywrite(fd, buf, strlen(buf));
    // sync_fs();

    mylseek(fd, 10, SEEK_CUR);

    printf("%s\n", buf);

    int x = myread(fd, buf, 25000);
    buf[x] = '\0';

    printf("%s\n", buf);

    print_fs();

    myclose(fd);

    return 0;
}