My program compiles to an executable file called 'computer'. My program can be compiled and executed using the execute script. I compiled and tested my program on the cs3.udallas.edu server using the following commands.

My program can be compiled with make using the command 'make all'

Commands to compile source to object files
	gcc -c computer.c -std=gnu99
	gcc -c cpu.c -std=gnu99
	gcc -c load.c -std=gnu99
	gcc -c memory.c -std=gnu99
	gcc -c shell.c -std=gnu99
Command to compile executable file 'computer'
	gcc -o computer computer.o cpu.o memory.o load.o shell.o -std=gnu99
My file is executed using './computer'

computer.c
  computer.c reads from the config.sys file, which only contains 1 integer, and initializes memory according to the integer specified in config.sys. computer.c computer.c prompts the user to enter a program file and a base address. Then, it calls the load_prog function to load the program file into memory. The functions process_init_PCB and process_set_registers are called. process_init_PCB initializes the PCB variable RUNNING_PROGRAM and the process_set_registers sets the computer registers according to the RUNNING_PROGRAM PCB that was just initialized Then, the cpu_operation function is called.

load.c
  load.c parses the user program file and loads the file into memory.

shell.c
  shell.c provides a function to print the contents of the registers and a function to print the contents of the memory.

cpu.c
  cpu.c has the functions cpu_fetch_instruction, cpu_execute_instruction, cpu_mem_address, and cpu_operation. cpu_fetch_instruction reads 2 words from memory, places the first word into IR0 and the second word into IR1, and increments the PC by 2. cpu_execute_instruction uses the values of IR0 and IR1 to execute the correct actions according to the instruction. cpu_mem_address calculates the absolute address given an address relative to the BASE address/register value. cpu_operation calls the cpu_fetch_instruction and cpu_execute_instruction functions until the exit instruction is executed.
