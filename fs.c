#include "fs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
// #include <unistd.h>

struct superblock sb;
struct inode *inodes;
struct disk_block *dbs;
const char *fs;
int save_to_disk;

struct myopenfile *opened_files[MAX_FILES];

int get_dir_num(const char *path)
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

const char *get_filename(const char *path)
{
    if (strlen(path) == 1)
    {
        return path;
    }

    const char *p = strlen(path) - 1 + path;
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
    perror("No Empty fd left");
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
    perror("No Empty blocks left");
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
    perror("No Empty inodes left");
    return -1;
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
int mymount(const char *path)
{
    if (inodes)
    {
        free(inodes);
    }

    if (dbs)
    {
        free(dbs);
    }
    fs = path;
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return -1;
    }

    // superblock
    fread(&sb, sizeof(struct superblock), 1, file);

    inodes = (struct inode *)malloc(sizeof(struct inode) * sb.num_inodes);
    dbs = (struct disk_block *)malloc(sizeof(struct disk_block) * sb.num_blocks);

    // inodes
    fread(inodes, sizeof(struct inode), sb.num_inodes, file);

    // blocks
    fread(dbs, sizeof(struct disk_block), sb.num_blocks, file);

    fclose(file);
    return 1;
}

void sync_fs(const char *path)
{
    FILE *file = fopen(path, "w+");

    // superblock
    fwrite(&sb, sizeof(struct superblock), 1, file);

    // inodes
    fwrite(inodes, sizeof(struct inode), sb.num_inodes, file);

    // blocks
    fwrite(dbs, sizeof(struct disk_block), sb.num_blocks, file);

    fclose(file);
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
    // printf("--------------read\n");
    struct myopenfile *p = opened_files[ofd];

    int actual_size = inodes[p->inode].actual_size;

    int max_read_bytes = actual_size - p->offset;

    int c = max_read_bytes > count ? count : max_read_bytes;

    // printofd(ofd);
    // printf("actual size: %d\n", actual_size);
    // printf("count: %d\n", count);
    // printf("max_read_bytes: %d\n", max_read_bytes);
    // printf("bytes to read: %d\n", c);

    int offset_block = p->offset / BLOCK_SIZE;
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

    // printf("--------------readend\n");
    return c;
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
int set_filesize(int inode, int size)
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
            if (empty < 0)
            {
                return -1;
            }
            dbs[b_num].next_block_num = empty;
            dbs[empty].next_block_num = -2;
        }
        b_num = dbs[b_num].next_block_num;
        num--;
    }

    shrink_file(b_num);
    dbs[b_num].next_block_num = -2;
    return 1;
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
    for (size_t i = 0; i < count; i++)
    {
        write_byte(p->inode, p->offset, b);
        p->offset++;
        b++;
    }

    if (save_to_disk)
    {
        sync_fs(fs);
    }

    return count;
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
int get_inode_from_dir(int inode, const char *filename)
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

int get_last_dir_inode_from_path(const char *path)
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

