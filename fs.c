#include "fs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
// #include <unistd.h>

#define O_CREAT 1

struct superblock sb;
struct inode *inodes;
struct disk_block *dbs;

struct myopenfile *opened_files[MAX_FILES];

int get_dir_num(char *path)
{
    if (strlen(path) == 0)
    {
        return -1;
    }

    char *p = (char *)malloc(sizeof(char) * strlen(path) + 1);
    strcpy(p, path);
    char *pp = p;
    pp++;

    int cnt = 0;
    while ((pp = strstr(pp, "/")))
    {
        cnt++;
        pp++;
    }

    free(p);
    return cnt;
}

char *get_filename(char *path)
{
    char *p = strlen(path) - 1 + path;
    while (*p != '/')
    {
        p--;
    }
    return ++p;
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

void move_cursur(struct myopenfile *op)
{
    // if ((op->offset + 1) % BLOCK_SIZE == 0 && op->offset + 1 > 0 && dbs[op->current_block].next_block_num != -2)
    // {
    //     op->current_block = dbs[op->current_block].next_block_num;
    // }
    op->offset++;
}

/**
 * @brief Create a fs object
 *
 *
 *
 */
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
    }

    dbs = (struct disk_block *)malloc(sizeof(struct disk_block) * sb.num_blocks);

    for (size_t i = 0; i < sb.num_blocks; i++)
    {
        dbs[i].next_block_num = -1;
        dbs[i].data[BLOCK_SIZE - 1] = '\0';
    }

    allocate_file();
}

/**
 * @brief
 *      mounting the file system from file
 *
 */
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

int allocate_file()
{
    // find an empty inode
    int in = find_empty_inode();
    int block = find_empty_block();

    inodes[in].first_block = block;
    dbs[block].next_block_num = -2;

    inodes[in].size = BLOCK_SIZE;
    inodes[in].actual_size = 0;

    return in;
}

void write_byte(int inode, int pos, char *data)
{
    if (pos >= inodes[inode].actual_size)
    {
        inodes[inode].actual_size++;
    }

    int relative_block = pos / BLOCK_SIZE;

    int b_num = get_block(inode, relative_block);

    int offset = pos % BLOCK_SIZE;

    dbs[b_num].data[offset] = *data;
}

void printofd(int ofd)
{
    struct myopenfile *p = opened_files[ofd];
    printf(" offset: %d  inode %d\n", p->offset, p->inode);
}

