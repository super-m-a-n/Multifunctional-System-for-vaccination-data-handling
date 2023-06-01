SRC = ./src
OBJ = ./obj
STRUCTS = $(SRC)/structs
BASE = $(SRC)/base

CC = gcc
CFLAGS = -g -Wall -I. -I$(STRUCTS) -I$(BASE)

target: vaccineMonitor

OBJS = vaccineMonitor.o
OBJS += bloom.o hash.o list.o skip_list.o
OBJS += items.o monitor.o

bloom.o: $(STRUCTS)/bloom.c
	$(CC) $(CFLAGS) -c $(STRUCTS)/bloom.c
skip_list.o: $(STRUCTS)/skip_list.c
	$(CC) $(CFLAGS) -c $(STRUCTS)/skip_list.c
items.o: $(BASE)/items.c
	$(CC) $(CFLAGS) -c $(BASE)/items.c
list.o: $(STRUCTS)/list.c
	$(CC) $(CFLAGS) -c $(STRUCTS)/list.c
hash.o: $(STRUCTS)/hash.c
	$(CC) $(CFLAGS) -c $(STRUCTS)/hash.c
monitor.o: $(BASE)/monitor.c
	$(CC) $(CFLAGS) -c $(BASE)/monitor.c
vaccineMonitor.o: $(SRC)/vaccineMonitor.c
	$(CC) $(CFLAGS) -c $(SRC)/vaccineMonitor.c


vaccineMonitor: $(OBJS) 
	$(CC) $(CFLAGS) $(OBJS) -o vaccineMonitor
	mkdir -p $(OBJ)
	mv $(OBJS) $(OBJ)

.PHONY: clean

clean:
	rm -f vaccineMonitor
	rm -rf $(OBJ)
