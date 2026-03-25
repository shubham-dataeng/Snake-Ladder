#include "utils.h"

int main() {
    seed_rng();
    clear_screen();

    int x = random_int(1, 6);
    printf("Dice roll: %d\n", x);

    pause_for_enter();
    return 0;
}