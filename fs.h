
#include <sys/types.h>

#define BLOCK_SIZE 512
#define INODE_NUM 10
#define BLOCKS_NUM 100
#define NAME_LENGTH 256
#define MAX_FILES 1000
#define O_CREAT 1
#define TYPE_FILE 1;
#define TYPE_DIR 2;

struct superblock
{
    int num_inodes;
    int num_blocks;
    int size_blocks;
};

struct inode
{
    int size;
    int actual_size;
    int first_block;
    int file_type;
};

struct disk_block
{
    int next_block_num;
    char data[BLOCK_SIZE + 1];
};

struct myopenfile
{
    int offset;
    int inode;
};

struct dirent
{
    int inode;
    char name[NAME_LENGTH];
};

typedef struct myDIR
{
    int ofd;
    struct dirent ent;
} myDIR;

void create_fs(); // initialize new filesystem
void sync_fs();   // write the file system
void init_fs1();
void init_fs2();
void print_fs();

int mymount(const char *path);
int myopen(const char *pathname, int flags);
int myclose(int myfd);
size_t myread(int myfd, void *buf, size_t count);
size_t mywrite(int myfd, const void *buf, size_t count);
off_t mylseek(int myfd, off_t offset, int whence);
myDIR *myopendir(const char *name);
struct dirent *myreaddir(myDIR *dirp);
int myclosedir(myDIR *dirp);
