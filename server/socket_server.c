#include <stdio.h>
#include <string.h>
#include <pthread.h>

#if defined(WIN32) || defined(__WIN32)
  #define WINDOWS 1
  #define delay_ms(ms) Sleep(ms)
  #define FAIL_SOCKET(s) \
      closesocket(s); \
      WSACleanup(); \
      return 1;
  #define errno WSAGetLastError()

  #include <ws2tcpip.h>
  #include <winsock.h>
  #include <windows.h>
#else
  #define delay_ms(ms) usleep(1000*(ms))
  #define FAIL_SOCKET(s) \
      close(s); \
      return 1;
  #define SOCKET int
  #define INVALID_SOCKET (-1)

  #include <stdlib.h>
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
  selectStatus = select(sock+1, &fds, 0, 0, &tv);
  pthread_mutex_unlock(&lock_sockAccept);
  return selectStatus;
}

void *receiveRoutine(void *threadData) {
  char message[STRLEN+1];
  int recvLen;

  while (1) {
    recvLen = pollSelect(sockAccept);
    if (recvLen < 0) {
      break;
    } else if (recvLen > 0) {
      pthread_mutex_lock(&lock_sockAccept);
      recvLen = recv(sockAccept, message, STRLEN, 0);
      if (recvLen <= 0) {
        pthread_mutex_unlock(&lock_sockAccept);
        break;
      }
      pthread_mutex_unlock(&lock_sockAccept);
      if (recvLen > 0) {
        message[MIN(recvLen, STRLEN)] = '\0';
        PRINT("Other: %s", message);
        if (recvLen == STRLEN)
          PRINT("\n");
      }
    } else {
      delay_ms(100);
    }
  }

  PRINT("Other user has closed client. Exiting...\n")
  exit(-1);
  pthread_exit(NULL);
  return 0;
}

void *sendRoutine(void *threadData) {
  char message[STRLEN+1];
  int status;

  while (1) {
    if (fgets(message, sizeof(message), stdin)) {
      pthread_mutex_lock(&lock_sockAccept);
      status = send(sockAccept, message, strlen(message), 0);
      if (status < 0) {
        pthread_mutex_unlock(&lock_sockAccept);
        break;
      }
      pthread_mutex_unlock(&lock_sockAccept);
    }
  }
  PRINT("Other user has closed client. Exiting...\n")
  return 0;
}

int main(int argc, char *argv[]) {
  SOCKET sockListen = INVALID_SOCKET;
  struct sockaddr_in server, client;
  char c, option = 1;
  pthread_t receiveThread;
  pthread_mutex_init(&lock_sockAccept, NULL);

  #if WINDOWS
    PRINT("Initialising Winsock... ");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      PRINT("WSAStartup failed. Error Code : %d", errno);
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
  sendRoutine(NULL);

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
