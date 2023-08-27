#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/computer.h"
#include "../headers/darray.h"

bool TERMINATE = false;
sem_t MUTEX;
uint CID, PRINTER_PORT;
char *PRINTER_IP;
pthread_t shellThread, computerThread;

void computer_poweroff(void) {
  process_destroy_readyQ();
  process_destroy_PCBs();
  mem_destroy();
  print_terminate();
  sem_destroy(&MUTEX);
  exit(0);
}

// Invokes the initialization functions in other modules.
void boot_system(uint M, uint TQ, uint PT) {
  mem_init(M);
  cpu_init(TQ);
  print_init(PT);
  process_init();
  shellThread = shell_init();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: %s [CID]\n", argv[0]);
    return 69;
  }
  CID = atoi(argv[1]);
  char *lineptr = NULL;
  const char *delim = " \n";
  uint M, TQ, PT;
  size_t n;
  FILE *configFile = fopen("config.sys", "r");
  sem_init(&MUTEX, 0, 1);
  if (!configFile) {
    puts("unable to read from file config.sys");
    exit(1);
  }
  // parse config file
  if (getline(&lineptr, &n, configFile) == -1) {
    perror("reading config.sys");
    exit(1);
  }
  PRINTER_IP = strtok(lineptr, delim);
  PRINTER_PORT = atoi(strtok(NULL, delim));
  M = atoi(strtok(NULL, delim));
  TQ = atoi(strtok(NULL, delim));
  PT = atoi(strtok(NULL, delim));

  // deallocate resources for parsing config
  fclose(configFile);
  // initalize
  boot_system(M, TQ, PT);
  // run scheduler
  process_execute();
  // wait for shell thread
  pthread_join(shellThread, NULL);
  // deallocate resource
  free(lineptr);
  computer_poweroff();
}
