/*
 * utils.c — implementation of shared utility functions
 * Depends: utils.h
 */
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* ■■ RNG ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
void seed_rng(void) {
    /* time(NULL) = seconds since epoch → different seed each run */
    srand((unsigned int)time(NULL));
}
int random_int(int min, int max) {
    /*
     * rand() % N has modulo bias when RAND_MAX is not a
     * multiple of N. Fix: rejection sampling.
     * Accept only values in [0, limit) where limit is the
     * largest multiple of range that fits in RAND_MAX.
     */
    int range = max - min + 1;
    int limit = RAND_MAX - (RAND_MAX % range);
    int r;
    do {
        r = rand();
    } while (r >= limit);
    return min + (r % range);
}
/* ■■ Safe I/O ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
int read_int(const char *prompt, int min, int max) {
    int value;
    char buf[64];
    while (1) {
        printf("%s", prompt);
        fflush(stdout); /* Ensure prompt appears before blocking */
        /*
         * WHY fgets, not scanf?
         * scanf("%d") leaves '\n' in the buffer. The next
         * fgets() would instantly return empty.
         * fgets() consumes the entire line including '\n'.
         */
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            printf("\nUnexpected end of input.\n");
            exit(EXIT_FAILURE);
        }
        if (sscanf(buf, "%d", &value) != 1) {
            printf(" Enter a number between %d and %d.\n", min, max);
            continue;
        }
        if (value < min || value > max) {
            printf(" Out of range. Enter %d to %d.\n", min, max);
            continue;
        }
        return value;
    }
}
void read_string(const char *prompt, char *buf, int max_len) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, max_len, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0'; /* Strip trailing newline */
}
/* ■■ Terminal ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
void clear_screen(void) {
    /* ESC[2J = clear screen, ESC[H = cursor to top-left */
    printf("\033[2J\033[H");
    fflush(stdout);
}
void pause_for_enter(void) {
    printf("\n Press ENTER to continue...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* drain */
    }
}
/* ■■ Error handling ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
void die(const char *msg) {
    /* stderr is unbuffered — message always appears even if
    stdout is redirected to a file */
    fprintf(stderr, "\n[FATAL ERROR] %s\n", msg);
    exit(EXIT_FAILURE);
}