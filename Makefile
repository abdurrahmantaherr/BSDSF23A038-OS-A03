CC = gcc
CFLAGS = -Iinclude -Wall -g
SRC = src/main.c src/shell.c src/execute.c src/signals.c

OBJ = $(SRC:.c=.o)
BIN = bin/myshell

all: $(BIN)

$(BIN): $(OBJ)
	mkdir -p bin
	$(CC) $(OBJ) -o $(BIN) -lreadline

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

