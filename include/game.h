#ifndef GAME_H
#define GAME_H
#include "board.h"
#include "player.h"
/* Game version — used in save file validation (Phase 6) */
#define GAME_VERSION 1
/*
 * GamePhase: the state machine states.
 * WHY a state machine?
 * A game has distinct phases with different valid actions.
 * In PHASE_SETUP, rolling dice is not valid.
 * In PHASE_PLAYING, changing player count is not valid.
 * The state machine enforces these rules automatically.
 */
typedef enum {
    GPHASE_MENU = 0,    /* Main menu — waiting for user to start */
    GPHASE_SETUP = 1,   /* Configuring players and board */
    GPHASE_PLAYING = 2, /* Active gameplay */
    GPHASE_PAUSED = 3,  /* Mid-game save/load */
    GPHASE_OVER = 4,    /* Game finished, show results */
} GamePhase;
/*
 * GameState: the COMPLETE state of a game in progress.
 *
 * DESIGN PRINCIPLE: If you can save this struct to disk and
 * reload it, the game resumes exactly where it left off.
 * That's the goal. Every field that affects gameplay is here.
 *
 * WHY not global variables?
 * Global state is the enemy of modular design. With GameState
 * as a struct, we can:
 * 1. Pass it to any function that needs it
 * 2. Create multiple game instances (e.g. for AI simulation)
 * 3. Serialize it trivially with fwrite()
 * 4. Unit-test functions in isolation
 */
typedef struct {
    Board board;
    Player players[MAX_PLAYERS];
    int num_players;
    int current_player_idx; /* Index into players[] */
    int turn_number;
    GamePhase phase;
    int version; /* For save file validation */
    /* Win tracking */
    int winners[MAX_PLAYERS]; /* players[] indices in win order */
    int num_winners;
    /* Settings */
    bool custom_board; /* true if loaded from config file */
    bool ai_enabled;
    bool analytics_on;
    bool replay_on;
} GameState;
/* Function prototypes */
void game_init(GameState *gs, int num_players);
void game_loop(GameState *gs);
void game_next_turn(GameState *gs);
bool game_is_over(const GameState *gs);
void game_declare_winner(GameState *gs, int player_idx);
Player *game_current_player(GameState *gs);
#endif /* GAME_H */