int create_file(int dir_inode, const char *filename)
{

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

int myopen(const char *path, int flags)
{
    // printf("------------------\n");

    int dir_inode = get_last_dir_inode_from_path(path);
    // printf("last dir inode: %d\n", dir_inode);

    if (dir_inode == -1)
    {
        printf("path is not correct\n");
        return -1;
    }
    const char *filename = get_filename(path);
    // printf("filename: %s\n" , filename);
    int file_indoe = get_inode_from_dir(dir_inode, filename);
    // printf("file inode %d\n", file_indoe);

    if (file_indoe < 0)
    {
        if (flags == O_CREAT)
        {
            file_indoe = create_file(dir_inode, filename);
        }
    }

    if (file_indoe < 0)
    {
        return -1;
    }

    // printf("%d------------------\n\n", file_indoe);

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
    printf("\n\n\n");
    printf("Super block info :\n");
    printf("Inodes number: %d\n", sb.num_inodes);
    printf("Blocks number: %d\n", sb.num_blocks);
    printf("Block size: %d\n", sb.size_blocks);

    printf("first 20 inode:\n");
    for (size_t i = 0; i < 20; i++)
    {
        printf("\tsize: %d block: %d  actual size : %d\n", inodes[i].size, inodes[i].first_block, inodes[i].actual_size);
    }

    printf("first 20 blocks:\n");
    for (int i = 0; i < 20; i++)
    {
        printf("\tblock num: %d  next block: %d\n", i, dbs[i].next_block_num);
    }

    printf("\n\n\n");
}

myDIR *myopendir(const char *dirp)
{
    int dir_inode = get_last_dir_inode_from_path(dirp);
    // printf("++++++++dir_inode %d\n", dir_inode);

    const char *dirname = get_filename(dirp);
    // printf("++++++++dirname: %s\n", dirname);

    if (strcmp(dirname, "/") && dir_inode >= 0)
    {
        dir_inode = get_inode_from_dir(dir_inode, dirname);
        // printf("++++++++dir_inode %d\n", dir_inode);
    }

    myDIR *dir = (myDIR *)malloc(sizeof(struct myDIR));
    dir->ofd = get_file_descriptor(dir_inode);
    return dir;
}

struct dirent *myreaddir(myDIR *dir)
{
    if (myread(dir->ofd, (char *)dir + sizeof(int), sizeof(struct dirent)) > 0)
    {
        return &dir->ent;
    }
    return NULL;
}

int myclosedir(myDIR *dir)
{
    if (myclose(dir->ofd) < 0)
    {
        return -1;
    }
    free(dir);
    return 1;
}

void init_fs2()
{
    create_fs();
    create_file(create_file(0, "inner-dir"), "inner-dir2");
    create_file(0, "dir2");
    create_file(0, "dir3");
    create_file(0, "dir4");
    create_file(0, "dir5");
    create_file(0, "dir6");

    int fd = myopen("/inner-dir/inner-dir2/myfile.txt", O_CREAT);
    if (fd < 0)
    {
        perror("Error");
        return;
    }

    char a = 'a';
    for (size_t i = 0; i < 55; i++)
    {
        mywrite(fd, &a, sizeof(char));
    }

    myclose(fd);
    sync_fs("fs_data2");
}

void init_fs1()
{
    create_fs();
    create_file(create_file(0, "inner"), "inner2");
    create_file(0, "dir2");
    create_file(0, "dir3");

    int fd = myopen("/inner/inner2/a3.txt", O_CREAT);
    if (fd < 0)
    {
        perror("Error");
        return;
    }

    char a = 'a';
    for (size_t i = 0; i < 512; i++)
    {
        mywrite(fd, &a, sizeof(char));
    }

    myclose(fd);
    sync_fs("fs_data1");
}

void set_save_to_disk(int mode)
{
    save_to_disk = mode;
}

// int main(int argc, char const *argv[])
// {
//     // create_fs();

//     // create_file(create_file(0, "inner"), "inner2");
//     // create_file(0, "dir2");
//     // create_file(0, "dir3");

//     mount_fs();
//     // set_filesize(0, 1000);
//     char buf[25000];

//     // // print_fs();
//     int fd = myopen("/inner/inner2/a1.txt", O_CREAT);
//     if (fd < 0)
//     {
//         perror("Error");
//         return -1;
//     }

//     printf("\n\n\n\n");

//     myDIR *dir = myopendir("/");
//     printf("dir ofd: %d\n", dir->ofd);

//     struct dirent *d;
//     printf("------------------------------------------------------>\n");
//     while ((d = myreaddir(dir)))
//     {
//         printf("name: %s\n", d->name);
//     }

//     myclosedir(dir);
//     // int fd2 = myopen("/inner/inner2/a1.txt", O_CREAT);
//     mylseek(fd, 0, SEEK_END);
//     char a = 'a';
//     for (size_t i = 0; i < 1024; i++)
//     {
//         mywrite(fd, &a, sizeof(char));
//     }

//     // printf("+++++++++++++++++++++++++++++++++++++\n");
//     // mylseek(fd, 0, SEEK_END);
//     // printofd(fd);
//     // printf("+++++++++++++++++++++++++++++++++++++\n");
//     // for (size_t i = 0; i < 1024; i++)
//     // {
//     //     mywrite(fd, &a, sizeof(char));
//     // }

//     printf("+++++++++++++++++++++++++++++++++++++\n");
//     mylseek(fd, 0, SEEK_SET);
//     printofd(fd);
//     printf("+++++++++++++++++++++++++++++++++++++\n");

//     int x = myread(fd, buf, 25000);
//     buf[x] = '\0';
//     printf("%s %d\n", buf, (int)strlen(buf));

//     myclose(fd);

//     sync_fs();

//     print_fs();

//     return 0;
// }