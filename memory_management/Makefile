CC = gcc
CFLAGS = -Wall -Werror -pedantic -std=c99 -g
OBJ = mmu.o list.o util.o

mmu: $(OBJ)
	$(CC) $(CFLAGS) -o mmu $(OBJ)

mmu.o: mmu.c list.h util.h
	$(CC) $(CFLAGS) -c mmu.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

clean:
	rm -f mmu $(OBJ)
