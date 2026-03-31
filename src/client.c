#include "network.h"
#include "ui.h"
#include "board.h"
#include "player.h"
#include "dice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
	int player_id;
	int opponent_id;
	char opponent_name[64];
	int positions[2];
	int current_turn;
	int turn_number;
	bool game_active;
} GameDisplay;

/* Helper: Parse game state from network message */
static void parse_game_state(const char *payload, GameDisplay *display) {
	sscanf(payload, "board_sz:100|p1_pos:%d|p2_pos:%d|turn:%d",
	       &display->positions[0],
	       &display->positions[1],
	       &display->current_turn);
}

/* Helper: Display local board */
static void display_board(const GameDisplay *display, const char *my_name, const char *opp_name) {
	clear_screen();
	printf("\n ■■■ SNAKE & LADDER — NETWORK GAME ■■■■■■■■■■■■■■■■■■\n");
	printf(" ■\n");
	printf(" ■  You: %s (Player %d)\n", my_name, display->player_id);
	printf(" ■  Opponent: %s (Player %d)\n", opp_name, display->opponent_id);
	printf(" ■\n");
	printf(" ■  Board (simplified):\n");

	/* Simple text board */
	for (int row = 9; row >= 0; row--) {
		printf(" ■  ");
		for (int col = 0; col < 10; col++) {
			int cell = row * 10 + col + 1;
			if (cell > 100) {
				printf("     ");
			} else if (display->positions[0] == cell && display->positions[1] == cell) {
				printf(" [12] "); /* Both */
			} else if (display->positions[0] == cell) {
				printf(" [1] ");
			} else if (display->positions[1] == cell) {
				printf(" [2] ");
			} else {
				printf(" %3d ", cell);
			}
		}
		printf("\n");
	}

	printf(" ■\n");
	printf(" ■  Player 1 cell: %d\n", display->positions[0]);
	printf(" ■  Player 2 cell: %d\n", display->positions[1]);
	printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
}

/* Helper: Display status message */
static void display_status(const GameDisplay *display, const char *message) {
	printf("\n [Status] %s\n", message);
	if (display->current_turn == display->player_id) {
		printf(" [Action] Your turn! Press [ENTER] to roll dice:\n");
	} else {
		printf(" [Waiting] Opponent's turn...\n");
	}
}

