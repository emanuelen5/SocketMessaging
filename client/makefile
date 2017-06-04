EXECUTABLE=run.exe
LIBS=-lws2_32 -lpthread
CFLAGS=-Wall
SRCS=$(wildcard *.c)
OBJ=$(SRCS:.c=.o)
CC=gcc
.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) 

clean:
	rm -f $(OBJ) $(EXECUTABLE)
