#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "computer.h"

typedef struct {
  int ID, PC, AC, BASE;
} PCB;

PCB RUNNING_PROGRAM;
int processCount = 0; // used to assign process ID

void process_init_PCB() {
  RUNNING_PROGRAM.AC = 0;
  RUNNING_PROGRAM.BASE = BASE;
  RUNNING_PROGRAM.ID = processCount++;
}
void process_set_registers() {
  PC = RUNNING_PROGRAM.PC;
  AC = RUNNING_PROGRAM.AC;
  BASE = RUNNING_PROGRAM.BASE;
}

// Invokes the initialization functions in other modules.
void boot_system(int k) {
  mem_init(k);
  shell_init();
}

int main(int argc, char **argv) {
  char *programFileName, *lineptr = NULL;
  size_t n;
  FILE *configFile = fopen("config.sys", "r");
  if (!configFile) {
    puts("unable to read from file config.sys");
    return 1;
  }
  getline(&lineptr, &n, configFile);
  boot_system(atoi(lineptr));
  fclose(configFile);
  free(lineptr);
  lineptr = NULL;
  printf("Input Program File and BASE> ");
  getline(&lineptr, &n, stdin);
  programFileName = strtok(lineptr, " \n");
  BASE = atoi(strtok(NULL, " \n"));
  printf("\n");
  load_prog(programFileName, BASE);
  free(lineptr);
  process_init_PCB();
  process_set_registers();
  cpu_operation();

  free(Mem);

  return 0;
}
