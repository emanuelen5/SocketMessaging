#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(__WIN32)
  #include <winsock.h>
  #pragma comment(lib,"ws2_32.lib") //Winsock Library
#else
  #error "This won't build on other than Windows"
  #include <sys/socket.h>
  #include <sys/un.h>
#endif

#define PRINT(...) printf(__VA_ARGS__); fflush(stdout);

int main(int argc, char *argv[]) {
  WSADATA wsa;
  SOCKET s, new_socket;
  struct sockaddr_in server, client;
  int c;
  char *message;

  PRINT("\nInitialising Winsock...");
  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
  {
    PRINT("Failed. Error Code : %d",WSAGetLastError());
    return 1;
  }
    
  PRINT("Initialised.\n");
    
  //Create a socket
  if((s = socket(AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET)
  {
    PRINT("Could not create socket : %d", WSAGetLastError());
  }

  PRINT("Socket created.\n");
    
  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( 8888 );
    
  //Bind
  if( bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
  {
    PRINT("Bind failed with error code : %d", WSAGetLastError());
  }
    
  PRINT("Bind done\n");

  //Listen to incoming connections
  listen(s, 3);
    
  //Accept and incoming connection
  PRINT("Waiting for incoming connections...\n");
    
  c = sizeof(struct sockaddr_in);
  new_socket = accept(s, (struct sockaddr *)&client, &c);
  if (new_socket == INVALID_SOCKET)
  {
    PRINT("accept failed with error code : %d", WSAGetLastError());
  }
    
  PRINT("Connection accepted\n");

  //Reply to client
  message = "Hello Client, I have received your connection. But I have to go now, bye\n";
  send(new_socket, message, strlen(message), 0);
    
  getchar();

  closesocket(s);
  WSACleanup();
    
  return 0;
}