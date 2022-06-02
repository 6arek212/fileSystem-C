#include <sys/types.h>

#define MAX_FILES 10000

struct myDIR
{
};

struct mydirent
{
    char *d_name;
};

struct myopenfile
{
};

/**
 * @brief
 *     mount() attaches the filesystem specified by source (which is
       often a pathname referring to a device, but can also be the
       pathname of a directory or file, or a dummy string) to the
       location (a directory or file) specified by the pathname in
       target.
 *
 */
// Remount an existing mount: mountflags includes MS_REMOUNT.
//Create a new mount: mountflags includes none of the above flags
int mymount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data);
int myopen(const char *pathname, int flags);
int myclose(int myfd);
ssize_t myread(int myfd, void *buf, size_t count);
ssize_t mywrite(int myfd, const void *buf, size_t count);
off_t mylseek(int myfd, off_t offset, int whence);
myDIR *myopendir(const char *name);
struct mydirent *myreaddir(myDIR *dirp);
int myclosedir(myDIR *dirp);