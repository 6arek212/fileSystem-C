



# Operating Systems Assignment 6

#### A part of a university assignment

</br>

## Project Overview

In this project we have implemented a file system in C .

</br>

## How To Run & Notes 

</br>

    make all
    ./test
        


### (Please make sure to read the notes below relevant to the question being examed)


### We have created a small "demo" program for testing ! you can try it "./demo"



</br>

## Notes

"make all" will compile all files, create the libraries, and the exc file "test"

</br>

#### For **Section 1** all the relevant code is in "fs.c/h"

</br>

#### For **Section 2** all the relevant code is in "file.c/h"

</br>

A big part of "myfscanf" and "myfprintf" was taken from this implemntation : https://www.equestionanswers.com/c/c-printf-scanf-working-principle.php

Also we used some code from here: https://www.youtube.com/watch?v=n2AAhiujAqs

</br>

my file system will write to the disk automaticly only when "set_save_to_disk()" is set to "AUTO_MODE" , by defualt its set to "MANUAL MODE" which means to save the updated file system is by calling "sync_fs()" function , this was done so you can run the test multipule times and get the same result , in the test program we dont update the file system on the disk , only in RAM  , in other words "sync_fs()" will not be called at all !

</br>
</br>
</br>

we prepared two file systems: 

</br>


File system "fs_data1" will have: 

      "/"
       |
     "inner"
       |
    "inner2"
       |
     "a3.txt"

</br>


File system "fs_data2" will have: 

      "/"
       |                |        |         |          |         |      
     "inner-dir"     "dir2"    "dir3"     "dir4"    "dir5"    "dir6"        
       |
    "inner2-dir"
       |
    "myfile.txt"

</br>

Defualt values for upove file systems:

    BLOCK SIZE                  512
    NUMBER OF INODE             100
    NUMBER OF BLOCKS            1000
    NAME LENGTH                 256
    MAX OPENED FILES            1000

</br>


You can mount these files systems by using "mymount"

    mymount(const char *path);

    mymount("fs_data1");


    //to check that it works !

    int fd = myopen("/inner/inner2/a3.txt", O_CREAT);
    char buff[20];

    int b = myread(fd, buff, 10);
    buff[b] = '\0';
    printf("%s\n",buff);
    



</br>
</br>

## Authors

  **Tarik Husin**  - linkedin -> https://www.linkedin.com/in/tarik-husin-706754184/

  **Wissam Kabha**  - github -> https://github.com/Wissam111