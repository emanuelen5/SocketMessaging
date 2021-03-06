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

SOCKET sockOpen = INVALID_SOCKET;
pthread_mutex_t lock_sockOpen;

int pollSelect(SOCKET sock) {
  int selectStatus;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sock, &fds);

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  pthread_mutex_lock(&lock_sockOpen);
  selectStatus = select(sock+1, &fds, 0, 0, &tv);
  pthread_mutex_unlock(&lock_sockOpen);
  return selectStatus;
}

void *receiveRoutine(void *threadData) {
  char message[STRLEN+1];
  int recvLen;

  while (1) {
    recvLen = pollSelect(sockOpen);
    if (recvLen < 0) {
      break;
    } else if (recvLen > 0) {
      pthread_mutex_lock(&lock_sockOpen);
      recvLen = recv(sockOpen, message, STRLEN, 0);
      if (recvLen <= 0) {
        pthread_mutex_unlock(&lock_sockOpen);
        break;
      }
      pthread_mutex_unlock(&lock_sockOpen);
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
      pthread_mutex_lock(&lock_sockOpen);
      status = send(sockOpen, message, strlen(message), 0);
      if (status < 0) {
        pthread_mutex_unlock(&lock_sockOpen);
        break;
      }
      pthread_mutex_unlock(&lock_sockOpen);
    }
  }
  PRINT("Other user has closed client. Exiting...\n")
  return 0;
}

int main(int argc, char *argv[]) {
  SOCKET sockConnect = INVALID_SOCKET;
  struct sockaddr_in client;
  pthread_t receiveThread;
  char option = 1;
  pthread_mutex_init(&lock_sockOpen, NULL);

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
  if((sockOpen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    PRINT("Could not create socket : %d", errno);
    FAIL_SOCKET(sockOpen);
  }
  setsockopt(sockOpen, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  PRINT("Socket created.\n");

  //Prepare the sockaddr_in structure
  client.sin_family      = AF_INET;
  client.sin_addr.s_addr = inet_addr("127.0.0.1");
  client.sin_port        = htons(8888);

  //Accept incoming connection
  PRINT("Connecting to server... ");
  sockConnect = connect(sockOpen, (struct sockaddr*)&client, sizeof(struct sockaddr_in));
  if (sockConnect < 0) {
    PRINT("\nConnect failed with error code : %d", errno);
    FAIL_SOCKET(sockOpen);
  }
  PRINT("accepted.\n");

  pthread_create(&receiveThread, NULL, receiveRoutine, NULL);
  sendRoutine(NULL);

  #if WINDOWS
    closesocket(sockOpen);
    WSACleanup();
  #else
    close(sockOpen);
  #endif

  pthread_mutex_destroy(&lock_sockOpen);
  pthread_exit(NULL);
  return 0;
}
