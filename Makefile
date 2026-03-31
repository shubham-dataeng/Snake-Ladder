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

# ■■ PHASE 11: TEST SUITE ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
# Test targets for comprehensive bug fixing and quality assurance

TEST_TARGET := test-suite
TEST_SRCS := tests/test_suite.c
TEST_OBJS := $(TEST_SRCS:.c=.o)
# Exclude main.c from test build (test_suite has its own main)
TEST_GAME_OBJS := $(filter-out src/main.o, $(GAME_OBJS))

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(TEST_GAME_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# GDB debugging with custom commands
debug-with-gdb: debug
	gdb -x docs/gdb-commands.gdb --args ./$(GAME_TARGET)

# Valgrind memory check (light)
memcheck: $(GAME_TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(GAME_TARGET)

# Valgrind with suppressions (cleaner output)
memcheck-clean: $(GAME_TARGET)
	valgrind --suppressions=docs/valgrind-suppress.txt --leak-check=full ./$(GAME_TARGET)

# Valgrind for server binary
memcheck-server: $(SERVER_TARGET)
	valgrind --suppressions=docs/valgrind-suppress.txt --leak-check=full ./$(SERVER_TARGET)

# AddressSanitizer (runtime memory safety)
sanitize: clean
	CFLAGS="$(CFLAGS) -fsanitize=address -fsanitize=undefined" make $(GAME_TARGET)
	./$(GAME_TARGET)

# Static analysis with clang (if available)
static-check:
	@command -v clang >/dev/null 2>&1 && \
	clang --analyze src/*.c -Iinclude 2>&1 | grep -E "^src/" || \
	echo "clang not available; skipping static analysis"

# Full test suite
all-tests: test memcheck-clean
	@echo "✅ All tests completed successfully"

.PHONY: test debug-with-gdb memcheck memcheck-clean memcheck-server sanitize static-check all-tests

format:
	find src include \( -name "*.c" -o -name "*.h" \) | xargs clang-format -i

.PHONY: all clean run run-server run-client debug debug-server debug-client memcheck format
