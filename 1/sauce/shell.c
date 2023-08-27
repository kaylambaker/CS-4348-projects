#include <stdlib.h>

#include "computer.h"

void shell_init() {}
void shell_print_registers() {
  printf(
      "===============\n Register Dump \n===============\nRegister: "
      "Contents\nBASE: %d\nPC: %d\nIR0: %d\nIR1 %d\nAC: %d\nMAR: %d\nMBR: %d\n",
      BASE, PC, IR0, IR1, AC, MAR, MBR);
}
void shell_print_memory() {
  printf("=====================\n Memory Dump k = %d "
         "\n=====================\nAddress: Contents\n",
         Memsize);
  for (MAR = 0; MAR < Memsize; MAR++) {
    mem_read();
    printf("%d: %d\n", MAR, MBR);
  }
}
void shell_command(int cmd) {
  switch (cmd) {
  case 2:
    shell_print_registers();
    break;
  case 3:
    shell_print_memory();
    break;
  default:
    puts("bruh, wut?");
    break;
  }
}
