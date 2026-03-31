#include "analytics.h"
#include <stdio.h>
#include <string.h>

void analytics_init(GameAnalytics *a) {
	memset(a, 0, sizeof(GameAnalytics));
	a->shortest_game_turns = 99999;
}

void analytics_record_move(GameAnalytics *a, int roll, int was_snake,
							int was_ladder) {
	a->total_turns++;
	if (roll >= 1 && roll <= 6)
		a->dice_distribution[roll]++;
	if (was_snake)
		a->total_snake_hits++;
	if (was_ladder)
		a->total_ladder_climbs++;
}

void analytics_end_game(GameAnalytics *a, int turns_in_game) {
	a->total_games++;
	if (turns_in_game > a->longest_game_turns)
		a->longest_game_turns = turns_in_game;
	if (turns_in_game < a->shortest_game_turns)
		a->shortest_game_turns = turns_in_game;
}

void analytics_print(const GameAnalytics *a) {
	printf("\n ■■■ Game Analytics ■■■■■■■■■■■■■■■■■■■\n");
	printf(" Total turns : %d\n", a->total_turns);
	printf(" Snake hits : %d (%.1f%%)\n",
		   a->total_snake_hits,
		   a->total_turns > 0
			   ? (float)a->total_snake_hits / a->total_turns * 100
			   : 0);
	printf(" Ladder climbs : %d (%.1f%%)\n",
		   a->total_ladder_climbs,
		   a->total_turns > 0
			   ? (float)a->total_ladder_climbs / a->total_turns * 100
			   : 0);
	printf("\n Dice distribution:\n");
	for (int i = 1; i <= 6; i++) {
		int count = a->dice_distribution[i];
		float pct = a->total_turns > 0
					   ? (float)count / a->total_turns * 100
					   : 0;
		printf(" [%d]: %4d (%4.1f%%) ", i, count, pct);
		int bars = (int)(pct / 3.0f);
		for (int b = 0; b < bars; b++)
			printf("■");
		printf("\n");
	}
	printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
}
