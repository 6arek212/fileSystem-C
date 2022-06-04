#include "fs.h"
#include "file.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_file_scanf_printf()
{
    myFILE *f = myfopen("/inner/inner2/a1.txt", "w+");

    myfprintf(f, "%d %d", 15, 66);

    int x;
    int y;

    myfseek(f, 0, SEEK_SET);
    myfscanf(f, "%d %d", &x, &y);

    assert(x == 15);
    assert(y == 66);
    myfclose(f);
}

void test_file_fwrite_fread()
{
    myFILE *f = myfopen("/inner/inner2/a1.txt", "w+");
    char *str = "Hello World";
    myfwrite(str, 11, 1, f);

    myfseek(f, 0, SEEK_SET);

    char buff[12];
    myfread(buff, 11, 1, f);

    assert(strcmp(buff, str) == 0);
    myfclose(f);
}

void test_file_myfseek()
{
    myFILE *f = myfopen("/inner/inner2/a1.txt", "w+");
    char *str = "Hello World";
    myfwrite(str, 11, 1, f);

    myfseek(f, -1, SEEK_END);

    char c;
    myfread(&c, 1, 1, f);

    assert(c == 'd');

    myfseek(f, 2, SEEK_SET);
    myfread(&c, 1, 1, f);

    assert(c == 'l');

    myfseek(f, 1, SEEK_CUR);
    myfread(&c, 1, 1, f);
    assert(c == 'o');

    myfclose(f);
}

void test_opening_dir()
{
    myDIR *d = myopendir("/");
    struct dirent *dir;

    char *dirnames1[3] = {"inner", "dir2", "dir3"};
    int i = 0;
    while ((dir = myreaddir(d)))
    {
        assert(strcmp(dir->name, dirnames1[i++]) == 0);
    }

    char *dirnames2[1] = {"inner2"};
    d = myopendir("/inner");
    i = 0;

    while ((dir = myreaddir(d)))
    {
        assert(strcmp(dir->name, dirnames2[i++]) == 0);
    }

    myclosedir(d);
}

void test_myread_mywrite()
{
    int fd = myopen("/inner/hello.txt", O_CREAT);

    if (fd < 0)
    {
        perror("Error");
        printf("test failed\n");
        return;
    }

    const char *str = "hi im tarik";
    mywrite(fd, str, strlen(str));

    mylseek(fd, 0, SEEK_SET);

    char buff[20];

    myread(fd, buff, strlen(str));
    buff[strlen(str)] = '\0';

    assert(strcmp(buff, str) == 0);

    myclose(fd);
}

void test_myopen()
{

    // file not exsits so expect fd = -1

    int fd = myopen("/inner/check.txt", -1);

    assert(fd == -1);

    fd = myopen("/inner/check.txt", O_CREAT);

    assert(fd >= 0);

    myclose(fd);
}

void test_mount()
{
    mymount("fs_data2");

    myDIR *d = myopendir("/");
    struct dirent *dir;

    char *dirnames1[6] = {"inner-dir", "dir2", "dir3", "dir4", "dir5", "dir6"};
    int i = 0;
    while ((dir = myreaddir(d)))
    {
        assert(strcmp(dir->name, dirnames1[i++]) == 0);
    }

    myclosedir(d);
}

int main(int argc, char const *argv[])
{
    mymount("fs_data1");

    test_file_scanf_printf();
    test_file_fwrite_fread();
    test_file_myfseek();

    test_opening_dir();
    test_myopen();
    test_myread_mywrite();

    test_mount();
    print_fs();

    
    printf("test successfull :D\n");
    return 0;
}
