#include <stdio.h>
#include <stdbool.h>

#include <nds.h>
#include <nds/arm9/video.h>
#include <nds/arm9/sound.h>
#include <nds/arm9/input.h>
#include <nds/arm9/console.h>

#include "../../assets/backgrounds/bg.h"

#include "title_bin.h"
#include "chime_bin.h"

#include "../utils/misc.h"
#include "../views/mainMenu.h"

void runTitle(bool FATEnabled, bool haveShownWarning){

    vramSetBankA(VRAM_A_MAIN_BG);

    PrintConsole* text = consoleInit(0, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, true, true);

    if(!FATEnabled && !haveShownWarning){
        setXY(text, 12, 2);
        printf("WARNING!");
        setXY(text, 0, 5);
        printf("  This game requires a valid\n  filesystem to read and write\n  game data.\n\n  There was an error\n  initializing the filesystem.\n\n\n  Game data will NOT be saved.\n\n\n\n\n  I UNDERSTAND [Press START]");
        for(;;){
            scanKeys();
            if(keysDown() & KEY_START){ consoleClear(); break; }
            swiWaitForVBlank();
        }
    }

    sleepFrames(1);
    soundPlaySample(chime_bin, SoundFormat_8Bit, chime_bin_size, 16000, 127, 64, false, 0);
    videoSetModeSub(MODE_5_2D);
    sleepFrames(1);
    consoleClear();
    setXY(text, 9, 8);
    iprintf("-JuanR4140-");
    setXY(text, 10, 9);
    iprintf("presents");
    setXY(text, 5, 16);
    iprintf("copyright 2023 (not) ");
    sleepFrames(2);
    consoleClear();
    sleepFrames(1);

    int music = soundPlaySample(title_bin, SoundFormat_8Bit, title_bin_size, 16000, 127, 64, false, 0);
    
    int titleY = 0;
    int versionY = 1;
    int creatorY = 10;
    int musicY = 12;
    for(int i = 0; i < 100; i++){
        consoleClear();
        setXY(text, 8, titleY);
        iprintf("running W.I.P");
        titleY++;
        setXY(text, 11, versionY);
        iprintf("Alpha 2");
        versionY++;
        setXY(text, 4, creatorY);
        iprintf("Code:   JuanR4140");
        creatorY++;
        setXY(text, 4, musicY);
        iprintf("Music:  thepsynergist");
        musicY++;

        if(titleY == 6){
            for(int i = 0; i < 60; i++) swiWaitForVBlank();
            break;
        }

        for(int i = 0; i < 60; i++) swiWaitForVBlank();
    }

    int bg1 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 1, 0);
    dmaCopy(bgBitmap, bgGetGfxPtr(bg1), bgBitmapLen);
    dmaCopy(bgPal, BG_PALETTE, bgPalLen);
    int bg2 = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 1, 0);
    int bg1X = 0;
    int bg2X = 256;
    bgSetScroll(bg1, 0, 0);
    bgSetScroll(bg2, -256, 0);

    bool shouldShow = true;
    int showDuration = 0;
    int showDurationMax = 20;

    for(;;){
        showDuration++;
        if(showDuration > showDurationMax){ shouldShow = !shouldShow; showDuration = 0; }

        setXY(text, 8, 11);
        if(shouldShow){
            iprintf("[Press START]");
        }else{
            iprintf("             ");
        }

        scanKeys();
        if(bg1X < -255){
            bgSetScroll(bg1, -255, 0);
            bg1X = 255;
        }

        if(bg2X < -255){
            bgSetScroll(bg2, -255, 0);
            bg2X = 255;
        }

        bgScroll(bg1, 1, 0);
        bg1X--;
        bgScroll(bg2, 1, 0);
        bg2X--;
        bgUpdate();

        if(keysHeld() & KEY_START){
            soundKill(music);
            consoleClear();
            bgHide(bg1);
            bgHide(bg2);
            vramDefault();
            sleepFrames(2);
            mainMenu(FATEnabled);
            break;
        }

        for(int i = 0; i < 3; i++) swiWaitForVBlank();
    }
}