size_t myread(int ofd, void *buf, size_t count)
{
    printf("--------------read\n");
    struct myopenfile *p = opened_files[ofd];

    int actual_size = inodes[p->inode].actual_size;

    int max_read_bytes = actual_size - p->offset;

    int c = max_read_bytes > count ? count : max_read_bytes;

    printofd(ofd);
    printf("actual size: %d\n", actual_size);
    printf("count: %d\n", count);
    printf("max_read_bytes: %d\n", max_read_bytes);
    printf("bytes to read: %d\n", c);

    int offset_block = (p->offset + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int current_block = get_block(p->inode, offset_block);

    char *b = (char *)buf;
    for (size_t i = 0; i < c; i++)
    {
        // printofd(ofd);
        *b = dbs[current_block].data[p->offset % BLOCK_SIZE];
        p->offset++;
        if (p->offset % BLOCK_SIZE == 0)
        {
            current_block = dbs[current_block].next_block_num;
        }
        b++;
    }

    printf("--------------readend\n");
    return c;
}

size_t mywrite(int ofd, const void *buf, size_t count)
{
    // printofd(ofd);
    struct myopenfile *p = opened_files[ofd];
    int inode = p->inode;
    int size = inodes[p->inode].size;

    if (p->offset + count - 1 >= size)
    {
        set_filesize(inode, p->offset + count);
    }

    char *b = (char *)buf;
    int pos;
    for (size_t i = 0; i < count; i++)
    {
        write_byte(p->inode, p->offset, b);
        p->offset++;
        b++;
    }

    return count;
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
void set_filesize(int inode, int size)
{
    int num = size / BLOCK_SIZE;
    if (size % BLOCK_SIZE > 0)
    {
        num++;
    }
    inodes[inode].size = num * BLOCK_SIZE;

    int b_num = inodes[inode].first_block;
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

int get_file_descriptor(int inode)
{
    int ofd = get_free_opened_file_index();
    opened_files[ofd] = (struct myopenfile *)malloc(sizeof(struct myopenfile));
    opened_files[ofd]->offset = 0;
    opened_files[ofd]->inode = inode;
    return ofd;
}

int myclose(int ofd)
{
    if (ofd > MAX_FILES || ofd < 0 || !opened_files[ofd])
    {
        return -1;
    }
    free(opened_files[ofd]);
    opened_files[ofd] = NULL;
    return 1;
}

/**
 * @brief Get the inode from dir object
 *
 * @param inode for the dir
 * @param filename a null terminated string
 * @return int
 */
int get_inode_from_dir(int inode, char *filename)
{
    int ofd = get_file_descriptor(inode);
    int b;
    char buf[sizeof(struct dirent) + 1];

    while ((b = myread(ofd, buf, sizeof(struct dirent))) > 0)
    {
        buf[b] = '\0';
        // printf("++++dirname: %s\n", ((struct dirent *)buf)->name);
        if (!strcmp(filename, ((struct dirent *)buf)->name))
        {
            myclose(ofd);
            return ((struct dirent *)buf)->inode;
        }
    }

    myclose(ofd);
    return -1;
}

int get_last_dir_inode_from_path(char *path)
{
    if (strlen(path) == 0)
    {
        return -1;
    }
    if (path[0] != '/')
    {
        return -1;
    }

    char *pathcpy = (char *)malloc(sizeof(char) * strlen(path));
    strcpy(pathcpy, path);
    int dirnum = get_dir_num(path);

    char *p = strtok(pathcpy, "/");
    int i = 0;
    int cnt = 0;
    while (cnt < dirnum)
    {
        i = get_inode_from_dir(i, p);
        if (i < 0)
        {
            return -1;
        }
        cnt++;
        p = strtok(NULL, "/");
    }
    free(pathcpy);
    return i;
}


int myopen(char *path, int flags)
{
    printf("------------------\n");

    int dir_inode = get_last_dir_inode_from_path(path);
    printf("last dir inode: %d\n", dir_inode);

    if (dir_inode == -1)
    {
        printf("path is not correct\n");
        return -1;
    }
    char *filename = get_filename(path);
    int file_indoe = get_inode_from_dir(dir_inode, filename);
    printf("file inode %d\n", file_indoe);

    if (file_indoe < 0)
    {
        if (flags == O_CREAT)
        {
            file_indoe = make_file(dir_inode, filename);
        }
        else
        {
            return -1;
        }
    }

    printf("------------------\n\n");

    return get_file_descriptor(file_indoe);
}

off_t mylseek(int ofd, off_t offset, int whence)
{
    struct myopenfile *p = opened_files[ofd];

    int acutal_size = inodes[p->inode].actual_size;
    int size = inodes[p->inode].size;

    if (whence == SEEK_SET) // relative to begin of file
    {
        if (offset > size || offset < 0)
        {
            return -1;
        }
        p->offset = offset;
    }
    else if (whence == SEEK_CUR) // relative to current position
    {
        if (offset + p->offset > size || offset + p->offset < 0)
        {
            return -1;
        }
        p->offset = p->offset + offset;
    }
    else if (whence == SEEK_END) // relative to end of file
    {
        if (offset + acutal_size > size || offset + acutal_size < 0)
        {
            return -1;
        }
        p->offset = acutal_size + offset;
    }

    return offset;
}

void print_fs()
{
    printf("Super block info :\n");
    printf("Inodes number: %d\n", sb.num_inodes);
    printf("Blocks number: %d\n", sb.num_blocks);
    printf("Block size: %d\n", sb.size_blocks);

    for (size_t i = 0; i < sb.num_inodes; i++)
    {
        printf("\tsize: %d block: %d  actual size : %d\n", inodes[i].size, inodes[i].first_block, inodes[i].actual_size);
    }

    for (int i = 0; i < sb.num_blocks; i++)
    {
        printf("\tblock num: %d  next block: %d  len: %d\n", i, dbs[i].next_block_num, (int)strlen(dbs[i].data));
    }
}

int make_file(int dir_inode, char *filename)
{
    // TODO: check if there is a file same with same name !

    // create the file
    int ofd = get_file_descriptor(dir_inode);
    mylseek(ofd, 0, SEEK_END);

    struct dirent d;
    d.inode = allocate_file();
    strcpy(d.name, filename);

    char *p = (char *)&d;

    mywrite(ofd, p, sizeof(struct dirent));

    myclose(ofd);

    return d.inode;
}

int main(int argc, char const *argv[])
{
    create_fs();

    make_file(make_file(0, "inner"), "inner2");
    make_file(0, "dir2");
    make_file(0, "dir3");

    // mount_fs();
    // set_filesize(0, 1000);
    char buf[4000];

    // print_fs();
    int fd = myopen("/inner/inner2/a1.txt", O_CREAT);
    if (fd < 0)
    {
        perror("Error");
        return -1;
    }

    int fd2 = myopen("/inner/inner2/a1.txt", O_CREAT);

    char a = 'a';
    for (size_t i = 0; i < 1024; i++)
    {
        mywrite(fd, &a, sizeof(char));
    }

    printf("+++++++++++++++++++++++++++++++++++++\n");
    mylseek(fd, 0, SEEK_END);
    printofd(fd);
    printf("+++++++++++++++++++++++++++++++++++++\n");
    for (size_t i = 0; i < 1024; i++)
    {
        mywrite(fd, &a, sizeof(char));
    }

    printf("+++++++++++++++++++++++++++++++++++++\n");
    mylseek(fd, 0, SEEK_SET);
    printofd(fd);
    printf("+++++++++++++++++++++++++++++++++++++\n");

    int x = myread(fd, buf, 25000);
    buf[x] = '\0';
    printf("%s %d\n", buf, strlen(buf));
    print_fs();

    // myclose(fd);

    return 0;
}