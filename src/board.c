/*
* board.c — Board engine: grid, snakes, ladders, O(1) lookup
* Depends: board.h, utils.h
*/
#include "board.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/* ■■ Default board layout ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
* Standard Snake & Ladder positions (classic Milton Bradley layout).
* You can override these via board_config.txt in Phase 7.
*/
static const int DEFAULT_SNAKE_HEADS[] = {
99, 94, 87, 62, 54, 17
};
static const int DEFAULT_SNAKE_TAILS[] = {
78, 56, 24, 18, 9, 1 /* tail < head always */
};
static const int DEFAULT_LADDER_BOTTOMS[] = {
4, 9, 20, 28, 40, 51, 63, 71
};
static const int DEFAULT_LADDER_TOPS[] = {
25, 31, 38, 84, 59, 67, 81, 91 /* top > bottom always */
};
#define NUM_DEFAULT_SNAKES 6
#define NUM_DEFAULT_LADDERS 8
/* ■■ Internal helpers ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
/*
* board_reset_lookup():
* Initialize cell_type[] and destination[] to safe defaults.
* MUST be called before board_rebuild_lookup() or any add operations.
*/
static void board_reset_lookup(Board *b) {
for (int i = 0; i <= BOARD_SIZE; i++) {
b->cell_type[i] = CELL_NORMAL;
b->destination[i] = i; /* Normal cell: you stay where you land */
}
b->cell_type[0] = CELL_START;
b->cell_type[100] = CELL_END;
}
/* ■■ Public API ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
void board_rebuild_lookup(Board *b) {
/*
* Rebuild cell_type[] and destination[] from the snakes[]
* and ladders[] arrays.
* Call this after: init, add_snake, add_ladder, load_config.
*/
board_reset_lookup(b);
for (int i = 0; i < b->num_snakes; i++) {
int head = b->snakes[i].head;
int tail = b->snakes[i].tail;
b->cell_type[head] = CELL_SNAKE;
b->destination[head] = tail; /* Landing on head → go to tail */
}
for (int i = 0; i < b->num_ladders; i++) {
int bot = b->ladders[i].bottom;
int top = b->ladders[i].top;
b->cell_type[bot] = CELL_LADDER;
b->destination[bot] = top; /* Landing on bottom → climb to top */
}
}
void board_init_defaults(Board *b) {
/* Zero out the entire struct first — important! */
memset(b, 0, sizeof(Board));
b->num_snakes = 0;
b->num_ladders = 0;
/* Add default snakes */
for (int i = 0; i < NUM_DEFAULT_SNAKES; i++) {
board_add_snake(b, DEFAULT_SNAKE_HEADS[i],
DEFAULT_SNAKE_TAILS[i]);
}
/* Add default ladders */
for (int i = 0; i < NUM_DEFAULT_LADDERS; i++) {
board_add_ladder(b, DEFAULT_LADDER_BOTTOMS[i],
DEFAULT_LADDER_TOPS[i]);
}
/* Build the lookup table from the arrays we just filled */
board_rebuild_lookup(b);
}
bool board_add_snake(Board *b, int head, int tail) {
/*
* Validation: snakes must satisfy head > tail.
* Head and tail must be on the board (1..99).
* Head cannot be 100 (win cell — would be unwinnable).
*/
if (head <= tail) { fprintf(stderr, "Snake: head must be > tail\n"); return false; }
if (head < 2 || head > 99) { fprintf(stderr, "Snake head out of range\n"); return false; }
if (tail < 1 || tail > 98) { fprintf(stderr, "Snake tail out of range\n"); return false; }
if (b->num_snakes >= MAX_SNAKES) {
fprintf(stderr, "Max snakes reached\n"); return false;
}
b->snakes[b->num_snakes].head = head;
b->snakes[b->num_snakes].tail = tail;
b->num_snakes++;
return true;
}
bool board_add_ladder(Board *b, int bottom, int top) {
if (top <= bottom) { fprintf(stderr, "Ladder: top must be > bottom\n"); return false; }
if (bottom < 1 || bottom > 98) { fprintf(stderr, "Ladder bottom out of range\n"); return false; }
if (top < 2 || top > 100) { fprintf(stderr, "Ladder top out of range\n"); return false; }
if (b->num_ladders >= MAX_LADDERS) {
fprintf(stderr, "Max ladders reached\n"); return false;
}
b->ladders[b->num_ladders].bottom = bottom;
b->ladders[b->num_ladders].top = top;
b->num_ladders++;
return true;
}
int board_resolve(const Board *b, int pos) {
/*
* board_resolve(): given that a player just moved to 'pos',
* return where they actually end up.
* This uses the O(1) destination[] array.
*
* EDGE CASE: chaining. In some rule variants, if a ladder
* takes you to another ladder's bottom, you climb again.
* We implement that here with a while loop.
* Standard rules do NOT chain; we chain for fun.
* Change MAX_CHAIN to 1 to disable chaining.
*/
int current = pos;
int max_chain = 3; /* Prevent infinite loops */
for (int i = 0; i < max_chain; i++) {
int dest = b->destination[current];
if (dest == current) break; /* Normal cell — stop */
current = dest;
}
return current;
}
CellType board_cell_type(const Board *b, int pos) {
if (pos < 0 || pos > BOARD_SIZE) return CELL_NORMAL;
return b->cell_type[pos];
}
bool board_validate(const Board *b) {
/*
* Validate that no cell is both a snake head and a ladder bottom.
* Such a configuration would cause undefined game behavior.
*/
for (int i = 0; i < b->num_snakes; i++) {
int head = b->snakes[i].head;
for (int j = 0; j < b->num_ladders; j++) {
if (b->ladders[j].bottom == head) {
fprintf(stderr, "Conflict: cell %d is both a snake head"
" and a ladder bottom\n", head);
return false;
}
}
}
return true;
}
void board_print_legend(const Board *b) {
printf("\n Snakes (head → tail):\n");
for (int i = 0; i < b->num_snakes; i++)
printf(" %2d → %2d\n", b->snakes[i].head, b->snakes[i].tail);
printf("\n Ladders (bottom → top):\n");
for (int i = 0; i < b->num_ladders; i++)
printf(" %2d → %2d\n", b->ladders[i].bottom, b->ladders[i].top);
}