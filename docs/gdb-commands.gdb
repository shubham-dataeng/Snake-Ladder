#!/bin/bash
# gdb-commands.sh — Automated GDB debugging workflows for Phase 11
# Run with: gdb -x gdb-commands.sh --args ./snake-ladder

# ============================================================================
# WORKFLOW 1: Breakpoint on board_resolve (Bug #6 debugging)
# ============================================================================
define break-board-resolve
    break board_resolve
    commands 1
        silent
        printf "=== board_resolve called ===\n"
        printf "pos = %d\n", pos
        printf "BOARD_SIZE = 100\n"
        if pos < 0 || pos > 100
            printf "⚠️  WARNING: pos out of bounds!\n"
        end
        continue
    end
end

# ============================================================================
# WORKFLOW 2: Breakpoint on execute_move (Bug #1 debugging)
# ============================================================================
define break-execute-move
    break execute_move
    commands
        silent
        printf "=== execute_move called ===\n"
        printf "Player %d at position %d\n", pidx, gs->players[pidx].position
        printf "Roll: %d\n", roll
        continue
    end
end

# ============================================================================
# WORKFLOW 3: Breakpoint on player_init (Bug #2 debugging)
# ============================================================================
define break-player-init
    break player_init
    commands
        silent
        printf "=== player_init ===\n"
        printf "name = %s\n", name
        printf "MAX_NAME_LEN = %d\n", MAX_NAME_LEN
        printf "strlen(name) = %lu\n", strlen(name)
        continue
    end
end

# ============================================================================
# WORKFLOW 4: Breakpoint on fwrite (Bug #4 debugging)
# ============================================================================
define break-fwrite
    break save_game
    commands
        silent
        printf "=== save_game: checking fwrite semantics ===\n"
        printf "sizeof(GameState) = %lu\n", sizeof(GameState)
        continue
    end
end

# ============================================================================
# WORKFLOW 5: Detect stack corruption
# ============================================================================
define check-stack
    printf "=== Stack Frame Analysis ===\n"
    backtrace
    printf "\n=== Local Variables in Current Frame ===\n"
    info locals
end

# ============================================================================
# WORKFLOW 6: Memory watch in critical section
# ============================================================================
define watch-player-pos
    watch gs->players[pidx].position
    commands
        silent
        printf "Player %d position changed to %d\n", pidx, gs->players[pidx].position
        continue
    end
end

# ============================================================================
# WORKFLOW 7: Inspect board state
# ============================================================================
define show-board
    printf "=== Board State ===\n"
    printf "num_snakes: %d\n", b->num_snakes
    printf "num_ladders: %d\n", b->num_ladders
    printf "\nSnakes:\n"
    set $i = 0
    while $i < b->num_snakes
        printf "  [%d] %d -> %d\n", $i, b->snakes[$i].head, b->snakes[$i].tail
        set $i = $i + 1
    end
    printf "\nLadders:\n"
    set $j = 0
    while $j < b->num_ladders
        printf "  [%d] %d -> %d\n", $j, b->ladders[$j].bottom, b->ladders[$j].top
        set $j = $j + 1
    end
end

# ============================================================================
# WORKFLOW 8: Print game state snapshot
# ============================================================================
define show-game-state
    printf "=== Game State ===\n"
    printf "Phase: %d\n", gs->phase
    printf "Turn: %d\n", gs->turn_number
    printf "Players: %d\n", gs->num_players
    printf "Current player: %d\n", gs->current_player_idx
    set $p = 0
    while $p < gs->num_players
        printf "  Player %d: %s at position %d\n", $p, gs->players[$p].name, gs->players[$p].position
        set $p = $p + 1
    end
end

# ============================================================================
# INITIALIZATION: Set useful display
# ============================================================================
set print pretty on
set print array on
set print array-indexes on
set logging on
set logging file gdb-debug.log

printf "\n╔════════════════════════════════════════════════════════════╗\n"
printf "║    GDB DEBUGGING WORKFLOWS — Phase 11 Test Suite           ║\n"
printf "╚════════════════════════════════════════════════════════════╝\n"
printf "\nGDB Commands loaded. Available workflows:\n\n"
printf "  break-board-resolve  — Debug board_resolve (Bug #6)\n"
printf "  break-execute-move   — Debug execute_move (Bug #1)\n"
printf "  break-player-init    — Debug player_init (Bug #2)\n"
printf "  break-fwrite         — Debug fwrite (Bug #4)\n"
printf "  check-stack          — Analyze stack frame\n"
printf "  watch-player-pos     — Watch player position changes\n"
printf "  show-board           — Print board state\n"
printf "  show-game-state      — Print complete game state\n"
printf "\nUsage: (gdb) break-board-resolve\n"
printf "       (gdb) run\n"
printf "       (gdb) show-board\n\n"
