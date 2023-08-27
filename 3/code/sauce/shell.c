#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../headers/computer.h"

static const char *prompt = "shell> ";

pthread_t shell_init(void) {
  pthread_t p;
  assert(pthread_create(&p, NULL, shell_thread, NULL) == 0);
  return p;
}

void *shell_thread(void *nil) {
  static char *lineptr = NULL, *token, *endptr;
  size_t n;
  int cmd;
  while (!TERMINATE) {
    printf("%s", prompt);
    if (getline(&lineptr, &n, stdin) == -1) {
      if (errno)
        perror("shell_thread getline error");
      else
        puts("[ERROR READING FROM STDIN TERMINATING]");
      free(lineptr);
      TERMINATE = true;
      continue;
    }
    token = strtok(lineptr, " \n");
    if (!token) {
      free(lineptr);
      lineptr = NULL;
      continue;
    }
    cmd = strtol(token, &endptr, 10);
    if (*endptr) { // if not int
      free(lineptr);
      lineptr = NULL;
      puts("[INVALID SHELL COMMAND]");
      continue;
    }
    sem_wait(&MUTEX);
    shell_command(cmd, 0);
    sem_post(&MUTEX);
    free(lineptr);
    lineptr = NULL;
  }
  return NULL;
}

void shell_print_registers(uint pid) {
  char *buf;
  const char *fmt =
      "=================\n  Register Dump\n=================\nRegister: "
      "Contents\nBASE: %d\nPC: %d\nIR0: %d\nIR1 %d\nAC: %d\nMAR: %d\nMBR: %d\n";
  int len = snprintf(NULL, 0, fmt, BASE, PC, IR0, IR1, AC, MAR, MBR);
  buf = malloc(len * sizeof(char) + 1);
  sprintf(buf, fmt, BASE, PC, IR0, IR1, AC, MAR, MBR);
  printf("%s", buf);
  if (pid)
    print_print(buf, CID, pid);
  free(buf);
}
void shell_print_memory(unsigned int pid) {
  const char *fmt = "%d: %d\n",
             *heading = "=====================\n Memory Dump k = %d "
                        "\n=====================\nAddress: Contents\n";
  char *buf;
  int len = snprintf(NULL, 0, heading, mem_getsize());
  buf = malloc(len * sizeof(char) + 1);
  sprintf(buf, heading, mem_getsize());
  printf("%s", buf);
  if (pid)
    print_print(buf, CID, pid);
  free(buf);
  for (MAR = 0; MAR < mem_getsize(); MAR++) {
    mem_read();
    len = snprintf(NULL, 0, fmt, MAR, MBR);
    buf = malloc(len * sizeof(char) + 1);
    sprintf(buf, fmt, MAR, MBR);
    printf("%s", buf);
    if (pid)
      print_print(buf, CID, pid);
    free(buf);
  }
}

void shell_command(uint8_t cmd, uint pid) {
  char *lineptr = NULL, *fname, *baseToken, *endptr;
  int base;
  size_t n;
  switch (cmd) {
  case 0:
    TERMINATE = true;
    break;
  case 1:
    sem_post(&MUTEX);
    printf("enter file name and base> ");
    if (getline(&lineptr, &n, stdin) == -1) {
      perror("shell_command getline");
      free(lineptr);
      lineptr = NULL;
      sem_wait(&MUTEX);
      return;
    }
    fname = strtok(lineptr, " \n");
    baseToken = strtok(NULL, " \n");
    if (!fname || !baseToken) {
      puts("[INVALID INPUT]");
      free(lineptr);
      sem_wait(&MUTEX);
      return;
    }
    base = strtol(baseToken, &endptr, 10);
    if (*endptr) {
      free(lineptr);
      puts("[INVALID BASE]");
      return;
    }
    sem_wait(&MUTEX);
    if (load_prog(fname, base))
      process_submit(fname, base);
    free(lineptr);
    break;
  case 2:
    shell_print_registers(pid);
    break;
  case 3:
    shell_print_memory(pid);
    break;
  case 4:
    process_dump_readyQ(pid);
    break;
  case 5:
    process_dump_PCB(pid);
    break;
  case 6:
    printer_dump_spool(pid);
    break;
  default:
    puts("[INVALID SHELL COMMAND]");
    break;
  }
}
