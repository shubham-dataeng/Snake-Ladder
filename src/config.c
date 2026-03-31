/*
* config.c — Board configuration file parser
* Depends: config.h, board.h, utils.h
*/

#include "config.h"
#include "board.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_LEN 128

/* ■■ Strip leading/trailing whitespace ■■■■■■■■■■■■■■■■■■■■■■■■■ */
static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++; /* Skip leading spaces */
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

/* ■■ Parse one config line ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
static bool parse_line(Board *b, const char *line, int line_num) {
    char keyword[16];
    int v1, v2;

    /*
    * sscanf with %15s prevents buffer overflow on the keyword.
    * We expect exactly 3 tokens: keyword, int, int.
    * Lines with fewer tokens are silently skipped.
    */
    int n = sscanf(line, "%15s %d %d", keyword, &v1, &v2);
    if (n != 3) return true; /* Not an error — might be a comment/blank */

    /* Convert keyword to uppercase for case-insensitive comparison */
    for (int i = 0; keyword[i]; i++)
        keyword[i] = (char)toupper((unsigned char)keyword[i]);

    if (strcmp(keyword, "SNAKE") == 0) {
        if (!board_add_snake(b, v1, v2)) {
            fprintf(stderr, "Config line %d: invalid snake %d->%d\n",
                    line_num, v1, v2);
            return false;
        }
    } else if (strcmp(keyword, "LADDER") == 0) {
        if (!board_add_ladder(b, v1, v2)) {
            fprintf(stderr, "Config line %d: invalid ladder %d->%d\n",
                    line_num, v1, v2);
            return false;
        }
    } else {
        fprintf(stderr, "Config line %d: unknown keyword '%s'\n",
                line_num, keyword);
        return false;
    }

    return true;
}

/* ■■ Public API ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
bool config_load(Board *b, const char *filepath) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open config: %s\n", filepath);
        return false;
    }

    /* Reset board — start fresh, no defaults */
    memset(b->snakes, 0, sizeof(b->snakes));
    memset(b->ladders, 0, sizeof(b->ladders));
    b->num_snakes = 0;
    b->num_ladders = 0;

    char line[MAX_LINE_LEN];
    int line_num = 0;
    bool had_error = false;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        char *trimmed = trim(line);

        /* Skip empty lines and comments */
        if (trimmed[0] == '\0' || trimmed[0] == '#') continue;

        if (!parse_line(b, trimmed, line_num)) {
            had_error = true;
            /* Continue parsing — collect all errors before giving up */
        }
    }

    fclose(fp);

    if (had_error) {
        fprintf(stderr, "Config file had errors — using defaults\n");
        board_init_defaults(b);
        return false;
    }

    /* Rebuild the O(1) lookup tables from the loaded data */
    board_rebuild_lookup(b);

    /* Validate: no cell is both snake and ladder */
    if (!board_validate(b)) {
        fprintf(stderr, "Config validation failed — using defaults\n");
        board_init_defaults(b);
        return false;
    }

    printf(" Board loaded: %d snakes, %d ladders from %s\n",
            b->num_snakes, b->num_ladders, filepath);
    return true;
}
