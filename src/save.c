/*
* save.c — Binary save/load with header validation
* Depends: save.h, game.h, utils.h
*/
#include "save.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> /* mkdir() */
#include <sys/types.h>
#include <stdbool.h>
/* ■■ Ensure saves/ directory exists ■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
static void ensure_save_dir(void) {
struct stat st = {0};
if (stat(SAVE_DIR, &st) == -1) {
mkdir(SAVE_DIR, 0755); /* Create with rwxr-xr-x permissions */
}
}
/* ■■ Build save file path ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
static void get_save_path(char *buf, int slot) {
snprintf(buf, 64, "%ssave_%d.dat", SAVE_DIR, slot);
}
/* ■■ Checksum: XOR all bytes in GameState ■■■■■■■■■■■■■■■■■■■■■■■ */
static unsigned int compute_checksum(const GameState *gs) {
const unsigned char *bytes = (const unsigned char *)gs;
unsigned int cs = 0;
for (size_t i = 0; i < sizeof(GameState); i++)
cs ^= bytes[i];
return cs;
}
/* ■■ Save ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
bool save_game(const GameState *gs, int slot) {
if (slot < 1 || slot > MAX_SAVES) {
fprintf(stderr, "Save slot must be 1-%d\n", MAX_SAVES);
return false;
}
ensure_save_dir();
char path[64];
get_save_path(path, slot);
FILE *fp = fopen(path, "wb");
if (!fp) {
fprintf(stderr, "Cannot open save file: %s\n", path);
return false;
}
/* Build header */
SaveHeader hdr;
hdr.magic = SAVE_MAGIC;
hdr.version = GAME_VERSION;
hdr.checksum = compute_checksum(gs);
hdr.data_size = (unsigned int)sizeof(GameState);
/* Write header */
if (fwrite(&hdr, sizeof(SaveHeader), 1, fp) != 1) {
fprintf(stderr, "Failed to write save header\n");
fclose(fp);
return false;
}
/* Write GameState — the entire struct in one call */
if (fwrite(gs, sizeof(GameState), 1, fp) != 1) {
    fprintf(stderr, "Failed to write game state\n");
fclose(fp);
return false;
}
fclose(fp);
printf(" Game saved to slot %d (%s)\n", slot, path);
return true;
}
/* ■■ Load ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■ */
bool load_game(GameState *gs, int slot) {
if (slot < 1 || slot > MAX_SAVES) {
fprintf(stderr, "Save slot must be 1-%d\n", MAX_SAVES);
return false;
}
char path[64];
get_save_path(path, slot);
FILE *fp = fopen(path, "rb");
if (!fp) {
fprintf(stderr, "Save file not found: %s\n", path);
return false;
}
/* Read and validate header */
SaveHeader hdr;
if (fread(&hdr, sizeof(SaveHeader), 1, fp) != 1) {
fprintf(stderr, "Corrupt save: cannot read header\n");
fclose(fp); return false;
}
if (hdr.magic != SAVE_MAGIC) {
fprintf(stderr, "Invalid save file (wrong magic number)\n");
fclose(fp); return false;
}
if (hdr.version != GAME_VERSION) {
fprintf(stderr, "Incompatible save version "
"(file: %u, game: %d)\n", hdr.version, GAME_VERSION);
fclose(fp); return false;
}
if (hdr.data_size != sizeof(GameState)) {
fprintf(stderr, "Save data size mismatch — "
"struct layout changed?\n");
fclose(fp); return false;
}
/* Read GameState */
if (fread(gs, sizeof(GameState), 1, fp) != 1) {
fprintf(stderr, "Corrupt save: cannot read game state\n");
fclose(fp); return false;
}
fclose(fp);
/* Verify checksum */
unsigned int actual = compute_checksum(gs);
if (actual != hdr.checksum) {
    fprintf(stderr, "Save file corrupted (checksum mismatch)\n");
return false;
}
printf(" Game loaded from slot %d\n", slot);
return true;
}
bool save_exists(int slot) {
char path[64];
get_save_path(path, slot);
FILE *fp = fopen(path, "rb");
if (fp) { fclose(fp); return true; }
return false;
}
void save_list_saves(void) {
printf("\n Saved games:\n");
for (int i = 1; i <= MAX_SAVES; i++) {
if (save_exists(i)) {
printf(" Slot %d: [SAVED]\n", i);
} else {
printf(" Slot %d: [empty]\n", i);
}
}
}