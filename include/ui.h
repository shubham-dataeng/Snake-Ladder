#ifndef UI_H
#define UI_H

#include "game.h"

/*
 * ui.h — User interface functions
 * Handles board rendering, status printing, and end-game display.
 */

/*
 * ui_render_board(gs) — Print the current game board state.
 * Shows player positions, snakes, and ladders in a 10x10 grid.
 */
void ui_render_board(const GameState *gs);

/*
 * ui_print_status(gs) — Print current game status.
 * Shows turn number, current player, and all player positions.
 */
void ui_print_status(const GameState *gs);

/*
 * ui_show_results(gs) — Print final game results and rankings.
 * Called after game_is_over() returns true.
 */
void ui_show_results(const GameState *gs);

/*
 * ui_show_dice_roll(roll, player_name, color) — Display ASCII dice art.
 * Shows a visual die face for the rolled value.
 */
void ui_show_dice_roll(int roll, const char *player_name, const char *color);

/*
 * ui_show_menu() — Display the main menu.
 */
void ui_show_menu(void);

#endif /* UI_H */
