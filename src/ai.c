/*
 * ai.c — AI opponent intelligence
 * Depends: ai.h, board.h, player.h, utils.h
 * Compile with: -lm (for math functions)
 */

#include "ai.h"
#include "board.h"
#include "player.h"
#include "utils.h"
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>

/* ■■ Danger zone radius ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
/*
 * If a snake head is within DANGER_RADIUS cells ahead of us,
 * we consider that dangerous.
 * A roll of 1-6 can move us at most 6 cells — so radius 6 is maximum.
 */
#define DANGER_RADIUS 6

/* ■■ Position Evaluation ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
float ai_evaluate_position(const Board *b, int pos) {
    /*
     * Scoring function. Returns a float where:
     *   LOWER = BETTER for the AI (less dangerous)
     *   HIGHER = WORSE (more dangerous)
     *
     * Components:
     *   1. Distance penalty: further from 100 is worse
     *   2. Snake proximity penalty: near snake heads is bad
     *   3. Ladder proximity bonus: near ladder bottoms is good
     *   4. Exact snake head penalty: on a snake head is very bad
     */

    if (pos <= 0) return 1000.0f;     /* Off board — worst */
    if (pos >= 100) return -1000.0f;  /* Win cell — best */

    /* Start with distance penalty: further from 100 is worse */
    float score = (float)(100 - pos) * 0.5f;

    /* ■ Scan snakes: penalize proximity to snake heads ■ */
    for (int i = 0; i < b->num_snakes; i++) {
        int head = b->snakes[i].head;
        int tail = b->snakes[i].tail;
        int drop = head - tail;  /* How far the snake drops you */

        /* Distance from our position to the snake head */
        int dist = head - pos;

        /* Only care about snake heads within rolling distance */
        if (dist > 0 && dist <= DANGER_RADIUS) {
            /*
             * Penalty = drop_amount / distance
             * A snake that drops you far (big drop) and is nearby (small dist)
             * is the most dangerous.
             */
            score += (float)drop / (float)dist * 2.0f;
        }

        /* Already ON a snake head — very bad */
        if (pos == head) {
            score += (float)drop * 5.0f;
        }
    }

    /* ■ Scan ladders: reward proximity to ladder bottoms ■ */
    for (int i = 0; i < b->num_ladders; i++) {
        int bottom = b->ladders[i].bottom;
        int top = b->ladders[i].top;
        int climb = top - bottom;  /* How far the ladder lifts you */

        /* Distance from our position to the ladder bottom */
        int dist = bottom - pos;

        /* Only care about ladders we can reach */
        if (dist > 0 && dist <= DANGER_RADIUS) {
            /*
             * Reward = climb / distance (reachable high-climb ladder is great)
             */
            score -= (float)climb / (float)dist * 1.5f;
        }
    }

    return score;
}

/* ■■ AI Move Decision ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
int ai_make_move(const GameState *gs, int player_idx) {
    /*
     * Hard AI strategy:
     * 1. Roll the dice (we cannot control this)
     * 2. Simulate all 6 possible outcomes
     * 3. Compare scores: which outcome would be best?
     * 4. For standard rules: just roll randomly
     * 5. For variant rules: could decide whether to re-roll
     */

    const Player *p = &gs->players[player_idx];
    const Board *b = &gs->board;

    /* Evaluate all 6 possible rolls */
    float best_score = FLT_MAX;
    int best_roll = 1;

    for (int roll = 1; roll <= 6; roll++) {
        int raw = p->position + roll;

        /* Handle overshoot: stay in place if past 100 */
        if (raw > BOARD_SIZE) {
            raw = p->position;
        }

        /* Check what cell we'd actually land on (after snakes/ladders) */
        int resolved = board_resolve(b, raw);

        /* Score this position */
        float score = ai_evaluate_position(b, resolved);

        if (score < best_score) {
            best_score = score;
            best_roll = roll;
        }
    }

    /* For standard game: actually roll the die (can't choose outcome) */
    int actual_roll = random_int(1, 6);

    /* Hard AI shows analysis (verbose mode) */
    if (p->type == PLAYER_AI_HARD) {
        printf(" AI analysis: optimal roll would be %d (score: %.1f)\n",
               best_roll, best_score);
        printf(" Actual roll: %d\n", actual_roll);
    }

    return actual_roll;
}
