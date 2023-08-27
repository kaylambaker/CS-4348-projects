#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../headers/printer.h"
#include "../headers/util.h"

uint MQS, CQS, NC, PRINTER_PORT, PT;
char *PRINTER_IP;
bool PRINTER_TERMINATE = false;

const char *PRINTER_SPOOL_DIR = "./.printerspool/";

Queue *connectionQueue; // queue of client socket file descriptors
sem_t connectionQueueMutex, connectionQueueEmpty, connectionQueueFull, sync_pc,
    *messageQueueMutexs, *messageQueueEmptys, *messageQueueFulls, mainBusy;
pthread_t printerMainFunctionThread, *communicatorThreads;
Queue **messageQueues;
int serverSocket;

int digit_count(unsigned int n) { return ceil(log10(n)); }

// concatenate CID + PID

/* void printer_init_spool(uint cid, uint pid) {
  char *spoolName;
  FILE *spoolFile;
  int spoolNameLen;
  // get length of spool name string ./spool/cid.pid
  spoolNameLen =
      digit_count(cid) + digit_count(pid) + strlen(PRINTER_SPOOL_DIR) + 1;
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, PRINTER_SPOOL_DIR, cid, pid);
  // open spool file
  if (!(spoolFile = fopen(spoolName, "w")))
    perror("printer_init_spool, spool file");
  // deallocate resources
  fclose(spoolFile);
  free(spoolName);
} */

void write_msg(PrintMsg *msg, char *buf) {
  char *spoolName;
  // get length of spool name string ./spool/cid.pid
  int spoolNameLen =
      snprintf(NULL, 0, SPOOL_NAME_FMT, PRINTER_SPOOL_DIR, msg->cid, msg->pid);
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, PRINTER_SPOOL_DIR, msg->cid, msg->pid);
  FILE *f = fopen(spoolName, "w");
  fwrite(buf, 1, msg->fileSize, f);
  fclose(f);
  free(buf);
  free(spoolName);
}

// consume
void *communicator(void *threadNoPtr) {
  int threadNo = *(int *)threadNoPtr, clientSocket;
  free(threadNoPtr);
  Queue *messageQueue = messageQueues[threadNo];

  while (!PRINTER_TERMINATE) {
    // consume connection queue
    // A free communicator thread waits to read from the connection queue
    sem_wait(&connectionQueueFull);
    sem_wait(&connectionQueueMutex);
    queue_dequeue(connectionQueue, &clientSocket);
    sem_post(&connectionQueueMutex);
    sem_post(&connectionQueueEmpty);
    printf("[COMMUNICATOR %d WON CONNECTION]\n", threadNo);

    // if succeeds, serves the connection
    // once wins a connection, reads from the socket
    while (!PRINTER_TERMINATE) {
      PrintMsg msg;
      ssize_t res;
      res = read(clientSocket, &msg, sizeof(PrintMsg));
      if (res == 0) { // client closed socket
        printf("[COMMUNICATOR %d DISCONNECTED FROM CLIENT]\n", threadNo);
        close(clientSocket);
        break;
      }
      if (res == -1) {
        // TODO: handle error
        // else {
        perror("error reading client socket");
        exit(1);
        // }
      }
      char *buf = malloc(msg.fileSize);
      read(clientSocket, buf, msg.fileSize);
      if (res == 0) { // client closed socket
        close(clientSocket);
        break;
      }
      if (res == -1) {
        // TODO: handle error
        // else {
        perror("error reading client socket");
        exit(1);
        // }
      }

      // place the read messages in its message queue
      // produce message queue
      sem_wait(messageQueueEmptys + threadNo);
      sem_wait(messageQueueMutexs + threadNo);
      write_msg(&msg, buf);
      queue_enqueue(messageQueue, &msg);
      sem_post(messageQueueMutexs + threadNo);
      sem_post(messageQueueFulls + threadNo);
      sem_post(&sync_pc);
    }
  }
  return NULL;
}

