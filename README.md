# SocketMessaging
Message passing with TCP socket

## Usage
Run the following three commands to run the program:
1. `make`: Compile both the client and server code. 
2. `./server/run.exe`: Start the server
3. `./client/run.exe`: Start the client
Type something in either the client or server program and press enter (might depend on how the terminal is set up) to send it to the other.

## Possible customizations
* Change the IP address in the file `client/socket_client.c` to match the host IP address. 

## Limitations
* Currently only compiles for Windows
* Messages larger than the buffer will be split

## Notes
* Pthreads are used for simultaneously receiving and sending messages
