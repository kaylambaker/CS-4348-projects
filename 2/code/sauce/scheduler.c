#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../headers/computer.h"
#include "../headers/darray.h"
#include "../headers/printer.h"
#include "../headers/queue.h"

static DynamicArray *ProcessTable; // holds PCB
static Queue *ReadyQueue;          // points to PCBs in ProcessTable
static char *PROG_IDLE_PATH = "./prog/IDLE";
static PCB PROG_IDLE_PCB = {1, 0, 0, 0, 0, 0, 0, 0, "IDLE"};
static int PROCESS_COUNT = 2;
PCB *RUNNING_PCB = NULL;

void process_init_PCBs(void) { ProcessTable = darray_create(sizeof(PCB)); }

// returns pid
int process_init_PCB(char *fname, uint base) {
  PCB pcb = {PROCESS_COUNT++, 0, 0, base, 0, 0, 0, 0, {}};
  strcpy(pcb.fname, fname);
  darray_insert(ProcessTable, pcb.PID, &pcb);
  return pcb.PID;
}

void process_dispose_PCB(PCB *pcb) {
  // overwrite pcbs position in process table with nil
  // setting pcb to nil also sets terminated to false
  PCB nil = {0};
  darray_insert(ProcessTable, pcb->PID, &nil);
}

void process_init_readyQ(void) { ReadyQueue = queue_create(sizeof(PCB *)); }

void process_insert_readyQ(PCB *pcb) {
  if (pcb->PID > 1) // dont put PROG_IDLE in ready queue
    queue_enqueue(ReadyQueue, &pcb);
}

PCB *process_fetch_readyQ() {
  PCB *pcb;
  queue_dequeue(ReadyQueue, &pcb);
  return pcb;
}

void process_init() {
  process_init_readyQ();
  process_init_PCBs();
  if (load_prog(PROG_IDLE_PATH, 0)) {
    strcpy(PROG_IDLE_PCB.fname, PROG_IDLE_PATH);
  } else {
    puts("[LOADING BACKUP IDLE PROCESS]");
    load_idle();
  }
  darray_insert(ProcessTable, PROG_IDLE_PCB.PID, &PROG_IDLE_PCB);
}

// put job into ready queue and create
// call create and insert readyq
// called by shell
void process_submit(char *fname, uint base) {
  int pid = process_init_PCB(fname, base);
  print_init_spool(pid);
  process_insert_readyQ(darray_at(ProcessTable, pid));
}

void process_execute(void) {
  CPUStatus status;
  PCB *prev = RUNNING_PCB;
  while (!TERMINATE) {
    sem_wait(&MUTEX);
    if (queue_empty(ReadyQueue)) {
      RUNNING_PCB = &PROG_IDLE_PCB;
    } else {
      RUNNING_PCB = process_fetch_readyQ();
    }
    if (prev != RUNNING_PCB) { // instructions say print process switch
      if (prev && prev->PID)
        printf("[PROCESS SWITCH OUT PID %d]\n", prev->PID);
      printf("[PROCESS SWITCH IN PID %d]\n", RUNNING_PCB->PID);
    }
    process_context_switch(NULL, RUNNING_PCB);
    status = cpu_operation();
    if (status == EXIT) {
      printf("[PROCESS SWITCH OUT PID %d]\n",
             RUNNING_PCB->PID); // instructions say print process switch
      process_exit(RUNNING_PCB);
    } else { // status == TQ_EXPIRATION
      process_context_switch(RUNNING_PCB, NULL);
      process_insert_readyQ(RUNNING_PCB); // put back into ready queue
    }
    sem_post(&MUTEX);
    prev = RUNNING_PCB;
  }
}

void process_exit(PCB *pcb) {
  int pid = pcb->PID;
  process_dispose_PCB(pcb);
  print_end_spool(pid);
}

// save context of out
// load context of in
void process_context_switch(PCB *out, PCB *in) {
  if (out) {
    out->PC = PC;
    out->AC = AC;
    out->MBR = MBR;
    out->MAR = MAR;
    out->IR0 = IR0;
    out->IR1 = IR1;
  }
  if (in) {
    PC = in->PC;
    AC = in->AC;
    MBR = in->MBR;
    MAR = in->MAR;
    IR0 = in->IR0;
    IR1 = in->IR1;
    BASE = in->BASE;
  }
}

void process_dump_readyQ(uint pid) {
  Node *n = ReadyQueue->head;
  PCB *pcb;
  int index = 0, len;
  char *buf;
  const char *fmt = "%d: %d\n",
             *heading =
                 "=================\n   ReadyQ Dump\n=================\n";
  printf("%s", heading);
  if (pid)
    print_print(heading, pid);
  while (n) {
    pcb = *(PCB **)(n->data);
    len = snprintf(NULL, 0, fmt, index, pcb->PID);
    buf = malloc(len * sizeof(char) + 1);
    sprintf(buf, fmt, index, pcb->PID);
    printf("%s", buf);
    if (pid)
      print_print(buf, pid);
    n = n->next;
    index++;
    free(buf);
  }
}

void process_dump_PCB(uint pid) {
  PCB *pcb;
  int index = 0, len;
  char *buf;
  const char *fmt = "%d: [ Filename:%s, PID:%d, BASE:%d, PC:%d, IR0:%d, "
                    "IR1:%d, AC:%d, MAR:%d, MBR:%d ]\n",
             *heading =
                 "==================\n     PCB Dump\n==================\n"
                 "INDEX: [ Filename:XXXXX, PID:#, BASE,#, PC:#, IR0:#, IR1:#, "
                 "AC:#, MAR#, MBR:# ]\n";

  printf("%s", heading);
  if (pid)
    print_print(heading, pid);
  for (int i = 0; i < ProcessTable->size; i++) {
    pcb = darray_at(ProcessTable, i);
    if (pcb->PID != 0) { // if there is pcb in this position
      len = snprintf(NULL, 0, fmt, index, pcb->fname, pcb->PID, pcb->BASE,
                     pcb->PC, pcb->IR0, pcb->IR1, pcb->AC, pcb->MAR, pcb->MBR);
      buf = malloc(len * sizeof(char) + 1);
      sprintf(buf, fmt, index, pcb->fname, pcb->PID, pcb->BASE, pcb->PC,
              pcb->IR0, pcb->IR1, pcb->AC, pcb->MAR, pcb->MBR);
      printf("%s", buf);
      if (pid)
        print_print(buf, pid);
      free(buf);
      index++;
    }
  }
}

void process_destroy_readyQ(void) { queue_destroy(ReadyQueue); }
void process_destroy_PCBs(void) { darray_destroy(ProcessTable); }
