CC := gcc

CFLAGS := -Wall -Wextra -g -Iinclude
LDFLAGS := -lm

# Shared sources
SHARED_SRCS := src/utils.c src/board.c src/player.c \
src/dice.c src/config.c src/ai.c src/analytics.c src/replay.c src/game.c src/ui.c src/save.c src/network.c

# Game targets
GAME_SRCS := $(SHARED_SRCS) src/main.c
GAME_OBJS := $(GAME_SRCS:.c=.o)
GAME_TARGET := snake-ladder

# Server target
SERVER_SRCS := $(SHARED_SRCS) src/server.c
SERVER_OBJS := $(SERVER_SRCS:.c=.o)
SERVER_TARGET := snake-ladder-server

# Client target
CLIENT_SRCS := $(SHARED_SRCS) src/client.c
CLIENT_OBJS := $(CLIENT_SRCS:.c=.o)
CLIENT_TARGET := snake-ladder-client

# All targets
all: $(GAME_TARGET) $(SERVER_TARGET) $(CLIENT_TARGET)

$(GAME_TARGET): $(GAME_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(GAME_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS) $(GAME_TARGET) $(SERVER_TARGET) $(CLIENT_TARGET)

run: $(GAME_TARGET)
	./$(GAME_TARGET)

run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET) localhost 5000

debug: $(GAME_TARGET)
	gdb ./$(GAME_TARGET)

debug-server: $(SERVER_TARGET)
	gdb ./$(SERVER_TARGET)

debug-client: $(CLIENT_TARGET)
	gdb ./$(CLIENT_TARGET)

memcheck: $(GAME_TARGET)
	valgrind --leak-check=full --track-origins=yes ./$(GAME_TARGET)

format:
	find src include \( -name "*.c" -o -name "*.h" \) | xargs clang-format -i

.PHONY: all clean run run-server run-client debug debug-server debug-client memcheck format
