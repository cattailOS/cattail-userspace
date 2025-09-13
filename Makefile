CC = gcc
CFLAGS = -lm -lpthread -g
SANFLAGS = -g -fsanitize=address -fsanitize=undefined -O1 -Wall -Wextra
SRC = $(wildcard *.c)
OUT = novaos

all: clean build

build:
	$(CC) $(SRC) $(CFLAGS) -o $(OUT)

sanitize: clean
	$(CC) $(SRC) $(CFLAGS) $(SANFLAGS) -o $(OUT)

clean:
	rm -f 175*.c $(OUT)

run: all
	./$(OUT)

runw: all
	$(CMD) ./$(OUT)

run-sanitize: sanitize
	./$(OUT)

runw-sanitize: sanitize
	$(CMD) ./$(OUT)
