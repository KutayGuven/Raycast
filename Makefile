CC=gcc
CFLAGS=-lm -l ncurses
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $^
raycast: main.c
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm *.o
