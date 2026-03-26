#ifndef BOARD_H
#define BOARD_H
#include <stdbool.h>
/* Total cells on the board */
#define BOARD_SIZE 100
/* Hard limit: board can have at most this many snakes or ladders */
#define MAX_SNAKES 20
#define MAX_LADDERS 20
/*
 * Cell type enumeration.
 * WHY an enum instead of magic numbers?
 * Comparing cell_type == CELL_SNAKE is readable.
 * Comparing cell_type == 2 is a bug waiting to happen.
 */
typedef enum {
    CELL_NORMAL = 0,
    CELL_SNAKE = 1,
    CELL_LADDER = 2,
    CELL_START = 3, /* Cell 0 — off the board (waiting) */
    CELL_END = 4,   /* Cell 100 — win condition */
} CellType;
/*
 * Snake struct: head → tail (always head > tail)
 * Landing on head sends you DOWN to tail.
 */
typedef struct {
    int head; /* The cell you land on (e.g. 99) */
    int tail; /* Where you slide to (e.g. 54) */
} Snake;
/*
 * Ladder struct: bottom → top (always bottom < top)
 * Landing on bottom sends you UP to top.
 */
typedef struct {
    int bottom; /* The cell you land on (e.g. 4) */
    int top;    /* Where you climb to (e.g. 56) */
} Ladder;
/*
 * Board: the complete game grid.
 *
 * WHY two separate arrays (snakes[] and ladders[]) instead of
 * one lookup table?
 * 1. We need to know the TYPE of each cell for rendering.
 * (Snakes are red, ladders are green in the UI)
 * 2. We iterate over snakes and ladders separately when
 * loading from config and when printing the board legend.
 *
 * WHY also keep cell_type[BOARD_SIZE+1]?
 * O(1) lookup. When a player lands on cell N, we check
 * cell_type[N] instantly without looping through all snakes.
 * The +1 is because cells are 1-indexed (1..100).
 */
typedef struct {
    Snake snakes[MAX_SNAKES];
    Ladder ladders[MAX_LADDERS];
    int num_snakes;
    int num_ladders;
    /* O(1) lookup table: cell_type[N] tells you what cell N is */
    CellType cell_type[BOARD_SIZE + 1];
    /*
     * destination[N]: if you land on N, you end up at destination[N].
     * For normal cells: destination[N] == N.
     * For snake head 99: destination[99] = 54.
     * For ladder bottom 4: destination[4] = 56.
     */
    int destination[BOARD_SIZE + 1];
} Board;
/* Function prototypes */
void board_init_defaults(Board *b);
void board_rebuild_lookup(Board *b);
bool board_add_snake(Board *b, int head, int tail);
bool board_add_ladder(Board *b, int bottom, int top);
int board_resolve(const Board *b, int pos);
CellType board_cell_type(const Board *b, int pos);
void board_print_legend(const Board *b);
bool board_validate(const Board *b);
#endif /* BOARD_H */