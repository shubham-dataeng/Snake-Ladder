/*
 * player.c — Player state management
 * Depends: player.h, utils.h
 */
#include "player.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
/* ANSI color codes for up to 6 players */
static const char *PLAYER_COLORS[] = {
    "\033[91m", /* Bright red — Player 1 */
    "\033[92m", /* Bright green — Player 2 */
    "\033[93m", /* Bright yellow — Player 3 */
    "\033[94m", /* Bright blue — Player 4 */
    "\033[95m", /* Bright purple — Player 5 */
    "\033[96m", /* Bright cyan — Player 6 */
};
#define BOARD_SIZE 100
#define RESET_COLOR "\033[0m"
void player_init(Player *p, const char *name, PlayerType type,
                 const char *color_code) {
    memset(p, 0, sizeof(Player));
    /* Safely copy name — strncpy does NOT guarantee null termination */
    strncpy(p->name, name, MAX_NAME_LEN - 1);
    p->name[MAX_NAME_LEN - 1] = '\0'; /* Always null-terminate */
    p->position = 0;                  /* Off the board */
    p->type = type;
    p->has_won = false;
    p->rank = 0;
    /* Copy color code if provided, else use first color as default */
    if (color_code) {
        strncpy(p->color_code, color_code, sizeof(p->color_code) - 1);
        p->color_code[sizeof(p->color_code) - 1] = '\0';
    } else {
        strncpy(p->color_code, PLAYER_COLORS[0], sizeof(p->color_code) - 1);
    }
    /* Zero out stats (already done by memset, but be explicit) */
    memset(&p->stats, 0, sizeof(PlayerStats));
}
void player_init_all(Player players[], int count) {
    /*
     * Helper to init an array of players with sequential colors.
     * Called by game_init() in Phase 4.
     */
    if (count > MAX_PLAYERS)
        count = MAX_PLAYERS;
    for (int i = 0; i < count; i++) {
        /* Placeholder names — replaced by user input in game_init */
        char name[MAX_NAME_LEN];
        snprintf(name, sizeof(name), "Player %d", i + 1);
        player_init(&players[i], name, PLAYER_HUMAN, PLAYER_COLORS[i % 6]);
    }
}
void player_move(Player *p, int new_position) {
    /*
     * Update player position and record statistics.
     * new_position is already RESOLVED (after snake/ladder check).
     * The caller (game.c) passes in board_resolve(pos+roll).
     */
    int old = p->position;
    p->position = new_position;
    p->stats.moves_taken++;
    (void)old; /* Used by analytics in Phase 9 */
}
bool player_has_won(const Player *p) {
    return p->position >= BOARD_SIZE; /* 100 or more */
}
const char *player_color(const Player *p) {
    return p->color_code;
}
void player_print_stats(const Player *p) {
    printf(" %s%s%s — Stats:\n", p->color_code, p->name, RESET_COLOR);
    printf(" Moves taken : %d\n", p->stats.moves_taken);
    printf(" Snakes hit : %d\n", p->stats.snakes_hit);
    printf(" Ladders taken: %d\n", p->stats.ladders_taken);
    printf(" Max roll : %d\n", p->stats.max_roll);
}