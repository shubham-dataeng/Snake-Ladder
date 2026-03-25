#ifndef DICE_H
#define DICE_H

typedef struct {
	int total_rolls;
	int face_counts[6];
} DiceStats;

int dice_roll(DiceStats *stats);
void dice_print_stats(const DiceStats *stats);

#endif /* DICE_H */
