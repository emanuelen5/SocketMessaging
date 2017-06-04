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
#define STRLEN 100
#define MIN(a, b) ((a)<(b)?(a):(b))

int main(int argc, char *argv[]) {
  WSADATA wsaData;
  SOCKET sockOpen = INVALID_SOCKET, sockConnect = INVALID_SOCKET;
  struct sockaddr_in client;
  int c;
  char message[STRLEN+1];

  PRINT("Initialising Winsock... ");
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    PRINT("WSAStartup failed. Error Code : %d", WSAGetLastError());
    WSACleanup();
    return 1;
  }
  PRINT("initialised.\n");

  //Create a socket
  if((sockOpen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
    PRINT("Could not create socket : %d", WSAGetLastError());
    FAIL_SOCKET(sockOpen);
  }
  PRINT("Socket created.\n");

  //Prepare the sockaddr_in structure
  client.sin_family      = AF_INET;
  client.sin_addr.s_addr = inet_addr("127.0.0.1");
  client.sin_port        = htons(80);

  //Accept incoming connection
  PRINT("Connecting to server... ");
  c = sizeof(struct sockaddr_in);
  sockConnect = connect(sockOpen, (struct sockaddr*)&client, c);
  if (sockConnect == INVALID_SOCKET) {
    PRINT("\nConnect failed with error code : %d", WSAGetLastError());
    FAIL_SOCKET(sockOpen);
  }
  PRINT("accepted.\n");

  //Get message from server
  c = recv(sockOpen, message, STRLEN, 0);
  message[MIN(c, STRLEN)] = '\0';
  PRINT("\nMessage received: '%s'", message);
  c = recv(sockOpen, message, STRLEN, 0);
  message[MIN(c, STRLEN)] = '\0';
  PRINT("\nMessage received: '%s'", message);
  c = recv(sockOpen, message, STRLEN, 0);
  message[MIN(c, STRLEN)] = '\0';
  PRINT("\nMessage received: '%s'", message);
  c = recv(sockOpen, message, STRLEN, 0);
  message[MIN(c, STRLEN)] = '\0';
  PRINT("\nMessage received: '%s'", message);

  closesocket(sockOpen);
  WSACleanup();

  return 0;
}