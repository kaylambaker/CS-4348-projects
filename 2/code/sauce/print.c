#include <fcntl.h>
#include <math.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../headers/computer.h"
#include "../headers/printer.h"

pid_t printerpid;
int PT;

// call print init before printer init
// idk what the hell ACK is
void print_init(uint pt) {
  PrintMsg msg;
  PT = pt;
  pipe(print);
  pipe(printer);
  if ((printerpid = fork()) == 0) { // printer code
    printer_main();
  } else { // print code
    read(printer[READ], &msg, sizeof(PrintMsg));
    if (msg.type == PRINT_ACK) { // recieve ACK?
      puts("[PRINTER PROCESS INITALIZED]");
    } else { // error
      puts(msg.buf);
      exit(1);
    }
  }
}
void print_init_spool(uint pid) {
  PrintMsg msg = {{}, pid, INIT_SPOOL};
  write(print[WRITE], &msg, sizeof(PrintMsg));
}
void print_end_spool(uint pid) {
  PrintMsg msg = {{}, pid, END_SPOOL};
  write(print[WRITE], &msg, sizeof(PrintMsg));
}
void print_print(const char *buf, uint pid) {
  PrintMsg msg = {{}, pid, PRINT_REQ};
  const char *p = buf;
  uint32_t chuncks = ceil((float)strlen(buf) / (float)CHUNCK_SIZE);
  for (uint32_t i = 0; i < chuncks; i++) {
    strncpy(msg.buf, p, CHUNCK_SIZE);
    write(print[WRITE], &msg, sizeof(PrintMsg));
    p += CHUNCK_SIZE;
  }
}
void print_terminate(void) {
  PrintMsg msg = {{}, 0, PRINT_TERM};
  write(print[WRITE], &msg, sizeof(PrintMsg));
  waitpid(printerpid, NULL, 0);
}
