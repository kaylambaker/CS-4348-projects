#include <sys/types.h>

#ifndef PRINTER_H
#define PRINTER_H

#define WRITE 1
#define READ 0
#define CHUNCK_SIZE ((int)256)

typedef enum {
  PRINT_REQ,
  INIT_SPOOL,
  END_SPOOL,
  PRINT_TERM,
  PRINT_ACK
} MsgType;
// struct used to send message through pipe
typedef struct {
  char buf[CHUNCK_SIZE];
  int pid;
  MsgType type;
} PrintMsg;

extern int print[2], printer[2], PT;

extern void printer_init(void);
extern void printer_init_spool(int pid);
extern void printer_print(char *buf, uint pid);
extern void printer_end_spool(uint pid);
extern void printer_terminate(void);
extern void printer_main(void);
extern void printer_dump_spool(uint callPID);

#endif
