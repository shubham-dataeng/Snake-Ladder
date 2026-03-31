# ════════════════════════════════════════════════════════════════════════════════
# Snake & Ladder — Professional Makefile
# ════════════════════════════════════════════════════════════════════════════════
# Project: High-performance console-based Snake & Ladder game in C
# Build System: GNU Make with modular architecture
# License: MIT
# ════════════════════════════════════════════════════════════════════════════════

# ─── Compiler Configuration ─────────────────────────────────────────────────────
CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -g -Iinclude
LDFLAGS := -lm
VERSION := 1.0

# ─── Source Files Organization ──────────────────────────────────────────────────
# Shared sources used by all targets (local game, server, client)
SHARED_SRCS := \
	src/utils.c \
	src/board.c \
	src/player.c \
	src/dice.c \
	src/config.c \
	src/ai.c \
	src/analytics.c \
	src/replay.c \
	src/game.c \
	src/ui.c \
	src/save.c \
	src/network.c

# ─── Game Target (Local Play) ───────────────────────────────────────────────────
GAME_SRCS := $(SHARED_SRCS) src/main.c
GAME_OBJS := $(GAME_SRCS:.c=.o)
GAME_TARGET := snake-ladder

# ─── Server Target (Network Multiplayer) ────────────────────────────────────────
SERVER_SRCS := $(SHARED_SRCS) src/server.c
SERVER_OBJS := $(SERVER_SRCS:.c=.o)
SERVER_TARGET := snake-ladder-server

# ─── Client Target (Network Client) ─────────────────────────────────────────────
CLIENT_SRCS := $(SHARED_SRCS) src/client.c
CLIENT_OBJS := $(CLIENT_SRCS:.c=.o)
CLIENT_TARGET := snake-ladder-client

# ─── Test Target ────────────────────────────────────────────────────────────────
TEST_TARGET := test-suite
TEST_SRCS := tests/test_suite.c
TEST_OBJS := $(TEST_SRCS:.c=.o)
# Exclude main.c from test build (test suite has its own main)
TEST_GAME_OBJS := $(filter-out src/main.o, $(GAME_OBJS))

# ════════════════════════════════════════════════════════════════════════════════
# BUILD TARGETS
# ════════════════════════════════════════════════════════════════════════════════

# ─── Default: Build all three binaries ──────────────────────────────────────────
all: $(GAME_TARGET) $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "✅ Build complete: $(GAME_TARGET), $(SERVER_TARGET), $(CLIENT_TARGET)"


