#include "../include/utils.h"
#include "../include/dice.h"
#include "../include/player.h"
#include "../include/board.h" 
#include <stdio.h>
int main(void) {
seed_rng();
/* Test board */
Board b;
board_init_defaults(&b);
board_print_legend(&b);
printf("Cell 99 type: %d (should be 1=SNAKE)\n",
board_cell_type(&b, 99));
printf("board_resolve(99) = %d (should be 78)\n",
board_resolve(&b, 99));
/* Test player */
Player p;
player_init(&p, "Alice", PLAYER_HUMAN, NULL);
printf("Player: %s at position %d\n", p.name, p.position);
/* Test dice */
DiceStats ds = {0};
for (int i = 0; i < 100; i++) dice_roll(&ds);
dice_print_stats(&ds);
return 0;
}
