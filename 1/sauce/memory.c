#include <stdlib.h>

#include "computer.h"

int *Mem;
int Memsize;

void mem_init(int k) {
  Mem = calloc(k, sizeof(int));
  Memsize = k;
}
void mem_read() { MBR = Mem[MAR]; }
void mem_write() { Mem[MAR] = MBR; }
