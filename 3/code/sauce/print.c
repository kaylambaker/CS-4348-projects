#include <arpa/inet.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../headers/computer.h"
#include "../headers/printer.h"
#include "../headers/util.h"

int clientSocket;

// concatenate CID + PID
uint concatPrintID(uint cid, uint pid) {
  uint printID;
  const char *fmt = "%d.%d";
  // concatentate
  uint len = snprintf(NULL, 0, fmt, cid, pid);
  char *IDString = malloc(len * sizeof(char) + 1);
  sprintf(IDString, fmt, cid, pid);
  printID = atoi(IDString);
  free(IDString);
  return printID;
}

// call print init before printer init
// idk what the hell ACK is - i figured it out
void print_init(uint pt) {
  // prepare spool directory
  if (mkdir(SPOOL_DIR, 0777) != 0) {
    if (errno == EEXIST) { // if .spool file exists
      printf("[THERE IS ALREADY A %s DIRECTORY IN THE CURRENT WORKING "
             "DIRECTORY]\n[THE CONTENTS OF THE DIRECTORY MAY BE READ, "
             "MODIFIED, OR DELETED]",
             SPOOL_DIR);
    } else {
      puts("unable to create .spool directory");
      exit(1);
    }
  }
  // Set up a socket connection to the printer instance.
  // define address data structure
  struct sockaddr_in *printerAddrStruct;
  if (clientSocket == -1) {
    printf("could not create client socket\n");
    exit(3);
  }
  struct addrinfo *info, hints = {0};
  hints.ai_family = AF_INET; // try get IP4
  hints.ai_socktype = SOCK_STREAM;
  int ret = getaddrinfo(PRINTER_IP, NULL, &hints, &info);
  if (ret != 0) {
    printf("print_init: %s\n", gai_strerror(ret));
    exit(69);
  }
  if (info->ai_family !=
      AF_INET) { // if not IPv4 give up cause this class is too hard and i cant
                 // be bothered to make it work with IPv4 and IPv6
    printf("could not find IPv4 address for %s", PRINTER_IP);
    exit(69);
  }
  printerAddrStruct = (struct sockaddr_in *)(info->ai_addr);
  printerAddrStruct->sin_port = htons(PRINTER_PORT);
  clientSocket = socket(printerAddrStruct->sin_family, SOCK_STREAM, 0); // IPv4
  // connect to server
  puts("[CONNECTING TO PRINTER]");
  if (connect(clientSocket, (struct sockaddr *)printerAddrStruct,
              info->ai_addrlen) != 0) {
    printf("unable to connect to address %s port %d: %s ", PRINTER_IP,
           PRINTER_PORT, strerror(errno));
    exit(34);
  } else {
    puts("[CONNECTED TO PRINTER]");
  }
}
void print_init_spool(uint cid, uint pid) {
  char *spoolName;
  FILE *spoolFile;
  int spoolNameLen;
  // get length of spool name string ./spool/cid.pid
  spoolNameLen = snprintf(NULL, 0, SPOOL_NAME_FMT, SPOOL_DIR, cid, pid);
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, SPOOL_DIR, cid, pid);
  // open spool file
  if (!(spoolFile = fopen(spoolName, "w")))
    perror("print_init_spool, spool file");
  // deallocate resources
  fclose(spoolFile);
  free(spoolName);
}
void print_end_spool(uint cid, uint pid, MsgType type) {
  int spoolFile;
  char *spoolName;
  int spoolNameLen;
  // get length of spool name string ./spool/cid.pid
  spoolNameLen = snprintf(NULL, 0, SPOOL_NAME_FMT, SPOOL_DIR, cid, pid);
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, SPOOL_DIR, cid, pid);
  // open spool and paper
  if ((spoolFile = open(spoolName, O_RDONLY)) == -1) {
    perror("printer_end_spool, open spool file");
    return;
  }
  // send spool file through pipe
  struct stat st;
  fstat(spoolFile, &st);
  PrintMsg msg = {cid, pid, st.st_size, type};
  write(clientSocket, &msg, sizeof(PrintMsg));      // send file size
  sendfile(clientSocket, spoolFile, 0, st.st_size); // send file
  // deallocate resource
  close(spoolFile);
  remove(spoolName);
  free(spoolName);
}
void print_print(const char *buf, uint cid, uint pid) {
  char *spoolName;
  FILE *spoolFile;
  int spoolNameLen;
  // get length of spool name string ./spool/cid.pid
  spoolNameLen = snprintf(NULL, 0, SPOOL_NAME_FMT, SPOOL_DIR, cid, pid);
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, SPOOL_DIR, cid, pid);
  // open spool file
  if (!(spoolFile = fopen(spoolName, "a+"))) {
    perror("printer_print, open spool file");
    return;
  }
  // write to spool file
  fwrite(buf, sizeof(char), strlen(buf), spoolFile);
  // deallocate resources
  fclose(spoolFile);
  free(spoolName);
}

void printer_dump_spool(uint callPID) {
  DIR *spoolDir;
  int pid, cid, index = 0, len;
  char *cidStr, *pidStr, *fileName, *buf;
  const char *fmt = "%d: %s\n",
             *heading =
                 "==================\n   Printer Dump\n==================\n",
             *delim = ".";

  struct dirent *file;
  if (!(spoolDir = opendir(SPOOL_DIR))) {
    printf("unable to open .spool directory");
    return;
  }
  printf("%s", heading);
  if (callPID)
    print_print(heading, CID, callPID);
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
    if (cid != CID) {
      free(fileName);
      continue;
    }

    len = snprintf(NULL, 0, fmt, index, file->d_name);
    buf = malloc(len * sizeof(char) + 1);
    sprintf(buf, fmt, index, cid, pid);
    printf("%s", buf);
    if (callPID)
      print_print(buf, CID, callPID);
    index++;
    free(buf);
  }
  closedir(spoolDir);
}

void print_terminate(void) {
  // open spool file
  DIR *spoolDir;
  uint fileCID, filePID;
  char *tempFileName, *cidStr, *pidStr;
  const char *delim = ".";
  struct dirent *file;
  if (!(spoolDir = opendir(SPOOL_DIR))) {
    perror("printer_terminate, open .spool directory");
    exit(1);
  }

  // for every file in .spool directory
  while ((file = readdir(spoolDir))) {
    // copy name for strtok
    tempFileName = malloc(strlen(file->d_name) * sizeof(char) + 1);
    strcpy(tempFileName, file->d_name);

    cidStr = strtok(tempFileName, delim);
    if (!cidStr) {
      free(tempFileName);
      continue;
    }
    pidStr = strtok(NULL, delim);
    if (!pidStr) {
      free(tempFileName);
      continue;
    }
    fileCID = atoi(cidStr);
    filePID = atoi(pidStr);
    if (fileCID != CID) {
      free(tempFileName);
      continue;
    }
    // end spool for pid
    print_end_spool(CID, filePID, CPU_TERM);
    free(tempFileName);
  }
  /* if (rmdir(SPOOL_DIR) != 0) {
    printf("could not remove %s directory: %s\n", SPOOL_DIR, strerror(errno));
  } */
  close(clientSocket);
}
