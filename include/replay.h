#ifndef REPLAY_H
#define REPLAY_H

#include "player.h"

#define MAX_REPLAY_MOVES 2000

typedef struct {
	int player_idx;
	int roll;
	int pos_before;
	int pos_after;
	int was_snake;
	int was_ladder;
	int was_bounce;
} ReplayMove;

typedef struct {
	ReplayMove moves[MAX_REPLAY_MOVES];
	int num_moves;
	int num_players;
	char player_names[MAX_PLAYERS][MAX_NAME_LEN];
} ReplayLog;

void replay_init(ReplayLog *log, int num_players,
				 const char names[][MAX_NAME_LEN]);
void replay_record(ReplayLog *log, int player_idx, int roll,
				   int before, int after, int snake, int ladder,
				   int bounce);
void replay_play(const ReplayLog *log);
bool replay_save(const ReplayLog *log, const char *path);
bool replay_load(ReplayLog *log, const char *path);

#endif
