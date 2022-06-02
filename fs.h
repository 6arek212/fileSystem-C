#define BLOCK_SIZE 512
#define INODE_NUM 10
#define BLOCKS_NUM 100
#define NAME_LENGTH 8
#define MAX_FILES 1000

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
};

struct disk_block
{
    int next_block_num;
    char data[BLOCK_SIZE + 1];
};

struct myopenfile
{
    int current_block;
    int offset;
    int inode;
};

struct dirent
{
    int inode;
    char name[NAME_LENGTH];
}

void
create_fs();     // initialize new filesystem
void mount_fs(); // load a file system
void sync_fs();  // write the file system

int allocate_file(char *name);
void set_filesize(int fd, int size);
void write_byte(int fd, int pos, char *data);

void print_fs();