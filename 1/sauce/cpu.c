#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "computer.h"

int PC, MBR, MAR, AC, IR0, IR1, BASE;

void cpu_fetch_instruction() {
  MAR = cpu_mem_address(PC++); // fetch opcode
  mem_read();
  IR0 = MBR;
  MAR = cpu_mem_address(PC++); // fetch argument
  mem_read();
  IR1 = MBR;
}

void cpu_execute_instruction() {
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
    printf("%d\n", AC);
    break;
  // Operand - Time
  // Actions Sleep for the given "time", which is the operand
  case 8:
    usleep(IR1);
    break;
  // Operand - Code
  // Actions - Execute the shell command according to code
  case 9:
    shell_command(IR1);
    break;
  // case 0
  // Actions - End of current program, null is 0 and is unused
  default:
    free(Mem);
    exit(0);
  }
}

int cpu_mem_address(int m_addr) { return m_addr + BASE; }

void cpu_operation() {
  while (1) {
    cpu_fetch_instruction();
    cpu_execute_instruction();
  }
}
