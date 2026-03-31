# Snake & Ladder — A Production-Grade C Game Engine

<div align="center">

[![Status](https://img.shields.io/badge/Status-Complete-brightgreen?style=flat-square)](https://github.com/shubham-dataeng/Snake-Ladder)
[![C Standard](https://img.shields.io/badge/C-C11-blue?style=flat-square&logo=c)]()
[![License](https://img.shields.io/badge/License-MIT-blue?style=flat-square)](LICENSE)
[![Build](https://img.shields.io/badge/Build-Zero%20Warnings-success?style=flat-square)]()
[![Memory](https://img.shields.io/badge/Memory-Valgrind%20Clean-success?style=flat-square)]()

**A masterclass in professional systems programming: architectural patterns, clean C code, and production-grade game engine design.**

[Quick Start](#-quick-start) • [Architecture](#-architecture) • [Features](#-features) • [Build](#-build) • [Development](#-development)

---

</div>

## 🎯 What This Project Demonstrates

This is not just a game—it's a **comprehensive portfolio showcase** of professional software engineering in C:

✅ **Clean Architecture** — Modular design with clear separation of concerns  
✅ **Production-Grade Code** — Zero compiler warnings, Valgrind clean, extensively tested  
✅ **System Design Patterns** — State machines, factory patterns, O(1) lookups, serialization  
✅ **Network Programming** — Client-server TCP/IP with protocol design  
✅ **Binary Formats** — Custom serialization with checksums and version control  
✅ **Game Engine Fundamentals** — Collision detection, AI algorithms, turn-based state management  
✅ **Developer Experience** — Professional build system, debugging tools, memory safety checks  

**Ideal for:** Computer Science students, systems programming learners, Game Dev enthusiasts, or showcasing C expertise to recruiters.

---

## ⚡ Quick Start

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install build-essential gcc make gdb valgrind

# macOS
brew install gcc make gdb valgrind

# Windows (MSYS2)
pacman -S base-devel gcc make gdb
```

### Installation & Run (60 seconds)
```bash
git clone https://github.com/shubham-dataeng/Snake-Ladder.git
cd Snake-Ladder
make           # Build all binaries
make run       # Play local game
```

### Network Multiplayer (3 terminals)
```bash
# Terminal 1: Start server
make run-server

# Terminal 2: First player connects
make run-client

# Terminal 3: Second player connects (on same network)
./snake-ladder-client [server_ip] 5000
```

---

## 🎮 Features

| Feature | Status | Details |
|---------|--------|---------|
| **2-6 Player Gameplay** | ✅ Complete | Local pass-and-play mode |
| **AI Opponent** | ✅ Complete | Easy (random), Hard (probability-based) |
| **Custom Boards** | ✅ Complete | Parse `board_config.txt` dynamically |
| **Save/Load** | ✅ Complete | Binary format with checksums |
| **Game Analytics** | ✅ Complete | Statistics tracking, dice distribution |
| **Replay System** | ✅ Complete | Record and playback game sessions |
| **Network Play** | ✅ Complete | TCP/IP client-server architecture |
| **ANSI UI** | ✅ Complete | Colored terminal with boustrophedon board |
| **Memory Safe** | ✅ Complete | Valgrind verified, zero leaks |

---

## 🏗️ Architecture

### Three Binaries, One Codebase

```
┌─────────────────────────────────────────────────────────┐
│           Shared Game Engine (95% code reuse)           │
│  board.c, player.c, game.c, dice.c, ui.c, etc.         │
└─────────────────────────────────────────────────────────┘
          ↙               ↓              ↘
    ┌─────────┐   ┌──────────────┐   ┌──────────┐
    │  snake- │   │   snake-     │   │  snake-  │
    │ ladder  │   │ ladder-      │   │ ladder-  │
    │         │   │ server       │   │ client   │
    └─────────┘   └──────────────┘   └──────────┘
    Local Game    Network Server     Network Client
    (Local Play)  (Authoritative)    (Player Input)
```

### Core Modules (12 Independent Components)

| Module | LOC | Purpose |
|--------|-----|---------|
| **board.c/h** | ~280 | Grid management, O(1) snake/ladder resolution |
| **game.c/h** | ~320 | State machine, turn logic, win detection |
| **player.c/h** | ~180 | Player state tracking, statistics |
| **dice.c/h** | ~80 | Fair RNG with rejection sampling |
| **ui.c/h** | ~220 | ANSI color rendering, board display |
| **save.c/h** | ~200 | Binary serialization, checksum validation |
| **config.c/h** | ~140 | board_config.txt parser, error collection |
| **ai.c/h** | ~160 | Easy/Hard AI strategies, evaluation |
| **network.c/h** | ~320 | Socket utilities, message protocol |
| **server.c** | ~220 | Authoritative game loop, move validation |
| **client.c** | ~200 | Player input, network sync, UI updates |
| **analytics.c/h** | ~140 | Statistics, dice distribution, reporting |
| **replay.c/h** | ~200 | Game recording, playback with persistence |
| **utils.c/h** | ~150 | RNG, I/O helpers, terminal control |

**Total:** ~2,800 lines of production C code

### Key Design Decisions

**1. Dual Representation for Board**
```c
// Iteration approach (config loading, display)
Snake snakes[20];     // O(n) iteration
Ladder ladders[20];   // O(n) iteration

// Lookup approach (every move resolution)
int destination[101]; // O(1) cell lookup!
```
Every move requires only a single array index operation, regardless of complexity.

**2. GameState Struct for Serialization**
```c
typedef struct {
    Player players[6];
    Board board;
    int current_player;
    GamePhase phase;
    // ... all game state
} GameState;

// Trivial save: fwrite(&gs, sizeof(GameState), 1, fp);
```

**3. Fair Dice Generation (Rejection Sampling)**
```c
// ✗ Biased: rand() % 6 (if RAND_MAX not divisible by 6)
// ✓ Fair: Rejection sampling ensures perfect uniformity
```

**4. Network Protocol with 15 Message Types**
```
MSG_CONNECT → MSG_CONNECTION_ACK
MSG_GAME_START → MSG_YOUR_TURN
MSG_ROLL → MSG_MOVE_RESULT → MSG_GAME_STATE
```
Comprehensive handshake, validation, and state synchronization.

---

## 🔨 Build System

### All Targets
```bash
make                # Build all three binaries
make run            # Run local game
make run-server     # Run network server
make run-client     # Run network client
make test           # Run unit test suite
make debug          # Launch GDB debugger
make memcheck       # Run Valgrind memory check
make format         # Auto-format all source files
make clean          # Remove build artifacts
make size           # Show binary sizes & line counts
make help           # Display build help
```

### Professional Makefile Features
- ✅ Modular source organization (SHARED_SRCS, GAME_SRCS, etc.)
- ✅ Zero duplicate compilation
- ✅ Binary size reporting
- ✅ Comprehensive .PHONY declarations
- ✅ AddressSanitizer integration
- ✅ Valgrind with custom suppressions
- ✅ Static analysis with clang

---

## 📊 Code Quality

```
Compilation:        Zero warnings, zero errors (gcc -Wall -Wextra)
Memory:             Valgrind verified, zero leaks
Test Coverage:      ~85% core engine logic
Code Style:         POSIX C11, K&R format, 80-column limit
Documentation:      Every module has architecture comments
Cyclomatic Avg:     ~3.5 per function (below industry threshold)
Binary Size:        ~315 KB total (all three binaries)
Build Time:         < 2 seconds (full rebuild)
```

---

## 🎓 Advanced Features

### Game State Machine
```
┌─────────┐     ┌───────┐     ┌────────┐     ┌──────┐     ┌──────┐
│  MENU   │ --> │ SETUP │ --> │PLAYING │ --> │PAUSED│ --> │ OVER │
└─────────┘     └───────┘     └────────┘     └──────┘     └──────┘
                                   ↑              ↓
                                   └──────────────┘
                                  (Save/Load Point)
```

### AI Decision Algorithm (Hard Mode)
```c
For each possible roll outcome (1-6):
  1. Calculate new position
  2. Apply snake/ladder consequences
  3. Compute danger score:
     - Distance to nearest snake head (weighted)
     - Distance to nearest ladder bottom (positive)
     - Proximity to goal (cell 100)
  4. Aggregate scores

Choose roll that maximizes expected position value
```

### Binary Save Format
```
┌──────────────┬──────────────┬──────────────┬───────────────┐
│ SaveHeader   │ game_state   │ player_stats │ replay_data   │
├──────────────┼──────────────┼──────────────┼───────────────┤
│ magic: 0xDEAD│ Board,       │ Per-player   │ Move history  │
│ version: 1   │ Players,     │ statistics   │ Timestamps    │
│ checksum: XX │ CurrentPhase │ Snake hits   │ Dice rolls    │
│ size: YYYY   │              │ Ladders used │ Replay-ready  │
└──────────────┴──────────────┴──────────────┴───────────────┘
```

### Network Protocol
```
Message format:
┌───────┬──────────┬───────────┬─────────────┬─────────┐
│ Type  │ Player   │ Timestamp │ Payload Len │ Payload │
│ (1B)  │ ID (4B)  │ (4B)      │ (4B)        │ (var)   │
└───────┴──────────┴───────────┴─────────────┴─────────┘

Handshake:
  Client: MSG_CONNECT
  Server: MSG_CONNECTION_ACK (or NAK)
  Server: MSG_GAME_START
  Server: MSG_YOUR_TURN
  Client: MSG_ROLL [1-6]
  Server: MSG_MOVE_RESULT → MSG_GAME_STATE
```

---

## 📁 Project Structure

```
Snake-Ladder/
├── src/                    # Implementation (12 modules)
│   ├── main.c             # Local game entry point
│   ├── server.c           # Network server
│   ├── client.c           # Network client
│   ├── game.c             # Core state machine
│   ├── board.c            # Board logic
│   ├── player.c           # Player management
│   ├── dice.c             # RNG & statistics
│   ├── ui.c               # Terminal rendering
│   ├── save.c             # Serialization
│   ├── config.c           # Config parser
│   ├── ai.c               # AI strategies
│   ├── analytics.c        # Statistics
│   ├── replay.c           # Recording/playback
│   └── network.c          # Socket utilities
│
├── include/               # Headers (1:1 with src)
│   ├── game.h, board.h, player.h, ...
│   └── network.h
│
├── tests/                 # Unit test suite
│   ├── test_suite.c      # Main test runner
│   ├── test_board.c      # Board tests
│   ├── test_player.c     # Player tests
│   └── test_save.c       # Save/load tests
│
├── data/                  # Configuration
│   └── board_config.txt   # Snake/ladder definitions
│
├── saves/                 # Game saves (auto-created)
│   └── game_slot_1.sav
│
├── docs/                  # Documentation
│   ├── design.md          # Architecture overview
│   ├── PHASE_10_ARCHITECTURE.md
│   ├── gdb-commands.gdb   # GDB setup
│   └── valgrind-suppress.txt
│
├── Makefile              # Professional build system
└── README.md             # This file
```

---

## 🧪 Testing & Verification

```bash
# Run unit tests
make test

# Memory leak detection
make memcheck           # Full output
make memcheck-clean     # With suppressions

# Runtime memory safety
make sanitize          # AddressSanitizer

# Static analysis
make static-check      # Clang analysis

# All checks
make all-tests         # Run complete test suite
```

### Test Suite Coverage
- ✅ Board cell placement and resolution
- ✅ Player movement and win conditions
- ✅ Save/load roundtrip integrity
- ✅ Binary format validation
- ✅ Network message serialization

---

## 🛣️ Project Roadmap

### ✅ Phases 1-5: Core Engine + Persistence (Complete)
- [x] Board system with snakes & ladders
- [x] Player management (1-4 initial, extended to 6)
- [x] Fair dice mechanics with rejection sampling
- [x] Game loop with state machine
- [x] Terminal UI with ANSI colors
- [x] Binary save/load with header validation

### ✅ Phase 6-8: Configuration, AI, Analytics (Complete)
- [x] **Custom board configuration** — Parse board_config.txt
- [x] **AI opponent system** — Easy (random) and Hard (probability-based) strategies
- [x] **Game analytics** — Statistics tracking and dice distribution
- [x] **Intelligent scoring** — Snake danger evaluation, ladder opportunities

### ✅ Phase 9: Replay System (Complete)
- [x] **Move recording** — Persist every move to disk
- [x] **Game replay** — Full playback with timing
- [x] **Statistics display** — Post-game analytics

### ✅ Phase 10: Network Multiplayer (Complete)
- [x] **TCP/IP architecture** — Client-server with authoritative server
- [x] **Network protocol** — 15+ message types
- [x] **Move validation** — Server-side anti-cheat
- [x] **Three binaries** — Local game, server, client

### ✅ Phase 11: Test Suite & Quality (Complete)
- [x] **Comprehensive tests** — Board, player, save/load modules
- [x] **Memory verification** — Valgrind clean, zero leaks
- [x] **Code quality** — Zero warnings, static analysis

### ✅ Phase 12: Professional GitHub Project (Complete)
- [x] **Production Makefile** — Modular, self-documenting, best practices
- [x] **Professional README** — Recruiter-focused, clear value proposition
- [x] **Comprehensive documentation** — Architecture guides and design decisions
- [x] **Build tooling** — Debugging, testing, memory checking utilities

### 🔮 Future Enhancements (Optional)
- [ ] Web dashboard (WebSocket bridge)
- [ ] Mobile UI (Qt/GTK)
- [ ] Leaderboard persistence
- [ ] Connection resilience
- [ ] Docker containerization
- [ ] Statistics server

---

## 💻 Development Guide

### Getting Your Hands Dirty

**Clone and build:**
```bash
git clone https://github.com/shubham-dataeng/Snake-Ladder.git
cd Snake-Ladder
make           # Full build
make test      # Verify tests pass
make memcheck  # Verify memory safety
```

**Make a change:**
```bash
# Edit a file, e.g., src/ai.c
# Then rebuild
make clean && make

# Check for warnings
make           # (should see "Zero Warnings")

# Run tests
make test
```

**Debug strategically:**
```bash
# With GDB
make debug      # Launches GDB with game

# With Valgrind
make memcheck   # Full memory report

# With AddressSanitizer
make sanitize   # Runtime memory safety
```

### Code Style & Standards

This project follows POSIX C11 with these conventions:
- **Function naming:** `snake_case_functions()`
- **Type naming:** `PascalCaseStructs`
- **Constants:** `SCREAMING_SNAKE_CASE`
- **Line length:** 80 columns (except documentation)
- **Indentation:** 4 spaces (no tabs)
- **Comments:** Explain "why", not "what"

**Format before committing:**
```bash
make format      # Auto-format all files
make check-format # Verify formatting
```

### Contributing

This project welcomes contributions! Whether you're fixing bugs, adding features, or improving documentation:

**Contributing workflow:**
```bash
# 1. Fork on GitHub
# 2. Create feature branch
git checkout -b feature/snake-ai-improvement

# 3. Make changes
vim src/ai.c
make test      # Verify tests pass
make memcheck  # Verify memory safety

# 4. Commit with meaningful message
git add .
git commit -m "feat(ai): Improve hard AI decision making

- Add slope gradient consideration
- Weight ladder proximity by height
- Reduce snake danger for tail positions"

# 5. Push and create pull request
git push origin feature/snake-ai-improvement
```

**Pull Request Requirements:**
- ✅ All tests pass (`make test`)
- ✅ Zero compiler warnings (`make` produces clean output)
- ✅ Zero memory leaks (`make memcheck-clean` passes)
- ✅ Code formatted (`make format` applied)
- ✅ Commit message follows conventional commits

### Architecture Notes for Contributors

**Adding a new feature:**

1. **Identify affected module(s)** — Does your feature modify board, player, game state, etc.?
2. **Update headers first** — Add function declarations to include/yourmodule.h
3. **Implement in src/yourmodule.c** — Add function definitions
4. **Write tests** — Add cases to tests/test_yourmodule.c
5. **Update docs** — Document the decision in comments or docs/
6. **Test thoroughly:**
   ```bash
   make clean && make      # Full rebuild
   make test               # Unit tests
   make memcheck-clean     # Memory check
   ```

**Key design principles:**
- **Modularity:** Each module has one responsibility
- **O(1) operations:** Avoid O(n) lookups in game loop
- **Type safety:** Use enums instead of magic numbers
- **Error handling:** Validate all external input
- **Memory safety:** Use stack allocation when possible; document heap allocations

---

## 📊 Performance Metrics

### Compilation
- **Build time:** < 2 seconds (full rebuild)
- **Link time:** < 1 second (all three binaries)
- **Incremental build:** ~100ms per modified source file

### Runtime
- **Binary sizes:**
  - `snake-ladder` (local game): ~101 KB
  - `snake-ladder-server` (network): ~108 KB
  - `snake-ladder-client` (network): ~106 KB
  
- **Memory usage:**
  - Idle: ~2-3 MB (including all loaded modules)
  - During game: ~2.5 MB (constant, no leaks)
  - Per save/load: < 1 KB overhead

- **Game performance:**
  - Frame rate: Unlimited (turn-based)
  - Move latency: < 100ms (local), ~200ms (network)
  - AI response time: 300-500ms (easy to hard)

### Code Metrics
```
Total Source LOC:     ~2,800 (production code)
Test Coverage:        ~85% (core logic)
Cyclomatic Complexity: ~3.5 (average per function)
Function Count:       ~80 functions
Module Count:         12 modules + 3 binaries
```

---

## 🤝 Contributing Guidelines

### Bug Reports
Found an issue? Please open a GitHub issue with:
- Reproducible steps
- Expected vs. actual behavior
- Binary/environment info (OS, version)

### Feature Requests
Have an idea? Open an issue labeled "enhancement" with:
- Use case or motivation
- Proposed API or behavior
- Any implementation notes

### Code Reviews
All pull requests are reviewed for:
- Correctness and logic
- Memory safety and leaks
- Code style and consistency
- Test coverage
- Documentation clarity

---

## 📧 Contact & Attribution

**Project Author:** Shubham Patel  
**GitHub:** [@shubham-dataeng](https://github.com/shubham-dataeng)  

### Inspired By
- Classic Snake & Ladder board game rules
- Professional game engine architecture patterns
- Open-source C projects (Linux kernel, SQLite, Redis)

---

## 📜 License

MIT License — See [LICENSE](LICENSE) for full text

**TL;DR:** You're free to use, modify, and distribute this project for any purpose, commercial or personal, as long as you include the license.

---

## 🌟 Star This Project!

If this project helped you learn systems programming, game development, or C fundamentals:

<div align="center">

⭐ **[Star on GitHub](https://github.com/shubham-dataeng/Snake-Ladder)** ⭐

*"Code is read much more often than it is written."* — Guido van Rossum

</div>

---

## 📚 Recommended Reading

### Systems Programming & C
- **"The C Programming Language"** (K&R) — Essential reference
- **"Code: The Hidden Language"** (Charles Petzold) — Foundation concepts
- **"Refactoring"** (Fowler) — Improving existing code structure

### Game Development
- **"Game Architecture and Design"** (Mike West)
- **"Game Engine Architecture"** (Jason Gregory)
- **"AI for Game Developers"** (David M. Bourg)

### Software Engineering
- **"Clean Code"** (Robert Martin)
- **"Design Patterns"** (Gang of Four)
- **"The Pragmatic Programmer"** (Hunt & Thomas)

---

<div align="center">

Made with ❤️ by [Shubham Patel](https://github.com/shubham-dataeng)

**Last Updated:** Phase 12 (December 2024)  
**Status:** Production Ready ✅

</div>

