#ifndef COMPUTER_H
#define COMPUTER_H

#include <stdio.h>

extern int PC, MBR, MAR, AC, IR0, IR1, BASE;
extern int *Mem, Memsize;

// memory.c

// Ititalize memory, includeing allocationg the simulated memory Mem of M words
extern void mem_init(int M);
// copy Mem[MAR to MBR]
extern void mem_read();
// Copy MBR to Mem[MAR]
void mem_write();

// cpu.c

// Read the next 2 instruction words from memory, the addresses are computed
// from PC
extern void cpu_fetch_instruction();
// For each instruction code, perform the siumlated hardware operations
extern void cpu_execute_instruction();
// Compute the Memory address to be accessed and put it in MAR
extern int cpu_mem_address(int m_addr);
// Loop for executing instructions, starting from 0 till the exit instruction;
extern void cpu_operation();

// shell.c

// Initalize the shell module
extern void shell_init();
// Print out all the registers on the screen
extern void shell_print_registers();
// Print out all the words in memory in integer form on the screen
extern void shell_print_memory();
// For each shell command code, call the corresponding functions
// Input cmd is the command code
extern void shell_command(int cmd);

// load.c

// Open the user program file with the file name "fname"
// Copy the program into memory starting from memory address "base"
extern void load_prog(char *fname, int base);
// Clean up and close the program file
extern void load_finish(FILE *f);

// computer.c

// Create a PCB for the user program and initialize it
extern void process_init_PCB();
// Copy the PCB content data to the registers
extern void process_set_registers();

#endif
