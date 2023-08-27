#ifndef COMPUTER_H
#define COMPUTER_H

#include "printer.h"
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  int PID, PC, AC, BASE, MAR, MBR, IR0, IR1;
  char fname[256];
} PCB;

typedef enum { EXIT, TQ_EXPIRATION } CPUStatus;

// computer.c
extern bool TERMINATE;
extern sem_t MUTEX;
extern uint CID; // command line argument
extern char *PRINTER_IP;
extern void computer_poweroff(void);

// memory.c

// Ititalize memory, includeing allocationg the simulated memory Mem of M
// words
extern void mem_init(unsigned int M);
// copy Mem[MAR to MBR]
extern void mem_read(void);
// Copy MBR to Mem[MAR]
extern void mem_write(void);
// return memsize M
extern unsigned int mem_getsize(void);
// deallocate Mem array
extern void mem_destroy(void);

// cpu.c

extern int PC, AC, BASE, MAR, MBR, IR0, IR1;
// Read the next 2 instruction words from memory, the addresses are computed
// from PC
extern void cpu_fetch_instruction(void);
// For each instruction code, perform the siumlated hardware operations
extern bool cpu_execute_instruction(void);
// Compute the Memory address to be accessed and put it in MAR
extern uint cpu_mem_address(uint m_addr);
// Loop for executing instructions, starting from 0 till the exit instruction;
extern CPUStatus cpu_operation(void);
// initialize cpu with time quantum TQ
extern void cpu_init(uint TQ);

// shell.c

// Initalize the shell module
extern pthread_t shell_init(void);
// Print out all the registers on the screen
extern void shell_print_registers(uint pid);
// Print out all the words in memory in integer form on the screen
extern void shell_print_memory(uint pid);
// For each shell command code, call the corresponding functions
// Input cmd is the command code
extern void shell_command(uint8_t cmd, uint pid);
extern void *shell_thread(void *nil);

// load.c

// Open the user program file with the file name "fname"
// Copy the program into memory starting from memory address "base"
// returns size in words of program
extern bool load_prog(char *fname, uint base);
// Clean up and close the program file
extern void load_finish(FILE *f);
// load PROG_IDLE
extern void load_idle(void);

// schedule.c

extern PCB *RUNNING_PCB;
extern void process_init_readyQ(void);
extern void process_destroy_readyQ(void);
extern void process_destroy_PCBs(void);
// create pcb and put it in process table
// returns pid of process
extern int process_init_PCB(char *fname, uint base);
// returns pointer at start of ready queue
// removes PCB from ready queue
extern PCB *process_fetch_readyQ(void);
// initalize scheduler
extern void process_init(void);
// initalize process table
extern void process_init_PCBs(void);
// create process table entry for process and insert into ready queue
extern void process_submit(char *fname, uint base);
// insert pcb into ready queue
extern void process_insert_readyQ(PCB *pcb);
// execute scheduler functions to decide which process to run
// call cpu_execute to execute selected process
extern void process_execute();
// delete pcb entry in process table
extern void process_exit(PCB *pcb);
// save context of out, load context of in
extern void process_context_switch(PCB *out, PCB *in);
// output process pids in ready queue
extern void process_dump_readyQ(uint pid);
// output processes in process table
extern void process_dump_PCB(uint pid);

// print.c

// initialize print and printer
// create printer thread
extern void print_init(uint pt);
// send message to printer to create spool file for pid
extern void print_init_spool(uint cid, uint pid);
// send message to printer to output pids spool file to printer.out
extern void print_end_spool(uint cid, uint pid, MsgType type);
// send message to printer to write buf to pids spool file
extern void print_print(const char *buf, uint cid, uint pid);
// tell printer to output reamining spool files to printer.out
extern void print_terminate(void);
// print out all pids with open spool file
extern void printer_dump_spool(uint callPID);

#endif
