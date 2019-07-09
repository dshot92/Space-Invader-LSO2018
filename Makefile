#! bin/bash
CC = gcc
CFLAGS = -lncurses -lpthread
OBJFILES = space_invaders.c
TARGET = space_invaders.o

$(TARGET):	$(OBJFILES)
	$(CC) $(OBJFILES) -o $(TARGET) $(CFLAGS)
	
clean:
	rm -f *.o
	clear

run:
	./$(TARGET)
