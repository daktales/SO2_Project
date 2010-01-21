CC=gcc
LDFLAGS=-pthread
CFLAGS=-c -Wall -Wextra -pedantic
SOURCES=write.c fun.c buffer.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=writer

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
				$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
				$(CC) $(CFLAGS) $< -o $@

clean:
				rm -rf *o $(EXECUTABLE)
