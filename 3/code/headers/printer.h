#ifndef PRINTER_H
#define PRINTER_H

#include <sys/types.h>

#include "../headers/queue.h"

#define WRITE 1
#define READ 0
#define CHUNCK_SIZE ((int)4096)
#define SPOOL_DIR ("./.spool/")
#define SPOOL_NAME_FMT ("%s%d.%d") // "./.spool/CID.PID"
#define PAPER "printer.out"

extern uint MQS, CQS, NC, PRINTER_PORT, PT;
extern char *PRINTER_IP;

typedef enum {
  END_SPOOL,
  CPU_TERM,
} MsgType;

// struct used to send message
typedef struct {
  int cid, pid, fileSize;
  MsgType type;
} PrintMsg;

/* typedef struct {
  uint cid, pid, fileSize;
  char *buf;
  MsgType type;
} PrintQueueMsg; */

extern void printer_init(void);
extern void printer_init_spool(uint cid, uint pid);
extern void printer_print(const char *buf, uint cid, uint pid);
extern void printer_end_spool(PrintMsg *msg);
extern void printer_end_spool_computer(uint cid);
extern void printer_terminate(int signum);
extern void *printer_main(void *nil);
extern void printer_manager(void);
extern void printer_manager_init(void);

#endif
