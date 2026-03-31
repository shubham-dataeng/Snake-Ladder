#ifndef SAVE_H
#define SAVE_H

#include "game.h"
#include <stdbool.h>

/*
 * save.h — Game save/load system
 * Binary format with header validation and checksum
 */

#define SAVE_MAGIC 0x534C4700    /* 'SLG\0' — save file signature */
#define SAVE_DIR "saves/"
#define MAX_SAVES 5

/*
 * SaveHeader: metadata for each save file
 * Used to validate file integrity and version compatibility
 */
typedef struct {
    unsigned int magic;         /* SAVE_MAGIC */
    unsigned int version;       /* GAME_VERSION */
    unsigned int checksum;      /* XOR checksum of GameState bytes */
    unsigned int data_size;     /* sizeof(GameState) */
} SaveHeader;

/* Function prototypes */

/*
 * save_game(gs, slot) — Save game state to binary file
 * Validates slot range, creates saves/ directory if needed
 * Returns: true if successful, false otherwise
 */
bool save_game(const GameState *gs, int slot);

/*
 * load_game(gs, slot) — Load game state from binary file
 * Validates all headers and checksums
 * Returns: true if successful, false otherwise
 */
bool load_game(GameState *gs, int slot);

/*
 * save_exists(slot) — Check if a save slot exists
 * Returns: true if file exists, false otherwise
 */
bool save_exists(int slot);

/*
 * save_list_saves() — Display all save slots (filled/empty)
 */
void save_list_saves(void);

#endif /* SAVE_H */
