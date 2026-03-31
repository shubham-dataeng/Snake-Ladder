#include "network.h"
#include "game.h"
#include "player.h"
#include "board.h"
#include "dice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/* Helper: Serialize game state to JSON-like string */
static void serialize_game_state(const GameState *gs, char *buffer, int max_len) {
	snprintf(buffer, max_len,
	         "board_sz:100|p1_pos:%d|p2_pos:%d|turn:%d|p1_name:%s|p2_name:%s|phase:%d",
	         gs->players[0].position,
	         gs->players[1].position,
	         gs->current_player_idx,
	         gs->players[0].name,
	         gs->players[1].name,
	         gs->phase);
}

/* Helper: Send game state to both clients */
static void broadcast_game_state(ServerContext *server, const GameState *gs, int roll, const char *event) {
	NetworkMessage msg;
	message_init(&msg, MSG_GAME_STATE, 0);

	serialize_game_state(gs, msg.payload, sizeof(msg.payload));

	/* Append event info */
	char temp[256];
	snprintf(temp, sizeof(temp), "|roll:%d|event:%s|move_num:%d",
	         roll, event ? event : "normal", gs->turn_number);
	strncat(msg.payload, temp, sizeof(msg.payload) - strlen(msg.payload) - 1);

	server_send_message(server, 0, &msg);
	server_send_message(server, 1, &msg);
}

/* Helper: Convert move result to event string */
static const char *move_result_to_event(MoveResult result) {
	switch (result) {
	case MOVE_NORMAL: return "normal";
	case MOVE_SNAKE: return "snake";
	case MOVE_LADDER: return "ladder";
	case MOVE_WIN: return "win";
	default: return "unknown";
	}
}

