CC=gcc
GTK_FLAGS=`pkg-config --cflags --libs gtk+-3.0`

CFLAGS= -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -std=c17 -O3 -fsanitize=address,undefined

TARGET=interpreter
SRCS=interpreter.c hash_table.c label_table.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(GTK_FLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean
