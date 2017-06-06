# SocketMessaging
A simple client/server solution for message passing with TCP socket over a local network

## Usage
1. Run `make` in the main directory to compile both the server and client programs
2. Start the server and then the client by running the programs `run` in the server and client folders respectively.
3. Type something in either the client or server program and press enter (might depend on how the terminal is set up) to send it to the other.

Note: If you're on Windows, the executables will be called `run.exe` instead.

## Possible customizations
* Change the IP address in the file `client/socket_client.c` to match the host IP address. This way, you can have the host and client on two different computers. Most likely, this will only work on a local network since firewalls will otherwise block the port.

## Limitations
* Messages larger than the buffer will be split and look like several messages

## Notes
* Pthreads are used for simultaneously receiving and sending messages
