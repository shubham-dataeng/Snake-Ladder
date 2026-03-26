#ifndef PLAYER_H
#define PLAYER_H
#include <stdbool.h>
#define MAX_NAME_LEN 32
#define MAX_PLAYERS 6 /* Max players in one game */
/*
 * PlayerType: distinguishes human from AI.
 * WHY an enum? In game.c, we branch on player type:
 * if (p->type == PLAYER_AI) ai_make_move(...)
 * else human_take_turn(...)
 * An enum makes this intent crystal-clear.
 */
typedef enum {
    PLAYER_HUMAN = 0,
    PLAYER_AI_EASY,
    PLAYER_AI_HARD,
} PlayerType;
/*
 * PlayerStats: per-player game statistics.
 * Separated from Player so analytics.c can work with just stats
 * without needing the full Player struct.
 */
typedef struct {
    int moves_taken;   /* Total dice rolls made */
    int snakes_hit;    /* Times landed on a snake head */
    int ladders_taken; /* Times landed on a ladder bottom */
    int max_roll;      /* Highest single dice roll */
    int times_at_99;   /* How many times got to 99 (risky) */
} PlayerStats;
/*
 * Player struct.
 *
 * WHY position 0 = "not yet started" and not position 1?
 * Standard Snake & Ladder rules: you start OFF the board and
 * need a roll of any number to enter at that roll value.
 * Cell 0 represents "waiting to enter."
 */
typedef struct {
    char name[MAX_NAME_LEN];
    int position; /* 0 = not started, 1-100 = on board */
    PlayerType type;
    bool has_won;
    int rank; /* 1 = first to win, 2 = second, etc. */
    PlayerStats stats;
    /*
     * color_code: ANSI escape code for this player's token color.
     * Stored here so ui.c can render the player in their color
     * without a big switch statement.
     * Example: "■[31m" = red, "■[32m" = green
     */
    char color_code[16];
} Player;
/* Function prototypes */
void player_init(Player *p, const char *name, PlayerType type,
                 const char *color_code);
void player_move(Player *p, int new_position);
bool player_has_won(const Player *p);
void player_print_stats(const Player *p);
#endif /* PLAYER_H */