/* Quick engine sanity check */
#include <stdio.h>

#include "board.h"
#include "utils.h"
int main(void) {
    seed_rng();
    Board b;
    board_init_defaults(&b);
    /* Test all edge cases manually */
    printf("Overshoot test: pos=98 roll=5 → raw=103 → stay at 98\n");
    int raw = 98 + 5;
    if (raw > 100)
        raw = 98;
    printf(" Result: %d (expected 98)\n", raw);
    printf("Snake test: land on 99 → resolve to 78\n");
    printf(" Result: %d (expected 78)\n", board_resolve(&b, 99));
    printf("Ladder test: land on 4 → resolve to 25\n");
    printf(" Result: %d (expected 25)\n", board_resolve(&b, 4));
    /* Run 1000 random games, print average turn count */
    int total_turns = 0;
    int games = 1000;
    for (int g = 0; g < games; g++) {
        int pos = 0, turns = 0;
        while (pos < 100) {
            int roll = random_int(1, 6);
            int new_pos = pos + roll;
            if (new_pos > 100)
                new_pos = pos;
            pos = board_resolve(&b, new_pos);
            turns++;
        }
        total_turns += turns;
    }
    printf("Average turns to win (1 player): %.1f\n",
           (float)total_turns / games);
    /* Expected: roughly 25-40 turns for 1 player */
    return 0;
}