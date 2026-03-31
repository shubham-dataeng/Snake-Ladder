/*
* main.c — Program entry point
* Fully integrated: menu, save/load, custom board, game loop
*/
#include <stdio.h>
#include "game.h"
#include "utils.h"
#include "ui.h"
#include "save.h"
#include "config.h"

int main(void) {
    seed_rng();
    
    while (1) {
        ui_show_menu();
        int choice = read_int(" Your choice: ", 1, 5);
        
        switch (choice) {
            case 1: {
                /* New Game */
                clear_screen();
                int num = read_int(" How many players? (2-4): ", 2, 4);
                GameState gs;
                game_init(&gs, num);
                game_loop(&gs);
                printf("\n Thanks for playing!\n\n");
                break;
            }
            case 2: {
                /* Load Game */
                clear_screen();
                printf("\n ■■ LOAD GAME ■■\n\n");
                save_list_saves();
                
                int slot = read_int(" Which slot to load? (1-3, 0=cancel): ", 0, 3);
                if (slot == 0) break;
                
                GameState gs;
                if (load_game(&gs, slot)) {
                    printf(" ✓ Game loaded successfully!\n\n");
                    pause_for_enter();
                    game_loop(&gs);
                    printf("\n Thanks for playing!\n\n");
                } else {
                    printf(" ✗ Failed to load game from slot %d\n\n", slot);
                    pause_for_enter();
                }
                break;
            }
            case 3: {
                /* Custom Board — Load from data/board_config.txt */
                clear_screen();
                printf("\n ■■ CUSTOM BOARD ■■\n\n");
                
                int num = read_int(" How many players? (2-4): ", 2, 4);
                GameState gs;
                game_init(&gs, num);
                
                /* Load custom board layout */
                if (config_load(&gs.board, DEFAULT_CONFIG_PATH)) {
                    printf(" ✓ Custom board loaded!\n\n");
                    pause_for_enter();
                    gs.custom_board = true;
                    game_loop(&gs);
                    printf("\n Thanks for playing!\n\n");
                } else {
                    printf(" ✗ Failed to load custom board (using defaults)\n\n");
                    pause_for_enter();
                }
                break;
            }
            case 4: {
                /* View Rules */
                clear_screen();
                printf("\n ■■ RULES ■■\n\n");
                printf(" • Roll the dice to move forward\n");
                printf(" • Land on a ladder → climb up!\n");
                printf(" • Land on a snake → slide down!\n");
                printf(" • First to reach 100 wins\n");
                printf(" • Overshoot past 100 → stay in place\n\n");
                pause_for_enter();
                break;
            }
            case 5: {
                /* Quit */
                printf("\n Goodbye!\n\n");
                return 0;
            }
        }
    }
    
    return 0;
}