/* Main server game loop */
void server_game_loop(ServerContext *server, GameState *gs) {
	printf("[Server] Game loop started\n");

	/* Send game start to both clients */
	for (int i = 0; i < 2; i++) {
		NetworkMessage msg;
		message_init(&msg, MSG_GAME_START, i + 1);
		serialize_game_state(gs, msg.payload, sizeof(msg.payload));
		server_send_message(server, i, &msg);
	}

	gs->phase = GPHASE_PLAYING;
	int max_turns = 1000; /* Safety limit */

	while (gs->phase == GPHASE_PLAYING && max_turns-- > 0) {
		int pidx = gs->current_player_idx;
		Player *p = &gs->players[pidx];

		printf("[Server] Turn %d: %s's turn\n", gs->turn_number, p->name);

		/* Skip if already won */
		if (p->has_won) {
			gs->current_player_idx = (pidx + 1) % 2;
			continue;
		}

		/* Notify current player it's their turn */
		NetworkMessage turn_msg;
		message_init(&turn_msg, MSG_YOUR_TURN, pidx + 1);
		snprintf(turn_msg.payload, sizeof(turn_msg.payload), "timeout:30");
		server_send_message(server, pidx, &turn_msg);

		printf("[Server] Sent YOUR_TURN to player %d\n", pidx + 1);

		/* Wait for roll (with 30 second timeout) */
		NetworkMessage roll_msg;
		bool roll_received = false;

		for (int retry = 0; retry < 30; retry++) {
			if (!socket_has_data(server->clients[pidx].socket_fd, 1)) {
				continue; /* Wait another second */
			}

			if (server_recv_message(server, pidx, &roll_msg)) {
				if (roll_msg.type == MSG_ROLL) {
					roll_received = true;
					break;
				}
			}
		}

		if (!roll_received) {
			printf("[Server] Player %d timeout, auto-skip\n", pidx + 1);
			gs->current_player_idx = (pidx + 1) % 2;
			continue;
		}

		/* Extract roll value from payload */
		int roll = 0;
		sscanf(roll_msg.payload, "roll:%d", &roll);

		/* Validate roll */
		if (!validate_roll(roll)) {
			printf("[Server] Invalid roll from player %d: %d\n", pidx + 1, roll);

			NetworkMessage err_msg;
			message_init(&err_msg, MSG_ERROR, pidx + 1);
			snprintf(err_msg.payload, sizeof(err_msg.payload), "code:1001|reason:invalid_roll");
			server_send_message(server, pidx, &err_msg);

			/* Resend YOUR_TURN */
			server_send_message(server, pidx, &turn_msg);
			continue;
		}

		printf("[Server] Player %d rolled %d\n", pidx + 1, roll);

		/* Execute move on server (authoritative) */
		int old_pos = p->position;
		int raw_new = p->position + roll;
		int resolved = raw_new;

		if (raw_new > BOARD_SIZE) {
			resolved = p->position; /* Overshoot, stay put */
		} else {
			resolved = board_resolve(&gs->board, raw_new);
		}

		p->position = resolved;
		gs->turn_number++;

		CellType ct = (raw_new <= BOARD_SIZE) ? board_cell_type(&gs->board, raw_new) : CELL_NORMAL;
		MoveResult result = MOVE_NORMAL;

		if (raw_new > BOARD_SIZE) {
			result = MOVE_NORMAL; /* Overshoot */
		} else if (ct == CELL_SNAKE) {
			result = MOVE_SNAKE;
			p->stats.snakes_hit++;
		} else if (ct == CELL_LADDER) {
			result = MOVE_LADDER;
			p->stats.ladders_taken++;
		}

		if (resolved >= BOARD_SIZE) {
			p->has_won = true;
			p->rank = 1;
			gs->num_winners++;
			result = MOVE_WIN;
			gs->phase = GPHASE_OVER;
		}

		const char *event = move_result_to_event(result);
		printf("[Server] Player %d: %d → %d [%s]\n", pidx + 1, old_pos, resolved, event);

		/* Broadcast updated state to both clients */
		broadcast_game_state(server, gs, roll, event);

		/* Move to next player */
		gs->current_player_idx = (pidx + 1) % 2;

		/* Small delay for readability */
		usleep(500000);
	}

	/* Game over */
	if (gs->num_winners > 0) {
		printf("[Server] Player %s WINS!\n", gs->players[gs->winners[0]].name);
	}

	gs->phase = GPHASE_OVER;

	/* Send game over to both clients */
	for (int i = 0; i < 2; i++) {
		NetworkMessage msg;
		message_init(&msg, MSG_GAME_OVER, i + 1);
		int winner = gs->winners[0];
		snprintf(msg.payload, sizeof(msg.payload), "winner_id:%d|winner_name:%s",
		         winner + 1, gs->players[winner].name);
		server_send_message(server, i, &msg);
	}

	printf("[Server] Game loop ended\n");
}

/* Main server entry point */
int main(int argc, char *argv[]) {
	int port = DEFAULT_PORT;
	if (argc > 1) {
		port = atoi(argv[1]);
	}

	ServerContext server;
	if (!server_init(&server, port)) {
		fprintf(stderr, "Failed to initialize server\n");
		return 1;
	}

	printf("[Server] Waiting for 2 players...\n");

	/* Accept first client */
	while (server.num_connected < 1) {
		server_accept_clients(&server);
		usleep(100000);
	}

	printf("[Server] Player 1 ready: %s\n", server.clients[0].player_name);

	/* Accept second client */
	while (server.num_connected < 2) {
		server_accept_clients(&server);
		usleep(100000);
	}

	printf("[Server] All players connected!\n");

	/* Initialize game */
	GameState gs;
	game_init(&gs, 2);
	strncpy(gs.players[0].name, "Player1", MAX_NAME_LEN - 1);
	strncpy(gs.players[1].name, "Player2", MAX_NAME_LEN - 1);

	/* Send initial connection ack to both clients */
	for (int i = 0; i < 2; i++) {
		NetworkMessage msg;
		message_init(&msg, MSG_CONNECTION_ACK, i + 1);
		int opponent = (i == 0) ? 1 : 0;
		snprintf(msg.payload, sizeof(msg.payload),
		         "player_id:%d|opponent:%s",
		         i + 1, gs.players[opponent].name);
		server_send_message(&server, i, &msg);
	}

	/* Run game loop */
	server_game_loop(&server, &gs);

	server_cleanup(&server);
	printf("[Server] Shutdown complete\n");
	return 0;
}
