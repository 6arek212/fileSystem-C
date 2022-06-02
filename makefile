
CC = gcc
W = -Wall

.PHONEY : clean



fs_program: fs.o
	$(CC) $(W) -o fs_program fs.o


fs.o: fs.c fs.h
	$(CC) $(W) -c fs.c


clean:
	rm -rf *.o fs_program