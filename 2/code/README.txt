My program was compiled and ran on the cs3 server. My program is compiled using the gcc compiler to the gnu99 standard. My project compiles to an executable called 'computer'. It can be compiled using the command 'make all' and run using './computer'.

Commands to compile my project:
	gcc -std=gnu99 -c sauce/computer.c -o build/computer.o
	gcc -std=gnu99 -c sauce/cpu.c -o build/cpu.o
	gcc -std=gnu99 -c sauce/darray.c -o build/darray.o
	gcc -std=gnu99 -c sauce/load.c -o build/load.o
	gcc -std=gnu99 -c sauce/memory.c -o build/memory.o
	gcc -std=gnu99 -c sauce/print.c -o build/print.o
	gcc -std=gnu99 -c sauce/printer.c -o build/printer.o
	gcc -std=gnu99 -c sauce/queue.c -o build/queue.o
	gcc -std=gnu99 -c sauce/scheduler.c -o build/scheduler.o
	gcc -std=gnu99 -c sauce/shell.c -o build/shell.o
	gcc -o computer build/computer.o build/cpu.o build/darray.o build/load.o build/memory.o build/print.o build/printer.o build/queue.o build/scheduler.o build/shell.o -lm -lpthread

The program reads from the config.sys file and is expecting all M, TQ, and PT to be in that order all on 1 line separated by a space.
  Example:
  64 20 1000
The PT parameter should be in micro seconds.

The printer process creates a directory called '.spool' in the current working directory. If there is already a directory or file in the current path called .spool, the process will try to delete or empty it. If this does not work, the .spool file will need to be manually deleted.

The printer process will overwrite the printer.out file each time the program is run.

The maximum length for a process name is 256 characters. If the name is longer, it will be truncated.

My prog-idle is 4 words and takes up addresses 0 - 3, so the first address available for user programs is 4.

The program looks for prog-idle in the path './prog/IDLE'. If this file is not found, a hard coded version of prog-idle will be loaded.

My program can use the redirection using the '<' character. Example ./computer < comp.in. The input file should have 1 command per line. The file should end with 0, the shell exit command. If the file does not end with '0', it will cause getline to fail. If the shell's getline function fails, the shell sets the terminate flag to true and the computer terminate.
  Example of input file:
  1
  decrement 4
  2
  6
  1
  factorial 40
  0
