#ifndef UTILS_H
#define UTILS_H
/*
 * utils.h — shared utility functions
 * Every module in this project includes this header.
 * Provides: safe I/O, RNG, terminal control, error handling.
 */
#include <stdio.h>
#include <stdlib.h>
/* ■■ RNG ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
/* seed_rng(): seed rand() with current time. Call ONCE in main(). */
void seed_rng(void);
/*
 * random_int(min, max) → int in [min, max]
 * Uses rejection sampling to eliminate modulo bias.
 * Example: random_int(1, 6) simulates a fair die roll.
 */
int random_int(int min, int max);
/* ■■ Safe I/O ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
/*
 * read_int(prompt, min, max) → int
 * Prints prompt, reads integer from stdin.
 * Loops until input is in [min, max].
 * Handles non-numeric input — no crash.
 */
int read_int(const char *prompt, int min, int max);
/*
 * read_string(prompt, buf, max_len)
 * Reads a line into buf. Strips trailing newline.
 */
void read_string(const char *prompt, char *buf, int max_len);
/* ■■ Terminal ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
/* clear_screen(): ANSI ESC[2J — works on Linux terminals */
void clear_screen(void);
/* pause_for_enter(): wait for ENTER key between turns */
void pause_for_enter(void);
/* ■■ Error handling ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
/* die(msg): print to stderr and exit(1). For unrecoverable errors. */
void die(const char *msg);
#endif /* UTILS_H */