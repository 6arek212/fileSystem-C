#include "fs.h"
#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

char *_strrev(char *str)
{
    int i;
    int len = 0;
    char c;
    if (!str)
        return NULL;
    while (str[len] != '\0')
    {
        len++;
    }
    for (i = 0; i < (len / 2); i++)
    {
        c = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = c;
    }
    return str;
}

char *_itoa(int i, char *strout, int base)
{
    char *str = strout;
    int digit, sign = 0;
    if (i < 0)
    {
        sign = 1;
        i *= -1;
    }
    while (i)
    {
        digit = i % base;
        *str = (digit > 9) ? ('A' + digit - 10) : '0' + digit;
        i = i / base;
        str++;
    }
    if (sign)
    {
        *str++ = '-';
    }
    *str = '\0';
    _strrev(strout);
    return strout;
}

myFILE *myfopen(const char *restrict pathname, const char *restrict mode)
{
    myFILE *f;
    int fd;

    if (!strcmp(mode, "r"))
    {
        fd = myopen(pathname, -1);
        if (fd == -1)
        {
            return NULL;
        }
        f = (myFILE *)malloc(sizeof(myFILE));
        f->fd = fd;
        f->mode = READ;
        return;
    }

    if (!strcmp(mode, "r+"))
    {
        fd = myopen(pathname, -1);
        if (fd == -1)
        {
            return NULL;
        }
        f = (myFILE *)malloc(sizeof(myFILE));
        f->fd = fd;
        f->mode = READ_WRITE;
        return;
    }

    fd = myopen(pathname, O_CREAT);
    if (fd == -1)
    {
        return NULL;
    }
    f = (myFILE *)malloc(sizeof(myFILE));
    f->fd = fd;

    if (!strcmp(mode, "w+"))
    {
        f->mode = READ_WRITE;
        return;
    }

    if (!strcmp(mode, "w"))
    {
        f->mode = WRITE;
        return;
    }

    if (!strcmp(mode, "a"))
    {
        mylseek(f->fd, 0, SEEK_END);
        f->mode = WRITE;
        return;
    }

    if (!strcmp(mode, "a+"))
    {
        mylseek(f->fd, 0, SEEK_END);
        f->mode = READ_WRITE;
        return;
    }
}

int myfclose(myFILE *stream)
{
    myclose(stream->fd);
    free(stream);
}

size_t myfread(void *restrict ptr, size_t size, size_t nmemb, myFILE *restrict stream)
{
    if (stream->mode != READ && stream->mode != READ_WRITE)
    {
        printf("mode is not compatipabl\n");
        return 0;
    }
    return myread(stream->fd, ptr, size * nmemb);
}

size_t myfwrite(const void *restrict ptr, size_t size, size_t nmemb, myFILE *restrict stream)
{
    if (stream->mode != WRITE && stream->mode != READ_WRITE)
    {
        printf("mode is not compatipabl\n");
        return 0;
    }
    return mywrite(stream->fd, ptr, size * nmemb);
}

int myfseek(myFILE *stream, long offset, int whence)
{
    return mylseek(stream->fd, offset, whence);
}

int count_int_vars(const char *str)
{

    int cnt = 0;

    char *strcpy = (char *)malloc(sizeof(char) * strlen(str));
    char *p = strtok(strcpy, "%d");
}

int myfscanf(myFILE *restrict stream, const char *restrict format, ...)
{
    va_list vl;
    int i = 0, j = 0, ret = 0;
    char buff[100] = {0}, tmp[20], c = '\0';
    char *out_loc;
    while (c != '\n')
    {
        if (myfread(&c, 1, 1, stream))
        {
            buff[i] = c;
            i++;
        }
        else
        {
            break;
        }
    }
    va_start(vl, format);
    i = 0;
    while (format && format[i])
    {
        if (format[i] == '%')
        {
            i++;
            switch (format[i])
            {
            case 'c':
            {
                *(char *)va_arg(vl, char *) = buff[j];
                j++;
                ret++;
                break;
            }
            case 'd':
            {
                *(int *)va_arg(vl, int *) =
                    strtol(&buff[j], &out_loc, 10);
                j += out_loc - &buff[j];
                ret++;
                break;
            }
            case 's':
            {
                out_loc = (char *)va_arg(vl, char *);
                strcpy(out_loc, &buff[j]);
                j += strlen(&buff[j]);
                ret++;
                break;
            }
            }
        }
        else
        {
            buff[j] = format[i];
            j++;
        }
        i++;
    }
    printf("//%s\n", buff);
    va_end(vl);
    myfseek(stream, j, SEEK_SET);
    return ret;
}

int myfprintf(myFILE *restrict stream, const char *restrict format, ...)
{

    va_list vl;
    int i = 0, j = 0;
    char buff[100] = {0}, tmp[20];
    char *str_arg;

    va_start(vl, format);
    while (format && format[i])
    {
        if (format[i] == '%')
        {
            i++;
            switch (format[i])
            {
            /* Convert char */
            case 'c':
            {
                buff[j] = (char)va_arg(vl, int);
                j++;
                break;
            }
            /* Convert decimal */
            case 'd':
            {
                _itoa(va_arg(vl, int), tmp, 10);
                strcpy(&buff[j], tmp);
                j += strlen(tmp);
                break;
            }

            /* copy string */
            case 's':
            {
                str_arg = va_arg(vl, char *);
                strcpy(&buff[j], str_arg);
                j += strlen(str_arg);
                break;
            }
            }
        }
        else
        {
            buff[j] = format[i];
            j++;
        }
        i++;
    }
    myfwrite(buff, j, 1, stream);
    va_end(vl);
    return j;
}

int main(int argc, char const *argv[])
{
    init_fs();

    myFILE *f = myfopen("/inner/inner2/a1.txt", "w+");

    // myfwrite("1 2", 1, 2, f);
    myfprintf(f, "%d %d", 15, 66);

    int x;
    int y;

    myfseek(f, 0, SEEK_SET);

    myfscanf(f, "%d %d", &x, &y);

    // myfscanf(f, "%d", &x);
    // myfscanf(f, "%d", &y);

    printf("++>%d %d\n", x, y);

    myfseek(f, 0, SEEK_SET);

    char buf[500];
    int b = myfread(buf, 1, 29, f);
    buf[b] = '\0';
    printf("%s\n", buf);

    return 0;
}
