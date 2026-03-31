#ifndef CONFIG_H
#define CONFIG_H

#include "board.h"
#include <stdbool.h>

#define DEFAULT_CONFIG_PATH "data/board_config.txt"

/*
 * config_load(b, filepath) → bool
 * Load board configuration from a text file.
 * Returns true if successful, false on error (falls back to defaults).
 */
bool config_load(Board *b, const char *filepath);

#endif
