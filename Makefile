CC=gcc # compilador
CFLAGS=-c -g -Wall -std=c99 #flags para el compilador
LDFLAGS= #flags para enlazador

SOURCES=my_shell.c #nivel2.c nivel3.c nivel4.c nivel5.c nivel6.c my_shell.c
LIBRARIES=#.o
INCLUDES=my_shell.h #.h
PROGRAMS=my_shell #nivel2 nivel3 nivel4 nivel5 nivel6  my_shell

OBJS=$(SOURCES:.c=.o)

all: $(OBJS) $(PROGRAMS)

#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@

my_shell: my_shell.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@

%.o: %.c $(INCLUDES) 
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf *.o $(PROGRAMS)
