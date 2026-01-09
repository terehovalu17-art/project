CC = gcc
CFLAGS = -O2 -std=c11 -Wall -Wextra

OBJS = main.o image.o filters.o

all: image_craft

image_craft: $(OBJS)
	$(CC) $(CFLAGS) -o image_craft $(OBJS) -lm

main.o: main.c image.h filters.h
image.o: image.c image.h
filters.o: filters.c filters.h image.h

clean:
	del *.o image_craft.exe
