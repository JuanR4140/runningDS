#include <stdio.h>
#include <stdbool.h>

#include <nds.h>
#include <nds/arm9/video.h>
#include <nds/arm9/sound.h>
#include <nds/arm9/input.h>
#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <fat.h>

#include "assets/fonts/font.h"

#include "code/views/mainMenu.h"
#include "code/views/game.h"
#include "code/views/death.h"
#include "code/utils/misc.h"
#include "code/views/titleScreen.h"

void initializeSystem(){
    soundEnable();
    videoSetMode(MODE_5_2D);
}

bool initializeFAT(){
    return fatInitDefault();
}

int main(){
    initializeSystem();
    bool FATEnabled = initializeFAT();
    runTitle(FATEnabled, false);
    mainMenu(FATEnabled); // <-- Code is handled by function calling from here on out
    // game("PLAYER", 0, 3, 50, !FATEnabled);
    return 0;
}
