#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#if defined(WIN32) || defined(__WIN32)
  #include <winsock.h>
  #include <windows.h>
  #define delay(ms) Sleep(ms)
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
    return SOCKET_ERROR;
#define STRLEN 100
#define MIN(a, b) ((a)<(b)?(a):(b))

SOCKET sockAccept = INVALID_SOCKET;
pthread_mutex_t lock_sockAccept;

int pollSelect(SOCKET sock) {
  int selectStatus;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sock, &fds);

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  pthread_mutex_lock(&lock_sockAccept);
  selectStatus = select(sock, &fds, 0, 0, &tv);
  pthread_mutex_unlock(&lock_sockAccept);
  return selectStatus;
}

void *receiveRoutine(void *threadData) {
  char message[STRLEN+1];
  int recvLen;
  PRINT("In recv routine!");

  while (1) {
    recvLen = pollSelect(sockAccept);
    if (recvLen == SOCKET_ERROR) {
      break;
    } else if (recvLen > 0) {
      pthread_mutex_lock(&lock_sockAccept);
      PRINT("Recv: Got the lock, waiting for data...\n");
      recvLen = recv(sockAccept, message, STRLEN, 0);
      if (recvLen == SOCKET_ERROR) {
        pthread_mutex_unlock(&lock_sockAccept);
        break;
      }
      PRINT("Recv: got the data.\n");
      pthread_mutex_unlock(&lock_sockAccept);
      PRINT("Recv: Released lock.\n");
      if (recvLen > 0) {
        message[MIN(recvLen, STRLEN)] = '\0';
        PRINT("\nMessage received: '%s'", message); 
      }
    } else {
      delay(100);
    }
  }

  pthread_exit(NULL);
  return 0;
}

void *sendRoutine(void *threadData) {
  char message[STRLEN+1];
  int status;
  while (1) {
    PRINT("Send: Waiting for user input...\n");
    if (fgets(message, sizeof(message), stdin)) {
      PRINT("Send: Got user input. Acquiring lock...\n");
      pthread_mutex_lock(&lock_sockAccept);
      PRINT("Send: Got lock, sending data.\n");
      status = send(sockAccept, message, strlen(message), 0);
      if (status == SOCKET_ERROR) {
        PRINT("Other user has closed client. Closing...")
        break;
      }
      pthread_mutex_unlock(&lock_sockAccept);
      PRINT("Send: Released lock.\n");
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  WSADATA wsaData;
  SOCKET sockListen = INVALID_SOCKET;
  struct sockaddr_in server, client;
  int c;
  pthread_t receiveThread;
  pthread_mutex_init(&lock_sockAccept, NULL);

  PRINT("Socket error: %d\n", SOCKET_ERROR);

  PRINT("Initialising Winsock... ");
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    PRINT("WSAStartup failed. Error Code : %d", WSAGetLastError());
    WSACleanup();
    return 1;
  }
  PRINT("initialised.\n");

  //Create a socket
  if((sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
    PRINT("Could not create socket : %d", WSAGetLastError());
    FAIL_SOCKET(sockListen);
  }
  PRINT("Socket created.\n");

  //Prepare the sockaddr_in structure
  server.sin_family      = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port        = htons(8888);

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

  pthread_create(&receiveThread, NULL, receiveRoutine, NULL);
  sendRoutine(NULL);

  closesocket(sockAccept);
  closesocket(sockListen);
  WSACleanup();

  pthread_mutex_destroy(&lock_sockAccept);
  pthread_exit(NULL);

  return 0;
}
