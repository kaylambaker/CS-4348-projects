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

int main(void) {
  char *lineptr = NULL;
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
  M = atoi(strtok(lineptr, " \n"));
  TQ = atoi(strtok(NULL, " \n"));
  PT = atoi(strtok(NULL, " \n"));
  // deallocate resources for parsing config
  free(lineptr);
  fclose(configFile);
  // initalize
  boot_system(M, TQ, PT);
  // run scheduler
  process_execute();
  // wait for shell thread
  pthread_join(shellThread, NULL);
  // deallocate resource
  computer_poweroff();
}
