#include "replay.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* usleep() */
#include <stdbool.h>

void replay_init(ReplayLog *log, int num_players,
				 const char names[][MAX_NAME_LEN]) {
	memset(log, 0, sizeof(ReplayLog));
	log->num_players = num_players;
	for (int i = 0; i < num_players; i++)
		strncpy(log->player_names[i], names[i], MAX_NAME_LEN - 1);
}

void replay_record(ReplayLog *log, int pidx, int roll, int before, int after,
				   int snake, int ladder, int bounce) {
	if (log->num_moves >= MAX_REPLAY_MOVES)
		return;
	ReplayMove *m = &log->moves[log->num_moves++];
	m->player_idx = pidx;
	m->roll = roll;
	m->pos_before = before;
	m->pos_after = after;
	m->was_snake = snake;
	m->was_ladder = ladder;
	m->was_bounce = bounce;
}

void replay_play(const ReplayLog *log) {
	printf("\n ■■■ REPLAY (%d moves) ■■■■■■■■■■■\n",
		   log->num_moves);
	for (int i = 0; i < log->num_moves; i++) {
		const ReplayMove *m = &log->moves[i];
		const char *name = log->player_names[m->player_idx];
		printf(" Turn %4d: %-12s rolled %d "
			   "%3d → %3d",
			   i + 1, name, m->roll, m->pos_before, m->pos_after);
		if (m->was_snake)
			printf(" ■ SNAKE");
		if (m->was_ladder)
			printf(" ■ LADDER");
		if (m->was_bounce)
			printf(" ■ BOUNCE");
		printf("\n");
		usleep(80000); /* 80ms pause between moves */
	}
	printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
}

bool replay_save(const ReplayLog *log, const char *path) {
	FILE *f = fopen(path, "wb");
	if (!f) {
		printf("Cannot open replay file: %s\n", path);
		return false;
	}

	/* Write header */
	if (fwrite(&log->num_players, sizeof(int), 1, f) != 1 ||
		fwrite(&log->num_moves, sizeof(int), 1, f) != 1) {
		printf("Error writing replay header\n");
		fclose(f);
		return false;
	}

	/* Write player names */
	for (int i = 0; i < log->num_players; i++) {
		if (fwrite(log->player_names[i], MAX_NAME_LEN, 1, f) != 1) {
			printf("Error writing player names\n");
			fclose(f);
			return false;
		}
	}

	/* Write moves */
	if (fwrite(log->moves, sizeof(ReplayMove), log->num_moves, f) !=
		(size_t)log->num_moves) {
		printf("Error writing replay moves\n");
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
}

bool replay_load(ReplayLog *log, const char *path) {
	FILE *f = fopen(path, "rb");
	if (!f) {
		printf("Cannot open replay file: %s\n", path);
		return false;
	}

	/* Read header */
	if (fread(&log->num_players, sizeof(int), 1, f) != 1 ||
		fread(&log->num_moves, sizeof(int), 1, f) != 1) {
		printf("Error reading replay header\n");
		fclose(f);
		return false;
	}

	/* Validate */
	if (log->num_players < 1 || log->num_players > MAX_PLAYERS ||
		log->num_moves < 0 || log->num_moves > MAX_REPLAY_MOVES) {
		printf("Invalid replay file\n");
		fclose(f);
		return false;
	}

	/* Read player names */
	for (int i = 0; i < log->num_players; i++) {
		if (fread(log->player_names[i], MAX_NAME_LEN, 1, f) != 1) {
			printf("Error reading player names\n");
			fclose(f);
			return false;
		}
	}

	/* Read moves */
	if (fread(log->moves, sizeof(ReplayMove), log->num_moves, f) !=
		(size_t)log->num_moves) {
		printf("Error reading replay moves\n");
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
}
