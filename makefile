EXECUTABLE=run.exe
OBJ=socket_server.o
LIBS=-lws2_32

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	gcc $^ -o $@ $(LIBS)

socket_server.o: socket_server.c
	gcc -c $^ $(LIBS)

socket_client.o: socket_client.c
	gcc -c $^ $(LIBS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXECUTABLE)
