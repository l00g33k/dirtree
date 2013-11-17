OBJS = diff2.o diff2mv.o dirtree.o dirtrutl.o dirtsort.o

CFLAGS = -g -I.

all : dirtree

dirtree : $(OBJS)

%.o : %.c
	gcc $(CFLAGS) -c $< -o $@

clean : 
	rm *.o
	rm dirtree



