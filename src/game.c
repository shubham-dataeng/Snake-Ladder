/*
* game.c — Game loop and state machine
* Depends: game.h, board.h, player.h, dice.h, ui.h, utils.h, save.h, ai.h
*/
#include "game.h"
#include "board.h"
#include "player.h"
#include "dice.h"
#include "ui.h"
#include "utils.h"
#include "save.h"
#include "ai.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
/* ■■ Forward declarations ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
static void phase_setup(GameState *gs);
static MoveResult execute_move(GameState *gs, int pidx, int roll);
/* ■■ Initialization ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
void game_init(GameState *gs, int num_players) {
memset(gs, 0, sizeof(GameState));
gs->version = GAME_VERSION;
gs->num_players = num_players;
gs->phase = GPHASE_SETUP;
gs->turn_number = 0;
gs->num_winners = 0;
gs->ai_enabled = false;
gs->analytics_on= true;
gs->replay_on = false;
/* Initialize board with default layout */
board_init_defaults(&gs->board);
/* Initialize players with default names and sequential colors */
const char *colors[] = {"\033[91m","\033[92m","\033[93m",
"\033[94m","\033[95m","\033[96m"};
for (int i = 0; i < num_players; i++) {
char name[MAX_NAME_LEN];
snprintf(name, sizeof(name), "Player %d", i + 1);
player_init(&gs->players[i], name, PLAYER_HUMAN, colors[i%6]);
}
}
/* ■■ Setup phase ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
* Collect player names and types (human/AI).
*/
static void phase_setup(GameState *gs) {
clear_screen();
printf("\n ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
printf(" ■ SNAKE & LADDER — SETUP ■\n");
printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n\n");
for (int i = 0; i < gs->num_players; i++) {
Player *p = &gs->players[i];
char name[MAX_NAME_LEN];
printf(" Player %d\n", i + 1);
read_string(" Name: ", name, MAX_NAME_LEN);
if (strlen(name) == 0)
snprintf(name, MAX_NAME_LEN, "Player %d", i + 1);
/* Copy name safely */
strncpy(p->name, name, MAX_NAME_LEN - 1);
p->name[MAX_NAME_LEN - 1] = '\0';
/* Ask type */
printf(" Type: (1) Human (2) AI Easy (3) AI Hard\n");
int type_choice = read_int(" Choice: ", 1, 3);
switch (type_choice) {
case 1: p->type = PLAYER_HUMAN; break;
case 2: p->type = PLAYER_AI_EASY; break;
case 3: p->type = PLAYER_AI_HARD; break;
}
printf("\n");
}
gs->phase = GPHASE_PLAYING;
}
/* ■■ Main game loop ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
void game_loop(GameState *gs) {
    if (gs->phase == GPHASE_SETUP)
phase_setup(gs);
while (gs->phase == GPHASE_PLAYING) {
game_next_turn(gs);
if (game_is_over(gs)) {
gs->phase = GPHASE_OVER;
ui_show_results(gs);
}
}
}
/* ■■ Save game to slot ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
void game_save(GameState *gs, int slot) {
    if (save_game(gs, slot)) {
        printf(" \n ✓ Game saved to slot %d\n", slot);
        pause_for_enter();
    } else {
        printf(" \n ✗ Failed to save game\n");
        pause_for_enter();
    }
}
void game_next_turn(GameState *gs) {
int idx = gs->current_player_idx;
Player *p = &gs->players[idx];
DiceStats *ds = &gs->dice_stats; /* Added to GameState */
/* Skip players who have already won */
if (p->has_won) {
gs->current_player_idx = (idx + 1) % gs->num_players;
return;
}
/* Render board before this player's turn */
ui_render_board(gs);
ui_print_status(gs);
int roll;
if (p->type == PLAYER_HUMAN) {
/* Human: offer pause option before rolling */
printf("\n %s%s\033[0m — [ENTER] to roll, [S] to save & quit\n",
p->color_code, p->name);
fflush(stdout);
int c;
while ((c = getchar()) != '\n' && c != EOF) {
    if (c == 's' || c == 'S') {
        /* Save game and exit loop */
        printf(" \n Save game to slot (1-3): ");
        int slot = read_int("", 1, 3);
        game_save(gs, slot);
        gs->phase = GPHASE_OVER;  /* Exit game gracefully */
        return;
    }
}
roll = dice_roll(ds);
} else if (p->type == PLAYER_AI_EASY) {
/* Easy AI: just roll with a delay */
printf("\n %s%s\033[0m is thinking...\033[0m\n",
p->color_code, p->name);
usleep(300000);  /* 0.3 second pause */
roll = ai_make_move(gs, idx);
} else if (p->type == PLAYER_AI_HARD) {
/* Hard AI: strategic analysis */
printf("\n %s%s\033[0m is calculating...\033[0m\n",
p->color_code, p->name);
usleep(500000);  /* 0.5 second pause (more "thinking") */
roll = ai_make_move(gs, idx);
} else {
/* Fallback (shouldn't reach here) */
roll = dice_roll(ds);
}
printf(" Rolled: %d\n", roll);
gs->turn_number++;
/* Execute the move */
MoveResult result = execute_move(gs, idx, roll);
/* Report what happened */
switch (result) {
case MOVE_NORMAL:
printf(" → Moved to cell %d\n", p->position); break;
case MOVE_SNAKE:
printf(" ■ SNAKE! Slid down to cell %d\n", p->position); break;
case MOVE_LADDER:
printf(" ■ LADDER! Climbed up to cell %d\n", p->position); break;
case MOVE_WIN:
printf(" ■ %s WINS! (Turn %d)\n", p->name, gs->turn_number); break;
}
pause_for_enter();
/* Advance to next player */
gs->current_player_idx = (idx + 1) % gs->num_players;
}
bool game_is_over(const GameState *gs) {
/*
* Game ends when either:
* (a) At least one player has won (classic: first to 100 wins)
* (b) All but one player has won (if playing "rank all players")
*/
if (gs->num_winners >= 1) return true; /* Simple: first wins = game over */
/* For multi-winner mode (all players finish): */
/* return gs->num_winners >= gs->num_players - 1; */
return false;
}
void game_declare_winner(GameState *gs, int player_idx) {
Player *p = &gs->players[player_idx];
if (p->has_won) return; /* Already declared */
p->has_won = true;
p->rank = gs->num_winners + 1;
gs->winners[gs->num_winners] = player_idx;
gs->num_winners++;
}
Player* game_current_player(GameState *gs) {
return &gs->players[gs->current_player_idx];
}
/* ■■ execute_move(): the core move logic ■■■■■■■■■■■■■■■■■■■■■■■■ */
static MoveResult execute_move(GameState *gs, int pidx, int roll) {
Player *p = &gs->players[pidx];
Board *b = &gs->board;
int raw_new = p->position + roll;
/* If overshoot past 100, stay at current position */
if (raw_new > BOARD_SIZE) {
raw_new = p->position;
}
CellType ct = board_cell_type(b, raw_new);
int resolved = board_resolve(b, raw_new);
/* Update stats */
if (ct == CELL_SNAKE) p->stats.snakes_hit++;
if (ct == CELL_LADDER) p->stats.ladders_taken++;
if (roll > p->stats.max_roll) p->stats.max_roll = roll;
player_move(p, resolved);
if (player_has_won(p)) { game_declare_winner(gs, pidx); return MOVE_WIN; }
if (ct == CELL_SNAKE) return MOVE_SNAKE;
if (ct == CELL_LADDER) return MOVE_LADDER;
return MOVE_NORMAL;
}