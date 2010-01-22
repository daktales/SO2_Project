CC=gcc
LDFLAGS=-pthread -static
CFLAGS=-c -Wall -Wextra -pedantic
SOURCES=read.c fun.c buffer.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=reader

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
				$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
				$(CC) $(CFLAGS) $< -o $@

clean:
				rm -rf *o $(EXECUTABLE)
