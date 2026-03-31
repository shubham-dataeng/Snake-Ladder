/*
* ui.c — Terminal UI with ANSI colors
* Depends: ui.h, game.h, board.h, player.h
*/
#include "ui.h"
#include "board.h"
#include "player.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
/* ■■ ANSI Color Constants ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_BLUE "\033[44m"
#define BG_GRAY "\033[100m"
#define DIM "\033[2m"
/* ■■ Board Rendering ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
/*
* get_cell_char():
* Determine how to display a single cell.
* Priority: player token > snake > ladder > normal
* Multiple players on same cell: show count
*/
static void print_cell(const GameState *gs, int cell_num) {
const Board *b = &gs->board;
const Player *players = gs->players;
/* Check if any player is on this cell */
int player_on_cell = -1;
int players_on_cell = 0;
for (int i = 0; i < gs->num_players; i++) {
if (players[i].position == cell_num && !players[i].has_won) {
player_on_cell = i;
players_on_cell++;
}
}
CellType ct = board_cell_type(b, cell_num);
printf(" "); /* Left padding */
if (players_on_cell > 0) {
/* Show player token — use their color */
if (players_on_cell > 1) {
/* Multiple players: show count in yellow */
printf("%s%d%s", YELLOW, players_on_cell, RESET);
} else {
/* Single player: show first letter of name in their color */
printf("%s%c%s", players[player_on_cell].color_code,
players[player_on_cell].name[0], RESET);
}
} else if (ct == CELL_SNAKE) {
printf("%s●%s", RED, RESET); /* Red dot = snake head */
} else if (ct == CELL_LADDER) {
printf("%s▲%s", GREEN, RESET); /* Green triangle = ladder */
} else {
/* Normal cell — show number, dim */
if (cell_num == 100) {
printf("%s%d%s ", DIM, 100, RESET);
} else {
printf("%s%02d%s ", DIM, cell_num, RESET);
}
return; /* Already printed the number */
}
/* pad to width 2 */
printf(" ");
}
void ui_render_board(const GameState *gs) {
clear_screen();
/* Board header */
printf("\n %s■■■ SNAKE & LADDER ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■%s\n",
BOLD, RESET);
/*
* Board layout: 10×10 grid, cells 1-100.
* Row 10 (top): cells 100..91 (right to left — boustrophedon)
* Row 9: cells 81..90 (left to right)
* ...
* Row 1 (bottom): cells 1..10
*
* WHY boustrophedon (snake pattern)?
* This is how traditional Snake & Ladder boards are printed.
* Moving "forward" on the board always moves up the screen.
*/
for (int row = 9; row >= 0; row--) {
int row_start = row * 10 + 1;
printf(" ■");
for (int col = 0; col < 10; col++) {
int cell;
if (row % 2 == 0) {
cell = row_start + col; /* Left to right */
} else {
cell = row_start + (9 - col); /* Right to left */
}
/* Cell border */
printf("|");
print_cell(gs, cell);
}
printf("|■\n");
/* Row separator */
if (row > 0)
printf(" ■%s■\n", "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■");
}
printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■%s\n", RESET);
/* Legend */
printf(" %s●%s Snake head %s▲%s Ladder bottom ", RED, RESET, GREEN, RESET);
printf("Dimmed = normal cell\n");
}
void ui_print_status(const GameState *gs) {
printf("\n Turn #%d\n", gs->turn_number);
printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
for (int i = 0; i < gs->num_players; i++) {
const Player *p = &gs->players[i];
const char *indicator = (i == gs->current_player_idx) ? "■ " : " ";
if (p->has_won) {
printf(" %s%s%s%-16s%s ■ WON (rank %d)\n",
indicator, p->color_code, BOLD, p->name, RESET, p->rank);
} else {
printf(" %s%s%-16s%s Cell: %3d\n",
indicator, p->color_code, p->name, RESET, p->position);
}
}
printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
}
void ui_show_dice_roll(int roll, const char *player_name,
const char *color) {
/* ASCII dice face art */
const char *faces[7][3] = {
{" ", " ", " "}, /* 0: unused */
{" ", " ● ", " "}, /* 1 */
{" ● ", " ", " ● "}, /* 2 */
{" ● ", " ● ", " ● "}, /* 3 */
{" ● ● ", " ", " ● ● "}, /* 4 */
{" ● ● ", " ● ", " ● ● "}, /* 5 */
{" ● ● ", " ● ● ", " ● ● "}, /* 6 */
};
printf("\n %s%s%s rolled:\n%s", color, player_name, RESET, RESET);
printf(" ■■■■■■■■■\n");
for (int line = 0; line < 3; line++)
printf(" ■ %s ■\n", faces[roll][line]);
printf(" ■■■■■■■■■ = %s%s%d%s\n\n", BOLD, YELLOW, roll, RESET);
}
void ui_show_results(const GameState *gs) {
clear_screen();
printf("\n %s■■■■■■■ GAME OVER ■■■■■■■%s\n", BOLD, RESET);
for (int r = 0; r < gs->num_winners; r++) {
int idx = gs->winners[r];
const Player *p = &gs->players[idx];
const char *medal = (r == 0) ? "🥇" : (r == 1) ? "🥈" : "🥉";
printf(" ■ %s %s%s%s%s\n", medal,
p->color_code, BOLD, p->name, RESET);
}
printf(" ■■■■■■■■■■■■■■■■■■■■■■■■\n\n");
/* Print per-player stats */
for (int i = 0; i < gs->num_players; i++) {
player_print_stats(&gs->players[i]);
}
}
void ui_show_menu(void) {
clear_screen();
printf("\n ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
printf(" ■ SNAKE & LADDER v1.0 ■\n");
printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
printf(" ■ [1] New Game ■\n");
printf(" ■ [2] Load Game ■\n");
printf(" ■ [3] Custom Board ■\n");
printf(" ■ [4] View Rules ■\n");
printf(" ■ [5] Quit ■\n");
printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n\n");
}
