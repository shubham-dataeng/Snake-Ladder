#include "dice.h"

#include <stdio.h>

#include "utils.h"

int dice_roll(DiceStats *stats) {
    int roll = random_int(1, 6);

    if (stats) {
        stats->total_rolls++;
        stats->face_counts[roll - 1]++;
    }

    return roll;
}

void dice_print_stats(const DiceStats *stats) {
    if (!stats || stats->total_rolls == 0) {
        printf("No dice rolls recorded.\n");
        return;
    }

    printf("Dice stats (%d rolls):\n", stats->total_rolls);
    for (int face = 1; face <= 6; face++) {
        printf("  %d: %d\n", face, stats->face_counts[face - 1]);
    }
}
