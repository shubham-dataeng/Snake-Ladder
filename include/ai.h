#ifndef AI_H
#define AI_H

#include "game.h"

/*
 * ai.h — AI opponent intelligence
 *
 * Two AI strategies:
 *   EASY: Random rolls with delay (simulates thinking)
 *   HARD: Probability-based position scoring
 */

/*
 * ai_evaluate_position(b, pos) → float
 *
 * Score a board position for the Hard AI.
 * Lower score = better (safer) position
 * Higher score = worse (more dangerous) position
 *
 * Considers:
 *   - Distance to winning cell (100)
 *   - Proximity to snake heads (bad)
 *   - Proximity to ladder bottoms (good)
 *   - Already on a snake head (very bad)
 */
float ai_evaluate_position(const Board *b, int pos);

/*
 * ai_make_move(gs, player_idx) → int
 *
 * Simulate AI turn. Returns the roll value (1-6).
 * For standard rules: just rolls and reports.
 * For variants with re-roll: could decide whether to re-roll.
 *
 * Hard AI shows analysis of what roll would be optimal.
 */
int ai_make_move(const GameState *gs, int player_idx);

#endif /* AI_H */
