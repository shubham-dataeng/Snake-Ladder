#include "board.h"

#include <stdio.h>

static bool is_valid_pos(int pos) {
	return pos >= 1 && pos <= BOARD_SIZE;
}

void board_rebuild_lookup(Board *b) {
	for (int pos = 0; pos <= BOARD_SIZE; pos++) {
		b->cell_type[pos] = CELL_NORMAL;
		b->destination[pos] = pos;
	}

	b->cell_type[0] = CELL_START;
	b->cell_type[BOARD_SIZE] = CELL_END;

	for (int i = 0; i < b->num_snakes; i++) {
		int head = b->snakes[i].head;
		int tail = b->snakes[i].tail;
		b->cell_type[head] = CELL_SNAKE;
		b->destination[head] = tail;
	}

	for (int i = 0; i < b->num_ladders; i++) {
		int bottom = b->ladders[i].bottom;
		int top = b->ladders[i].top;
		b->cell_type[bottom] = CELL_LADDER;
		b->destination[bottom] = top;
	}
}

bool board_add_snake(Board *b, int head, int tail) {
	if (b->num_snakes >= MAX_SNAKES) {
		return false;
	}
	if (!is_valid_pos(head) || !is_valid_pos(tail) || head <= tail || head == BOARD_SIZE) {
		return false;
	}

	b->snakes[b->num_snakes].head = head;
	b->snakes[b->num_snakes].tail = tail;
	b->num_snakes++;
	board_rebuild_lookup(b);
	return true;
}

bool board_add_ladder(Board *b, int bottom, int top) {
	if (b->num_ladders >= MAX_LADDERS) {
		return false;
	}
	if (!is_valid_pos(bottom) || !is_valid_pos(top) || bottom >= top || top == BOARD_SIZE) {
		return false;
	}

	b->ladders[b->num_ladders].bottom = bottom;
	b->ladders[b->num_ladders].top = top;
	b->num_ladders++;
	board_rebuild_lookup(b);
	return true;
}

void board_init_defaults(Board *b) {
	b->num_snakes = 0;
	b->num_ladders = 0;
	board_rebuild_lookup(b);

	board_add_snake(b, 99, 78);
	board_add_snake(b, 95, 56);
	board_add_snake(b, 87, 24);
	board_add_snake(b, 62, 18);

	board_add_ladder(b, 4, 14);
	board_add_ladder(b, 9, 31);
	board_add_ladder(b, 20, 38);
	board_add_ladder(b, 28, 84);
}

int board_resolve(const Board *b, int pos) {
	if (pos < 0) {
		return 0;
	}
	if (pos > BOARD_SIZE) {
		return pos;
	}
	return b->destination[pos];
}

CellType board_cell_type(const Board *b, int pos) {
	if (pos < 0 || pos > BOARD_SIZE) {
		return CELL_NORMAL;
	}
	return b->cell_type[pos];
}

void board_print_legend(const Board *b) {
	printf("Board legend:\n");
	printf("  Snakes (%d):\n", b->num_snakes);
	for (int i = 0; i < b->num_snakes; i++) {
		printf("    %d -> %d\n", b->snakes[i].head, b->snakes[i].tail);
	}

	printf("  Ladders (%d):\n", b->num_ladders);
	for (int i = 0; i < b->num_ladders; i++) {
		printf("    %d -> %d\n", b->ladders[i].bottom, b->ladders[i].top);
	}
}

bool board_validate(const Board *b) {
	for (int i = 0; i < b->num_snakes; i++) {
		if (!is_valid_pos(b->snakes[i].head) || !is_valid_pos(b->snakes[i].tail) ||
			b->snakes[i].head <= b->snakes[i].tail) {
			return false;
		}
	}

	for (int i = 0; i < b->num_ladders; i++) {
		if (!is_valid_pos(b->ladders[i].bottom) || !is_valid_pos(b->ladders[i].top) ||
			b->ladders[i].bottom >= b->ladders[i].top) {
			return false;
		}
	}

	return true;
}
