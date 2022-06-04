
CC = gcc
W = -Wall

.PHONEY : clean




file_prog: libmyfs.so libmylibc.so
	$(CC) $(W) -o file_prog ./libmyfs.so ./libmylibc.so



test: libmyfs.so libmylibc.so test.o 
	$(CC) $(W) -o test test.o ./libmyfs.so ./libmylibc.so



libmylibc.so: file.o
	$(CC) $(W) --shared -o libmylibc.so file.o

libmyfs.so: fs.o
	$(CC) $(W) --shared -o libmyfs.so fs.o



test.o: test.c
	$(CC) $(W) -c test.c


fs.o: fs.c fs.h
	$(CC) $(W) -fPIC  -c fs.c


file.o: file.c
	$(CC) $(W) -c file.c



clean:
	rm -rf *.o fs_program test fs_data *.so