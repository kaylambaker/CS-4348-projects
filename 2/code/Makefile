CC=gcc
LIBS=-lm -lpthread

SAUCE=sauce
BUILD=build

SRC=$(wildcard $(SAUCE)/*.c)
OBJ=$(patsubst %,$(BUILD)/%, $(notdir $(SRC:.c=.o)))
TARGET=computer
CFLAGS=-Wall -std=gnu99

all: $(TARGET)

debug: $(OBJ)
	$(CC) -o -g  $@ $^ $(LIBS)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/%.o: $(SAUCE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(BUILD)/*.o
	rm $(TARGET)