void printer_manager(void) {
  int clientSocket;
  // Call printer_manager_init(…)
  printer_manager_init();
  // Listen on the server socket for connections
  while (!PRINTER_TERMINATE) {
    // When receiving a connection, put it in the connection queue
    clientSocket = accept(serverSocket, NULL, NULL);
    // produce on connection queue
    sem_wait(&connectionQueueEmpty);
    sem_wait(&connectionQueueMutex);
    queue_enqueue(connectionQueue, &clientSocket);
    sem_post(&connectionQueueMutex);
    sem_post(&connectionQueueFull);
  }
}
void printer_manager_init(void) {
  uint *threadNoInt;
  // Create and initialize a “connection queue” of size 𝑪𝑸�
  connectionQueue = queue_create(sizeof(int));
  // Create necessary semaphores and initialize them for synchronization control
  // on the connection queue
  sem_init(&connectionQueueMutex, 0, 1);
  sem_init(&connectionQueueEmpty, 0, CQS);
  sem_init(&connectionQueueFull, 0, 0);
  sem_init(&mainBusy, 0, 1);
  // Create a printer thread
  assert(pthread_create(&printerMainFunctionThread, NULL, printer_main, NULL) ==
         0);
  // Create NC communicator threads, queues, and semaphores
  communicatorThreads = malloc(sizeof(pthread_t) * NC);
  messageQueueMutexs = malloc(sizeof(sem_t) * NC);
  messageQueueFulls = malloc(sizeof(sem_t) * NC);
  messageQueueEmptys = malloc(sizeof(sem_t) * NC);
  messageQueues = malloc(sizeof(Queue *) * NC);
  for (uint i = 0; i < NC; i++) {
    threadNoInt = malloc(sizeof(uint));
    *threadNoInt = i;
    messageQueues[i] = queue_create(sizeof(PrintMsg));
    sem_init(messageQueueMutexs + i, 0, 1);
    sem_init(messageQueueFulls + i, 0, 0);
    sem_init(messageQueueEmptys + i, 0, MQS);
    assert(pthread_create(communicatorThreads + i, NULL, communicator,
                          (void *)threadNoInt) == 0);
  }

  // Create a server socket to listen to connections.
  //  create socket
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  // define address
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PRINTER_PORT);
  inet_aton(PRINTER_IP, &serverAddr.sin_addr);
  // bind socket to address
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) !=
      0) {

    printf("unable to bind to port %d of %s: %s", PRINTER_PORT, PRINTER_IP,
           strerror(errno));
    exit(34);
  }
  listen(serverSocket, SOMAXCONN);
  printf("[CREATED SERVER SOCKET WITH ADDRESS %s PORT %d]\n",
         inet_ntoa(serverAddr.sin_addr), htons(serverAddr.sin_port));

  // After finishing, print a message to indicate the readiness of the printer
  // manager
  puts("[PRINTER MANAGER READY]");
}

void printer_init() {
  // Initialize the simulated paper (like before).
  FILE *paper;
  if (!(paper = fopen(PAPER, "w"))) {
    puts("unable to open printer.out");
    exit(1);
  }
  if (mkdir(PRINTER_SPOOL_DIR, 0777) != 0) {
    if (errno == EEXIST) {                 // if .spool file exists
      if (!empty_dir(PRINTER_SPOOL_DIR)) { // try to empty/delete it
        puts("file .spool already exitst\ndelete the file and run again");
        exit(1);
      }
    } else {
      puts("unable to create .spool directory");
      exit(1);
    }
  }
  fclose(paper);
  // Create and initialize a sync_pc semaphore for synchronizing between the
  // printer and the communicators.
  sem_init(&sync_pc, 0, 0);
  // After finish initialing, you can print a message to indicate the readiness
  // of the printer
  puts("[PRINTER THREAD READY]");
}

void printer_end_spool(PrintMsg *msg) {
  printf("[ENDING SPOOL %d.%d]\n", msg->cid, msg->pid);
  FILE *paperOut, *spoolFile;
  char *spoolName, *lineptr = NULL;
  const char *partialOutputMsg = "[PARTIAL OUTPUT]\n";
  int spoolNameLen;
  size_t n;
  if (!(paperOut = fopen(PAPER, "a+"))) {
    perror("printer_end_spool, open printer.out");
    fclose(paperOut);
    return;
  }
  fprintf(paperOut, "[PRINT ID %d.%d]\n", msg->cid, msg->pid);
  // get length of spool name string ./spool/cid.pid
  spoolNameLen =
      snprintf(NULL, 0, SPOOL_NAME_FMT, PRINTER_SPOOL_DIR, msg->cid, msg->pid);
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, PRINTER_SPOOL_DIR, msg->cid, msg->pid);
  // open spool file
  if (!(spoolFile = fopen(spoolName, "r"))) {
    perror("printer_end_spool, spool file");
    fclose(spoolFile);
    free(spoolName);
    return;
  }
  // read spool file
  while (getline(&lineptr, &n, spoolFile) != -1) {
    fwrite(lineptr, sizeof(char), strlen(lineptr), paperOut);
    free(lineptr);
    lineptr = NULL;
    usleep(PT); // sleep each line
  }
  if (!feof(spoolFile)) { // if error on getline
    perror("printer_end_spool getline");
    free(lineptr);
  }
  if (msg->type == CPU_TERM) { // partial output message
    fwrite(partialOutputMsg, sizeof(char), strlen(partialOutputMsg), paperOut);
  }
  // deallocate resources
  fclose(paperOut);
  fclose(spoolFile);
  remove(spoolName);
  free(spoolName);
  printf("[PRINT ID %d.%d PRINTING COMPLETE]\n", msg->cid, msg->pid);
}

