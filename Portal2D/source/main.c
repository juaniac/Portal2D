#include <nds.h>
#include <stdio.h>
#include <math.h>
#include "game_state_management.h"

int main(void) {

    init_startup();

	while(1)
        swiWaitForVBlank();
}
