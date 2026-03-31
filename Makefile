CC := gcc

CFLAGS := -Wall -Wextra -g -Iinclude
LDFLAGS :=

SRCS := src/utils.c src/board.c src/player.c \
src/dice.c src/config.c src/game.c src/ui.c src/save.c src/main.c

OBJS := $(SRCS:.c=.o)

TARGET := snake-ladder

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)

debug: all
	gdb ./$(TARGET)

memcheck: all
	valgrind --leak-check=full --track-origins=yes ./$(TARGET)

format:
	find src include \( -name "*.c" -o -name "*.h" \) | xargs clang-format -i

.PHONY: all clean run debug memcheck format
