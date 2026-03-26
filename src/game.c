/*
 * execute_move(): apply one complete turn for a player.
 * Returns: description of what happened (for UI display)
 *
 * SEQUENCE:
 * 1. Roll dice
 * 2. Calculate raw new position (overshoot = stay in place)
 * 3. Resolve through snake/ladder lookup
 * 4. Update player position and stats
 * 5. Check win condition
 */
#include "game.h"
typedef enum {
    MOVE_NORMAL,
    MOVE_SNAKE,
    MOVE_LADDER,
    MOVE_WIN,
    MOVE_BOUNCE, /* Bounced back from cell 100 */
} MoveResult;
MoveResult execute_move(GameState *gs, int player_idx, int roll) {
    Player *p = &gs->players[player_idx];
    Board *b = &gs->board;
    int raw_new = p->position + roll;
    bool overshot = false;
    /* Step 1: Handle overshoot */
    if (raw_new > BOARD_SIZE) {
        raw_new = p->position;
        overshot = true;
    }
    /* Step 2: Resolve through snakes/ladders */
    int resolved = board_resolve(b, raw_new);
    /* Step 3: Update stats before moving */
    CellType ct = board_cell_type(b, raw_new);
    if (ct == CELL_SNAKE) {
        p->stats.snakes_hit++;
    }
    if (ct == CELL_LADDER) {
        p->stats.ladders_taken++;
    }
    if (roll > p->stats.max_roll)
        p->stats.max_roll = roll;
    /* Step 4: Move */
    player_move(p, resolved);
    /* Step 5: Check win */
    if (player_has_won(p)) {
        game_declare_winner(gs, player_idx);
        return MOVE_WIN;
    }
    /* Step 6: Return result for UI */
    if (overshot)
        return MOVE_BOUNCE;
    if (ct == CELL_SNAKE)
        return MOVE_SNAKE;
    if (ct == CELL_LADDER)
        return MOVE_LADDER;
    return MOVE_NORMAL;
}