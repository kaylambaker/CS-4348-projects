#include <stdint.h>
#include <stdlib.h>

#include "../headers/computer.h"

static int *Mem;
static uint size;

void mem_init(uint M) {
  Mem = calloc(M, sizeof(int));
  size = M;
}
void mem_read(void) { MBR = Mem[MAR]; }
void mem_write(void) { Mem[MAR] = MBR; }
void mem_destroy(void) { free(Mem); }
uint mem_getsize(void) { return size; }
