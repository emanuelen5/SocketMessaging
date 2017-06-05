EXECUTABLE=run
LIBS=-lpthread
CFLAGS=-Wall
SRCS=$(wildcard *.c)
OBJ=$(SRCS:.c=.o)
CC=gcc
.PHONY: all clean

# Add linking to Winsocket for Windows
ifeq ($(OS),Windows_NT)
  EXECUTABLE +=.exe
  LIBS += -lsw2_32
endif

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) 

clean:
	rm -f $(OBJ) $(EXECUTABLE)
