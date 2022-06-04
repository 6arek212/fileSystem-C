
CC = gcc
W = -Wall

.PHONEY : clean




file_prog: fs.o file.o
	$(CC) $(W) -o file_prog fs.o file.o

fs_program: fs.o
	$(CC) $(W) -o fs_program fs.o



fs.o: fs.c fs.h
	$(CC) $(W) -c fs.c


file.o: file.c
	$(CC) $(W) -c file.c



clean:
	rm -rf *.o fs_program