# ─── Compilation Rules ──────────────────────────────────────────────────────────
# Game binary
$(GAME_TARGET): $(GAME_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Binary size: $$(du -h $(GAME_TARGET) | cut -f1)"

# Server binary
$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Binary size: $$(du -h $(SERVER_TARGET) | cut -f1)"

# Client binary
$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Binary size: $$(du -h $(CLIENT_TARGET) | cut -f1)"

# Object files from source
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Test suite
$(TEST_TARGET): $(TEST_OBJS) $(TEST_GAME_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ════════════════════════════════════════════════════════════════════════════════
# EXECUTION TARGETS
# ════════════════════════════════════════════════════════════════════════════════

run: $(GAME_TARGET)
	./$(GAME_TARGET)

run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET) localhost 5000

# ════════════════════════════════════════════════════════════════════════════════
# DEBUGGING TARGETS
# ════════════════════════════════════════════════════════════════════════════════

debug: $(GAME_TARGET)
	gdb ./$(GAME_TARGET)

debug-server: $(SERVER_TARGET)
	gdb ./$(SERVER_TARGET)

debug-client: $(CLIENT_TARGET)
	gdb ./$(CLIENT_TARGET)

debug-with-gdb: $(GAME_TARGET)
	gdb -x docs/gdb-commands.gdb --args ./$(GAME_TARGET)

# ════════════════════════════════════════════════════════════════════════════════
# MEMORY & QUALITY CHECKS
# ════════════════════════════════════════════════════════════════════════════════

# Valgrind memory leak detection (full output)
memcheck: $(GAME_TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(GAME_TARGET)

# Valgrind with custom suppressions (cleaner output)
memcheck-clean: $(GAME_TARGET)
	valgrind --suppressions=docs/valgrind-suppress.txt --leak-check=full ./$(GAME_TARGET)

# Valgrind for server binary
memcheck-server: $(SERVER_TARGET)
	valgrind --suppressions=docs/valgrind-suppress.txt --leak-check=full ./$(SERVER_TARGET)

# AddressSanitizer (runtime memory safety check)
sanitize: clean
	CFLAGS="$(CFLAGS) -fsanitize=address -fsanitize=undefined" make $(GAME_TARGET)
	./$(GAME_TARGET)
# ════════════════════════════════════════════════════════════════════════════════
# TESTING & ANALYSIS
# ════════════════════════════════════════════════════════════════════════════════

# Run test suite
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# All testing targets combined
all-tests: test memcheck-clean
	@echo "✅ All tests passed successfully"

# Static analysis with clang (if available)
static-check:
	@command -v clang >/dev/null 2>&1 && \
	clang --analyze src/*.c -Iinclude 2>&1 | grep -E "^src/" || \
	echo "ℹ️  clang not available; skipping static analysis"

# ════════════════════════════════════════════════════════════════════════════════
# CODE QUALITY & FORMATTING
# ════════════════════════════════════════════════════════════════════════════════

# Auto-format all source code with clang-format
format:
	@echo "🔧 Formatting C source files..."
	find src include tests \( -name "*.c" -o -name "*.h" \) | xargs clang-format -i
	@echo "✅ Formatting complete"

# Check code formatting without modifying files
check-format:
	@clang-format --output-replacements-xml src/*.c include/*.h | grep -q "<replacement " && \
	echo "⚠️  Code formatting issues found. Run 'make format' to fix." || \
	echo "✅ Code formatting looks good"

# ════════════════════════════════════════════════════════════════════════════════
# INFORMATION & DIAGNOSTICS
# ════════════════════════════════════════════════════════════════════════════════

# Display binary sizes
size: all
	@echo "📊 Binary Sizes:"
	@du -h $(GAME_TARGET) $(SERVER_TARGET) $(CLIENT_TARGET) | awk '{print "  " $$2 ": " $$1}'
	@echo ""
	@echo "📝 Total LOC (Lines of Code):"
	@echo "  Source: $$(wc -l src/*.c | tail -1 | awk '{print $$1}') lines"
	@echo "  Headers: $$(wc -l include/*.h | tail -1 | awk '{print $$1}') lines"
	@echo "  Tests: $$(wc -l tests/*.c 2>/dev/null | tail -1 | awk '{print $$1}') lines"

# Show help message
help:
	@echo "════════════════════════════════════════════════════════════════════════════════"
	@echo "Snake & Ladder — Build System Help (v$(VERSION))"
	@echo "════════════════════════════════════════════════════════════════════════════════"
	@echo ""
	@echo "BUILD TARGETS:"
	@echo "  make                    Build all three binaries (default)"
	@echo "  make clean              Remove all compiled objects and binaries"
	@echo ""
	@echo "EXECUTION:"
	@echo "  make run                Run local game"
	@echo "  make run-server         Run network server (port 5000)"
	@echo "  make run-client         Run network client (connects to localhost:5000)"
	@echo ""
	@echo "DEBUGGING:"
	@echo "  make debug              Launch GDB with game binary"
	@echo "  make debug-server       Launch GDB with server binary"
	@echo "  make debug-client       Launch GDB with client binary"
	@echo "  make debug-with-gdb     Use custom GDB commands (docs/gdb-commands.gdb)"
	@echo ""
	@echo "TESTING & QUALITY:"
	@echo "  make test               Run unit test suite"
	@echo "  make memcheck           Run Valgrind (full output)"
	@echo "  make memcheck-clean     Run Valgrind (with suppressions)"
	@echo "  make memcheck-server    Memory check server binary"
	@echo "  make sanitize           Run AddressSanitizer"
	@echo "  make all-tests          Run all tests and checks"
	@echo "  make static-check       Run clang static analysis"
	@echo ""
	@echo "CODE QUALITY:"
	@echo "  make format             Auto-format all source files"
	@echo "  make check-format       Check formatting without modifying"
	@echo ""
	@echo "DIAGNOSTICS:"
	@echo "  make size               Show binary sizes and LOC statistics"
	@echo "  make help               Display this help message"
	@echo ""
	@echo "════════════════════════════════════════════════════════════════════════════════"

# ════════════════════════════════════════════════════════════════════════════════
# CLEANUP
# ════════════════════════════════════════════════════════════════════════════════

clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(GAME_OBJS) $(SERVER_OBJS) $(CLIENT_OBJS)
	rm -f $(TEST_OBJS) $(TEST_GAME_OBJS)
	rm -f $(GAME_TARGET) $(SERVER_TARGET) $(CLIENT_TARGET) $(TEST_TARGET)
	@echo "✅ Clean complete"

# Clean everything including test binaries
distclean: clean
	@echo "🔄 Full distclean: removing all generated files"
	find . -name ".DS_Store" -delete
	find . -name "*.o" -delete
	find . -name "*~" -delete

# ════════════════════════════════════════════════════════════════════════════════
# PHONY TARGETS (Not actual files)
# ════════════════════════════════════════════════════════════════════════════════

.PHONY: all clean distclean run run-server run-client \
        debug debug-server debug-client debug-with-gdb \
        test memcheck memcheck-clean memcheck-server sanitize all-tests \
        format check-format static-check \
        size help
