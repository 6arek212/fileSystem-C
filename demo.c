#include "fs.h"
#include "file.h"
#include <stdio.h>
#include <string.h>

void print_menu()
{
    printf("-------------------------\n\n");
    printf("1- make file system\n");
    printf("2- mount file system\n");
    printf("3- open file\n");
    printf("4- write to file\n");
    printf("5- read from file\n");
    printf("6- save file system\n");
    printf("7- print directories\n");
    printf("8- exit\n");
    printf("-------------------------\n\n");
}

int main(int argc, char const *argv[])
{

    int flag = 1;
    int fd = -1;
    int choice;

    while (flag)
    {
        print_menu();
        scanf("%d", &choice);
        char text[NAME_LENGTH];
        int len;
        int b;
        int size;
        myDIR *dir;
        struct dirent *d;

        switch (choice)
        {
        case 1:
            printf("Enter the size of the file system\n");
            scanf("%d", &size);
            mymkfs(size);
            printf("File system created\n");
            break;

        case 2:
            printf("Enter file system path\n");
            scanf("%s", text);
            text[strlen(text)] = '\0';
            mymount(text);
            break;

        case 3:
            printf("Enter file path\n");
            scanf("%s", text);
            text[strlen(text)] = '\0';
            fd = myopen(text, O_CREAT);
            break;

        case 4:
            if (fd < 0)
            {
                printf("file was not opened\n");
            }
            mylseek(fd, 0, SEEK_END);
            printf("Enter your text...\n");
            scanf("%s", text);
            mywrite(fd, text, strlen(text));
            break;
        case 5:
            if (fd < 0)
            {
                printf("file was not opened\n");
            }
            mylseek(fd, 0, SEEK_SET);
            printf("Enter length to read...\n");
            scanf("%d", &len);
            b = myread(fd, text, len);
            text[b] = '\0';
            printf("read text: %s\n", text);
            break;

        case 6:
            printf("Enter fs name to save\n");
            scanf("%s", text);
            sync_fs(text);
            break;

        case 7:
            printf("Enter dir path\n");
            scanf("%s", text);
            dir = myopendir(text);
            while ((d = myreaddir(dir)))
            {
                printf("%s\n", d->name);
            }
            break;

        case 8:
            flag = 0;
            break;
        }
    }

    return 0;
}