/* Main client game loop */
void client_game_loop(ClientContext *client, const char *player_name) {
	GameDisplay display;
	display.player_id = 0;
	display.opponent_id = 0;
	display.game_active = true;

	printf("[Client] Waiting for game to start...\n");

	/* Wait for CONNECTION_ACK */
	NetworkMessage msg;
	if (!client_recv_message(client, &msg)) {
		fprintf(stderr, "[Client] Failed to receive connection ack\n");
		return;
	}

	if (msg.type == MSG_CONNECTION_ACK) {
		display.player_id = msg.player_id;
		display.opponent_id = (display.player_id == 1) ? 2 : 1;
		sscanf(msg.payload, "player_id:%d|opponent:%63s",
		       &display.player_id, display.opponent_name);
		printf("[Client] Connected as Player %d\n", display.player_id);
		printf("[Client] Opponent: %s\n", display.opponent_name);
	}

	/* Wait for GAME_START */
	if (!client_recv_message(client, &msg)) {
		fprintf(stderr, "[Client] Failed to receive game start\n");
		return;
	}

	printf("[Client] Game started!\n");

	/* Main game loop */
	while (display.game_active) {
		/* Receive next game state or turn notification */
		if (!client_recv_message(client, &msg)) {
			fprintf(stderr, "[Client] Connection lost\n");
			break;
		}

		switch (msg.type) {
		case MSG_GAME_STATE: {
			parse_game_state(msg.payload, &display);
			const char *event = "";
			if (strstr(msg.payload, "event:snake")) {
				event = " [SNAKE HIT!]";
			} else if (strstr(msg.payload, "event:ladder")) {
				event = " [LADDER CLIMBED!]";
			}
			display_board(&display, player_name, display.opponent_name);
			printf("\n [Update] Game state received%s\n", event);
			break;
		}

		case MSG_YOUR_TURN: {
			display_board(&display, player_name, display.opponent_name);
			display_status(&display, "It's your turn!");

			/* Wait for user input */
			printf(" Press [ENTER] to roll: ");
			fflush(stdout);
			getchar();

			/* Roll dice */
			int roll = (rand() % 6) + 1;
			printf(" You rolled: %d\n", roll);

			/* Send roll to server */
			NetworkMessage roll_msg;
			message_init(&roll_msg, MSG_ROLL, display.player_id);
			snprintf(roll_msg.payload, sizeof(roll_msg.payload), "roll:%d", roll);

			if (!client_send_message(client, &roll_msg)) {
				fprintf(stderr, "[Client] Failed to send roll\n");
				display.game_active = false;
			}
			break;
		}

		case MSG_WAITING: {
			display_board(&display, player_name, display.opponent_name);
			display_status(&display, "Waiting for opponent...");
			printf("\n");
			break;
		}

		case MSG_GAME_OVER: {
			display.game_active = false;
			const char *winner = "";
			if (strstr(msg.payload, "winner_id:1")) {
				winner = display.opponent_id == 1 ? display.opponent_name : player_name;
			} else {
				winner = display.opponent_id == 2 ? display.opponent_name : player_name;
			}
			printf("\n ■■■ GAME OVER ■■■\n");
			printf(" Winner: %s\n", winner);
			printf(" ■■■■■■■■■■■■■■■\n\n");
			break;
		}

		case MSG_ERROR: {
			printf(" [ERROR] %s\n", msg.payload);
			break;
		}

		case MSG_DISCONNECT: {
			printf("\n [Disconnected] Opponent left\n");
			display.game_active = false;
			break;
		}

		default:
			printf(" [Debug] Received message type: %s\n", message_type_str(msg.type));
			break;
		}
	}

	printf("[Client] Game loop ended\n");
}

/* Main client entry point */
int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <server_ip> [port]\n", argv[0]);
		fprintf(stderr, "Example: %s localhost 5000\n", argv[0]);
		return 1;
	}

	const char *server_ip = argv[1];
	int server_port = DEFAULT_PORT;
	if (argc > 2) {
		server_port = atoi(argv[2]);
	}

	printf("╔════════════════════════════════════════╗\n");
	printf("║  SNAKE & LADDER — Network Client       ║\n");
	printf("╚════════════════════════════════════════╝\n\n");

	printf("Connecting to %s:%d...\n", server_ip, server_port);

	ClientContext client;

	/* Try to connect with retries */
	for (int retry = 1; retry <= 3; retry++) {
		if (client_connect(&client, server_ip, server_port)) {
			break;
		}
		if (retry < 3) {
			printf("Retry %d in 2 seconds...\n", retry);
			sleep(2);
		} else {
			fprintf(stderr, "Failed to connect after 3 attempts\n");
			return 1;
		}
	}

	char player_name[64];
	printf("Enter your name: ");
	fflush(stdout);
	if (!fgets(player_name, sizeof(player_name), stdin)) {
		strcpy(player_name, "Player");
	} else {
		/* Remove newline */
		player_name[strcspn(player_name, "\n")] = '\0';
	}

	printf("Welcome, %s!\n\n", player_name);

	/* Send connection handshake */
	NetworkMessage connect_msg;
	message_init(&connect_msg, MSG_CONNECT, 0);
	strncpy(connect_msg.payload, player_name, sizeof(connect_msg.payload) - 1);

	if (!client_send_message(&client, &connect_msg)) {
		fprintf(stderr, "Failed to send connection message\n");
		client_disconnect(&client);
		return 1;
	}

	/* Run game loop */
	client_game_loop(&client, player_name);

	client_disconnect(&client);
	printf("[Client] Disconnected\n");
	return 0;
}
