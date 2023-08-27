all: computer

debug: computer.o cpu.o memory.o load.o shell.o
	gcc -g -o debug computer.o cpu.o memory.o load.o shell.o -std=gnu99

computer: computer.o cpu.o memory.o load.o shell.o
	gcc -o computer computer.o cpu.o memory.o load.o shell.o -std=gnu99

computer.o: computer.c computer.h
	gcc -c computer.c -std=gnu99

cpu.o: cpu.c computer.h
	gcc -c cpu.c -std=gnu99

load.o: load.c computer.h
	gcc -c load.c -std=gnu99

memory.o: memory.c computer.h
	gcc -c memory.c -std=gnu99

shell.o: shell.c computer.h
	gcc -c shell.c -std=gnu99

clean: 
	rm *.o
