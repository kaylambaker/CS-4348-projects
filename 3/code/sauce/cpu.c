#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../headers/computer.h"

static int TQ;
int PC, AC, BASE, MAR, MBR, IR0, IR1;

void cpu_fetch_instruction() {
  MAR = cpu_mem_address(PC++); // fetch opcode
  mem_read();
  IR0 = MBR;
  MAR = cpu_mem_address(PC++); // fetch argument
  mem_read();
  IR1 = MBR;
}

bool cpu_execute_instruction() {
  char *buf;
  uint len;
  switch (IR0) {
  // Operand - constant
  // Actions - Load the constant to AC
  case 1:
    AC = IR1;
    break;
  // Operand - m-addr
  // Actions - load Mem[m-addr] into AC
  case 2:
    MAR = cpu_mem_address(IR1);
    mem_read();
    AC = MBR;
    break;
  // Operand - m-addr
  // Actions - load Mem[m-addr] into MBR, add MBR to AC
  case 3:
    MAR = cpu_mem_address(IR1);
    mem_read();
    AC += MBR;
    break;
  // Operand -  m-addr
  // Actions - Same as above (case 3), except that add becomes multiply
  case 4:
    MAR = cpu_mem_address(IR1);
    mem_read();
    AC *= MBR;
    break;
  // Operand - m-addr
  // Actions - Store AC to Mem[m-addr]
  case 5:
    MAR = cpu_mem_address(IR1);
    MBR = AC;
    mem_write();
    break;
  // Operand - m-addr
  // Actions - If AC > 0 then go to the address given in Mem[m-addr]
  // Otherwise, continue to the next instruction
  case 6:
    if (AC > 0) {
      /* MAR = cpu_mem_address(IR1);
      mem_read();
      PC = MBR; */
      PC = IR1;
    }
    break;
  // Operand - Null
  // Actions Print the value in AC
  case 7:
    len = snprintf(NULL, 0, "%d\n", AC);
    buf = malloc(len * sizeof(char) + 1);
    sprintf(buf, "%d\n", AC);
    printf("%s", buf);
    print_print(buf, CID, RUNNING_PCB->PID);
    free(buf);
    break;
  // Operand - Time
  // Actions Sleep for the given "time", which is the operand
  case 8:
    sem_post(&MUTEX);
    usleep(IR1);
    sem_wait(&MUTEX);
    break;
  // Operand - Code
  // Actions - Execute the shell command according to code
  case 9:
    shell_command(IR1, RUNNING_PCB->PID);
    break;
  // case 0
  // Actions - End of current program, null is 0 and is unused
  default:
    return true;
  }
  return false;
}

uint cpu_mem_address(uint m_addr) { return m_addr + BASE; }

void cpu_init(uint tq) {
  PC = AC = BASE = IR0 = IR1 = MAR = MBR = 0;
  TQ = tq;
}

CPUStatus cpu_operation() {
  bool exit = false;
  for (uint i = 0; i < TQ; i++) {
    cpu_fetch_instruction();
    exit = cpu_execute_instruction();
    if (exit)
      return EXIT;
  }
  return TQ_EXPIRATION;
}
