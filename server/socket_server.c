#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(__WIN32)
  #include <winsock.h>
  #pragma comment(lib, "ws2_32.lib") //Winsock Library
#else
  #warning "This is currently only for Windows"
  #include <sys/socket.h>
  #include <sys/un.h>
#endif

#define PRINT(...) printf(__VA_ARGS__); fflush(stdout);
#define FAIL_SOCKET(s) \
    closesocket(s); \
    WSACleanup(); \
    return 1;

int main(int argc, char *argv[]) {
  WSADATA wsaData;
  SOCKET sockListen = INVALID_SOCKET, sockAccept = INVALID_SOCKET;
  struct sockaddr_in server, client;
  int c;
  char *message;

  PRINT("Initialising Winsock... ");
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    PRINT("WSAStartup failed. Error Code : %d", WSAGetLastError());
    WSACleanup();
    return 1;
  }
  PRINT("Initialised.\n");

  //Create a socket
  if((sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
    PRINT("Could not create socket : %d", WSAGetLastError());
    FAIL_SOCKET(sockListen);
  }
  PRINT("Socket created.\n");

  //Prepare the sockaddr_in structure
  server.sin_family      = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port        = htons(80);

  //Bind
  if (bind(sockListen, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
    PRINT("Bind failed with error code : %d", WSAGetLastError());
    FAIL_SOCKET(sockListen);
  }
  PRINT("Bind done\n");

  //Listen to incoming connections
  listen(sockListen, 3);

  //Accept incoming connection
  PRINT("Waiting for incoming connections... ");
  c = sizeof(struct sockaddr_in);
  sockAccept = accept(sockListen, (struct sockaddr*)&client, &c);
  if (sockAccept == INVALID_SOCKET) {
    PRINT("\nAccept failed with error code : %d", WSAGetLastError());
    FAIL_SOCKET(sockListen);
  }
  PRINT("Accepted\n");

  //Reply to client
  message = "1. Hello Client, I have received your connection";
  send(sockAccept, message, strlen(message), 0);
  message = "2. Still there... Ehm.";
  send(sockAccept, message, strlen(message), 0);
  message = "3. Gotta go now!";
  send(sockAccept, message, strlen(message), 0);

  closesocket(sockAccept);
  closesocket(sockListen);
  WSACleanup();

  return 0;
}