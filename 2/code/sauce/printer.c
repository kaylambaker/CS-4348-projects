#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../headers/computer.h"
#include "../headers/printer.h"

#define SPOOL_DIR ("./.spool/")
#define SPOOL_NAME_FMT ("%s%d")
#define PAPER "printer.out"

int print[2], printer[2];

int digit_count(unsigned int n) { return ceil(log10(n)); }

bool empty_spool_dir(void) {
  char path[256];
  strcpy(path, SPOOL_DIR);
  if (remove(".spool") == 0) { // delete file and make directory
    if (mkdir(SPOOL_DIR, 0777) != 0)
      return false;
    return true;
  }
  DIR *dir;
  struct dirent *file;
  if (!(dir = opendir(SPOOL_DIR)))
    return false;
  while ((file = readdir(dir)) != NULL) {
    if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
      strcat(path, file->d_name);
      if (remove(path) == -1)
        return false;
      memset(path, 0, 256);
      strcpy(path, SPOOL_DIR);
    }
  }
  return true;
}

void printer_init() {
  PrintMsg ack = {{}, 0, PRINT_ACK};
  PrintMsg spoolFail = {"unable to create .spool directory", 0, TERMINATE};
  PrintMsg paperFail = {"unable to open printer.out", 0, TERMINATE};
  PrintMsg spoolDirExists = {"file .spool already exists\ndelete the "
                             "file and run again",
                             0, TERMINATE};
  FILE *paper;
  if (!(paper = fopen(PAPER, "w"))) {
    write(printer[WRITE], &paperFail, sizeof(PrintMsg));
    exit(1);
  }
  if (mkdir(SPOOL_DIR, 0777) != 0) {
    if (errno == EEXIST) {      // if .spool file exists
      if (!empty_spool_dir()) { // try to empty/delete it
        write(printer[WRITE], &spoolDirExists, sizeof(PrintMsg));
        exit(1);
      }
    } else {
      write(printer[WRITE], &spoolFail, sizeof(PrintMsg));
      exit(1);
    }
  }
  write(printer[WRITE], &ack, sizeof(PrintMsg)); // send ACK?
  fclose(paper);
}
void printer_init_spool(int pid) {
  char *spoolName;
  FILE *spoolFile;
  int spoolNameLen;
  // get length of spool name string
  spoolNameLen = digit_count(pid) + strlen(SPOOL_DIR);
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, SPOOL_DIR, pid);
  // open spool file
  if (!(spoolFile = fopen(spoolName, "w")))
    perror("printer_init_spool, spool file");
  // deallocate resources
  fclose(spoolFile);
  free(spoolName);
}
void printer_print(char *buf, uint pid) {
  char *spoolName;
  FILE *spoolFile;
  int spoolNameLen;
  // get length of spool name string
  spoolNameLen = digit_count(pid) + strlen(SPOOL_DIR);
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, SPOOL_DIR, pid);
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
void printer_end_spool(uint pid) {
  FILE *paperOut, *spoolFile;
  char *spoolName, *lineptr = NULL;
  int spoolNameLen;
  size_t n;
  if (!(paperOut = fopen(PAPER, "a+"))) {
    perror("printer_end_spool, open printer.out");
    return;
  }
  // get length of spool name string
  spoolNameLen = digit_count(pid) + strlen(SPOOL_DIR);
  // allocate memory for spoolName
  spoolName = malloc(sizeof(char) * spoolNameLen + 1);
  // copy spool name to spoolName
  sprintf(spoolName, SPOOL_NAME_FMT, SPOOL_DIR, pid);
  // open spool and paper
  if (!(spoolFile = fopen(spoolName, "r"))) {
    perror("printer_end_spool, open spool file");
    return;
  }
  fprintf(paperOut, "[PID %d]\n", pid);
  // read spool file
  while (getline(&lineptr, &n, spoolFile) != -1) {
    fwrite(lineptr, sizeof(char), strlen(lineptr), paperOut);
    free(lineptr);
    lineptr = NULL;
    usleep(PT); // sleep each line
  }
  if (errno) { // if error on getline
    perror("printer_end_spool getline");
    free(lineptr);
  }
  // deallocate resources
  fclose(paperOut);
  fclose(spoolFile);
  remove(spoolName);
  free(spoolName);
  printf("[PID %d PRINTING COMPLETE]\n", pid);
}
void printer_terminate() {
  DIR *spoolDir;
  FILE *paper;
  int pid;
  char *endptr = NULL;
  const char *partialOutputMsg = "[PARTIAL OUTPUT]\n";
  struct dirent *file;
  if (!(spoolDir = opendir(SPOOL_DIR))) {
    perror("printer_terminate, open .spool directory");
    exit(1);
  }

  // for every file in .spool directory
  while ((file = readdir(spoolDir))) {
    pid = strtol(file->d_name, &endptr, 10);
    if (*endptr) // if '.' or '..', skip
      continue;
    // end spool for pid
    printer_end_spool(pid);
    if (!(paper = fopen(PAPER, "a+"))) {
      perror("printer_terminate, open printer.out");
      continue;
    }
    fwrite(partialOutputMsg, sizeof(char), strlen(partialOutputMsg), paper);
    fclose(paper);
  }
  // deallocate resource
  closedir(spoolDir);
  if (rmdir(SPOOL_DIR) != 0) {
    perror("could not remove .spool directory");
    exit(69);
  }
  exit(0); // end forked process
}

void printer_dump_spool(uint callPID) {
  DIR *spoolDir;
  int pid, index = 0, len;
  char *buf, *endptr;
  const char *fmt = "%d: %d\n",
             *heading =
                 "==================\n   Printer Dump\n==================\n";
  struct dirent *file;
  if (!(spoolDir = opendir(SPOOL_DIR))) {
    printf("unable to open .spool directory");
    return;
  }
  printf("%s", heading);
  if (callPID)
    print_print(heading, callPID);
  // for every file in current working directory
  while ((file = readdir(spoolDir))) {
    // check if there is file name that ends with .spool
    // parse file name to get pid
    pid = strtol(file->d_name, &endptr, 10);
    if (*endptr)
      continue;

    len = snprintf(NULL, 0, fmt, index, pid);
    buf = malloc(len * sizeof(char) + 1);
    sprintf(buf, fmt, index, pid);
    printf("%s", buf);
    if (callPID)
      print_print(buf, callPID);
    index++;
    free(buf);
  }
  closedir(spoolDir);
}

void printer_main(void) {
  PrintMsg msg;
  printer_init();
  while (1) {
    read(print[READ], &msg, sizeof(PrintMsg));
    switch (msg.type) {
    case PRINT_REQ:
      printer_print(msg.buf, msg.pid);
      break;
    case INIT_SPOOL:
      printer_init_spool(msg.pid);
      break;
    case END_SPOOL:
      printer_end_spool(msg.pid);
      break;
    case PRINT_TERM:
      printer_terminate();
    case PRINT_ACK:
      break;
    default:
      puts("invalid print message");
      break;
    }
  }
}
