CFLAGS=-Wall -g -pedantic

PROGS=mysh

mysh: mysh.c
	gcc $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o $(PROGS)