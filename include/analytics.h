#ifndef ANALYTICS_H
#define ANALYTICS_H

#include "dice.h"

/* Per-game aggregate statistics */
typedef struct {
	int total_turns;
	int total_snake_hits;
	int total_ladder_climbs;
	int longest_game_turns;  /* Turns in the longest single game */
	int shortest_game_turns; /* Turns in the shortest single game */
	int total_games;         /* For multi-game tracking */
	int dice_distribution[7]; /* [1..6] across all games */
} GameAnalytics;

void analytics_init(GameAnalytics *a);
void analytics_record_move(GameAnalytics *a, int roll, int was_snake,
							int was_ladder);
void analytics_end_game(GameAnalytics *a, int turns_in_game);
void analytics_print(const GameAnalytics *a);

#endif
