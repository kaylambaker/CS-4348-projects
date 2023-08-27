CC=gcc
LIBS=-lm -lpthread

SAUCE=sauce
BUILD=build

SRC=$(wildcard $(SAUCE)/*.c)
OBJ=$(patsubst %,$(BUILD)/%, $(notdir $(SRC:.c=.o)))
CFLAGS=-Wall -std=gnu99

all: computer printer

computer: build/queue.o build/darray.o build/memory.o build/load.o build/print.o build/scheduler.o build/shell.o build/computer.o build/cpu.o build/util.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

printer: build/printer.o build/queue.o build/util.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/%.o: $(SAUCE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(BUILD)/*.o
	rm computer
	rm printer
	rm printer.out
