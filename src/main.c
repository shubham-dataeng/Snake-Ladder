/*
* main.c — Program entry point
* Phase 4: fully wired game loop
*/
#include <stdio.h>
#include "game.h"
#include "utils.h"
int main(void) {
seed_rng();
printf("\n ■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n");
printf(" ■ SNAKE & LADDER v1.0 ■\n");
printf(" ■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n\n");
int num = read_int(" How many players? (2-4): ", 2, 4);
GameState gs;
game_init(&gs, num);
game_loop(&gs);
printf("\n Thanks for playing!\n\n");
return 0;
}