#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#if defined(WIN32) || defined(__WIN32)
  #define WINDOWS 1
  #include <winsock.h>
  #include <windows.h>
  #define socket SOCKET
  #define FAIL_SOCKET(s) \
      closesocket(s); \
      WSACleanup(); \
      return 1;
  #define errno WSAGetLastError()
  #pragma comment(lib, "ws2_32.lib") //Winsock Library
#else
  #define FAIL_SOCKET(s) \
      close(s); \
      return 1;
  #define SOCKET int
  #define INVALID_SOCKET (-1)
  #include <unistd.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <errno.h>
#endif

#define PRINT(...) printf(__VA_ARGS__); fflush(stdout);
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

  PRINT("In receive routine\n");
  while (1) {
    recvLen = pollSelect(sockAccept);
    if (recvLen < 0) {
      break;
    } else if (recvLen > 0) {
      PRINT("Recv: Got data waiting, getting lock for retrieving...\n");
      pthread_mutex_lock(&lock_sockAccept);
      PRINT("Recv: got lock, retrieving data...\n");
      recvLen = recv(sockAccept, message, STRLEN, 0);
      if (recvLen < 0) {
        PRINT("Recv: Error retrieving data.\n");
        pthread_mutex_unlock(&lock_sockAccept);
        break;
      }
      pthread_mutex_unlock(&lock_sockAccept);
      if (recvLen > 0) {
        PRINT("Recv: Successfully got data. Printing...\n");
        message[MIN(recvLen, STRLEN)] = '\0';
        PRINT("Other: %s", message);
        if (recvLen == STRLEN)
          PRINT("\n");
      }
    } else {
      #if WINDOWS
        Sleep(100);
      #else
        usleep(100*1000);
      #endif
    }
  }

  PRINT("Other user has closed client. Exiting...")
  exit(-1);
  pthread_exit(NULL);
  return 0;
}

void *sendRoutine(void *threadData) {
  char message[STRLEN+1];
  int status;
  PRINT("In send routine\n");
  while (1) {
    PRINT("Waiting for keyboard input...\n");
    if (fgets(message, sizeof(message), stdin)) {
      PRINT("got input, getting lock...\n");
      pthread_mutex_lock(&lock_sockAccept);
      PRINT("got lock, sending data.\n");
      status = send(sockAccept, message, strlen(message), 0);
      if (status < 0) {
        PRINT("Failed to send data, closing.\n");
        pthread_mutex_unlock(&lock_sockAccept);
        break;
      }
      pthread_mutex_unlock(&lock_sockAccept);
    }
  }
  PRINT("Other user has closed client. Exiting...")
  return 0;
}

int main(int argc, char *argv[]) {
  SOCKET sockListen = INVALID_SOCKET;
  struct sockaddr_in server, client;
  int c, option =1;
  pthread_t receiveThread;
  pthread_mutex_init(&lock_sockAccept, NULL);

  #if WINDOWS
    PRINT("Initialising Winsock... ");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      PRINT("WSAStartup failed. Error Code : %d", errno;
      WSACleanup();
      return 1;
    }
    PRINT("initialised.\n");
  #endif

  //Create a socket
  if((sockListen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    PRINT("Could not create socket : %d", errno);
    FAIL_SOCKET(sockListen);
  }
  setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  PRINT("Socket created.\n");

  //Prepare the sockaddr_in structure
  server.sin_family      = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port        = htons(8888);

  //Bind
  if (bind(sockListen, (struct sockaddr*)&server, sizeof(server)) < 0) {
    PRINT("Bind failed with error code : %d", errno);
    FAIL_SOCKET(sockListen);
  }
  PRINT("Bind done\n");

  //Listen to incoming connections
  listen(sockListen, 3);

  //Accept incoming connection
  PRINT("Waiting for incoming connections... ");
  c = sizeof(struct sockaddr_in);
  sockAccept = accept(sockListen, (struct sockaddr*)&client, (socklen_t *)&c);
  if (sockAccept < 0) {
    PRINT("\nAccept failed with error code : %d", errno);
    FAIL_SOCKET(sockListen);
  }
  PRINT("accepted\n");

  pthread_create(&receiveThread, NULL, receiveRoutine, NULL);
  PRINT("Moving to send routine\n");
  sendRoutine(NULL);
  PRINT("Finished send routine, closing up.\n");

  #if WINDOWS
    closesocket(sockAccept);
    closesocket(sockListen);
    WSACleanup();
  #else
    close(sockAccept);
    close(sockListen);
  #endif

  pthread_mutex_destroy(&lock_sockAccept);
  pthread_exit(NULL);
  return 0;
}
