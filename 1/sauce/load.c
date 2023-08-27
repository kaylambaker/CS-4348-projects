#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "computer.h"

void load_prog(char *fname, int base) {
  char buf[22];
  int instructionNo, dataNo;
  FILE *file = fopen(fname, "r");
  if (!file) {
    printf("could not open file %s\n", fname);
    free(Mem);
    exit(1);
  }

  // read 22 bytes cause max int is 10 digits, 2 of them, plus space pluse
  // newline

  // first line is number of instruction lines and number of memory lines
  fgets(buf, 22, file);
  instructionNo = atoi(strtok(buf, " \n"));
  dataNo = atoi(strtok(NULL, " \n"));

  MAR = base;
  for (int i = 0; i < instructionNo; i++) {
    fgets(buf, 22, file);
    MBR = atoi(strtok(buf, " \n"));
    mem_write();
    MAR++;
    MBR = atoi(strtok(NULL, " \n"));
    mem_write();
    MAR++;
  }

  for (int i = 0; i < dataNo; i++) {
    fgets(buf, 22, file);
    MBR = atoi(strtok(buf, " \n"));
    mem_write();
    MAR++;
  }
  load_finish(file);
}
void load_finish(FILE *f) { fclose(f); }