void *printer_main(void *nil) {
  printer_init();
  PrintMsg msg;
  uint *queueLens = malloc(NC * sizeof(uint));
  memset(queueLens, 0, sizeof(uint) * NC); // zero out queueLens
  // Loop till the termination flag is on
  while (!PRINTER_TERMINATE) {
    memset(queueLens, 0, NC * sizeof(uint));
    // Wait on sync_pc semaphore.
    sem_wait(&sync_pc); // just check if there is something
    sem_wait(&mainBusy);
    sem_post(&sync_pc); // post immediately so reset is easier
    // Once through sync_pc, do a preliminary check on each message queue and
    // find the ones that are not empty
    for (uint i = 0; i < NC; i++) {
      sem_wait(messageQueueMutexs + i);
      queueLens[i] = messageQueues[i]->len;
      sem_post(messageQueueMutexs + i);
    }
    printf("[PRINTER READING REQUESTS FROM MESSAGE QUEUE]\n");
    // Read printer commands from the non-empty message queues.
    for (uint i = 0; i < NC; i++) {            // for each queue
      for (int j = 0; j < queueLens[i]; j++) { // for lenght of queue at i
        // consume message queue
        sem_wait(
            &sync_pc); // Reset sync_pc semaphore (careful design needed)
                       // decrament semaphore each time read message from queue
        sem_wait(messageQueueFulls + i);
        sem_wait(messageQueueMutexs + i);
        queue_dequeue(messageQueues[i], &msg);
        sem_post(messageQueueMutexs + i);
        sem_post(messageQueueEmptys + i);
        printer_end_spool(&msg);
      }
    }
    printf("[PRINTER FINISH READING REQUESTS FROM MESSAGE QUEUE]\n");
    sem_post(&mainBusy);
  }
  free(queueLens);
  return NULL;
}

void final_check_message_queues(void) {
  /* DIR *spoolDir;
  int pid, cid, len;
  char *cidStr, *pidStr, *fileName, *buf, *delim = ".";

  struct dirent *file;
  if (!(spoolDir = opendir(PRINTER_SPOOL_DIR))) {
    printf("unable to open .spool directory");
    return;
  }
  // for every file in current working directory
  while ((file = readdir(spoolDir))) {
    fileName = malloc(strlen(file->d_name) + 1);
    strcpy(fileName, file->d_name);
    cidStr = strtok(fileName, delim);
    if (!cidStr) {
      free(fileName);
      continue;
    }
    pidStr = strtok(NULL, delim);
    if (!pidStr) {
      free(fileName);
      continue;
    }
    cid = atoi(cidStr);
    pid = atoi(pidStr);
    PrintMsg msg = {cid, pid, 0, END_SPOOL}; // fileSize shouldnt be important
    printer_end_spool(&msg);
  }

  closedir(spoolDir); */
  PrintMsg msg;
  for (uint i = 0; i < NC; i++) {
    while (!queue_empty(messageQueues[i])) {
      queue_dequeue(messageQueues[i], &msg);
      printer_end_spool(&msg);
    }
  }
  printf("[PRINTER FINISH READING REQUESTS FROM MESSAGE QUEUE]\n");
}

void printer_terminate(int signum) {
  puts("[TERMINATING PRINTER PROCESS]");
  PRINTER_TERMINATE = true;
  sem_wait(&mainBusy);
  pthread_cancel(printerMainFunctionThread); // kill main
  for (uint i = 0; i < NC; i++) {
    pthread_cancel(communicatorThreads[i]);
  }
  final_check_message_queues();
  for (uint i = 0; i < NC; i++) {
    queue_destroy(messageQueues[i]);
    sem_destroy(messageQueueFulls + i);
    sem_destroy(messageQueueEmptys + i);
    sem_destroy(messageQueueMutexs + i);
  }
  free(communicatorThreads);
  free(messageQueues);
  free(messageQueueFulls);
  free(messageQueueEmptys);
  free(messageQueueMutexs);
  queue_destroy(connectionQueue);

  if (rmdir(PRINTER_SPOOL_DIR) != 0) {
    printf("could not remove %s directory: %s\n", PRINTER_SPOOL_DIR,
           strerror(errno));
    exit(69);
  }
  exit(0);
}

int main(void) {
  char *lineptr = NULL;
  const char *delim = " \n";
  size_t n;
  FILE *configFile = fopen("config.sys", "r");
  if (!configFile) {
    puts("unable to read from file config.sys");
    exit(1);
  }
  if (getline(&lineptr, &n, configFile) == -1) {
    perror("reading config.sys");
    exit(1);
  }

  PRINTER_IP = strtok(lineptr, delim);
  PRINTER_PORT = atoi(strtok(NULL, delim));
  for (uint i = 0; i < 2; i++) { // don't need M or TQ
    strtok(NULL, delim);
  }
  PT = atoi(strtok(NULL, delim));
  NC = atoi(strtok(NULL, delim));
  CQS = atoi(strtok(NULL, delim));
  MQS = atoi(strtok(NULL, delim));

  signal(SIGINT,
         printer_terminate); // kill printer with ctrl + c and perform cleanup
  printer_manager();

  // deallocate resource
  free(lineptr);
  return 0;
}
