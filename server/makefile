EXECUTABLE=run.exe
LIBS=-lws2_32
CFLAGS=-Wall
SRCS=$(wildcard *.c)
OBJ=$(SRCS:.c=.o)
CC=gcc

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) 

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXECUTABLE)
