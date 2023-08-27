#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../headers/computer.h"

static const int PROG_IDLE[] = {
    1, 1, // load 1 to AC
    6, 2, // if AC more than 0, goto 2
};

bool load_prog(char *fname, uint base) {
  char *lineptr = NULL, *buf;
  size_t n = 0;
  uint instructionNo, dataNo;
  FILE *file = fopen(fname, "r");
  if (!file) {
    uint len = snprintf(NULL, 0, "[COULD NOT LOAD FILE %s]", fname);
    buf = malloc(len * sizeof(char) + 1);
    sprintf(buf, "[COULD NOT LOAD FILE %s]", fname);
    perror(buf);
    free(buf);
    return false;
  }

  // read 22 bytes cause max int is 10 digits, 2 of them, plus space pluse
  // newline

  // first line is number of instruction lines and number of memory lines
  if (getline(&lineptr, &n, file) == -1) {
    perror("load_prog getline");
    free(lineptr);
    return false;
  }
  instructionNo = atoi(strtok(lineptr, " \n"));
  dataNo = atoi(strtok(NULL, " \n"));
  free(lineptr);
  lineptr = NULL;

  MAR = base;
  for (uint i = 0; i < instructionNo; i++) {
    if (getline(&lineptr, &n, file) == -1) {
      perror("load_prog getline");
      free(lineptr);
      return false;
    }
    MBR = atoi(strtok(lineptr, " \n"));
    mem_write();
    MAR++;
    MBR = atoi(strtok(NULL, " \n"));
    mem_write();
    MAR++;
    free(lineptr);
    lineptr = NULL;
  }

  for (uint i = 0; i < dataNo; i++) {
    if (getline(&lineptr, &n, file) == -1) {
      perror("load_prog getline");
      free(lineptr);
      return false;
    }
    MBR = atoi(strtok(lineptr, " \n"));
    mem_write();
    MAR++;
    free(lineptr);
    lineptr = NULL;
  }
  fclose(file);
  return true;
}
void load_finish(FILE *f) { fclose(f); }

void load_idle() {
  uint8_t idleSize = sizeof(PROG_IDLE) / sizeof(PROG_IDLE[0]);
  for (MAR = 0; MAR < idleSize; MAR++) {
    MBR = PROG_IDLE[MAR];
    mem_write();
  }
}
