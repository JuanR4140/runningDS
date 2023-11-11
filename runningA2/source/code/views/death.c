#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <nds.h>
#include <nds/arm9/console.h>
#include <nds/arm9/sound.h>
#include <nds/arm9/input.h>

#include "game_over_1_bin.h"
#include "game_over_2_bin.h"
#include "cursor_bin.h"

#include "../utils/misc.h"
#include "../views/game.h"
#include "../views/mainMenu.h"

void death(char* name, int choice, Sprite* player, bool FATEnabled){
    srand(time(NULL));
    vramSetBankA(VRAM_A_MAIN_BG);

    PrintConsole* text = consoleInit(0, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, true, true);

    sleepFrames(1);
    soundPlaySample(game_over_1_bin, SoundFormat_8Bit, game_over_1_bin_size, 16000, 127, 64, false, 0);
    videoSetModeSub(MODE_5_2D);
    
    int died = 0;
    for(int i = 0; i < 50; i++){
        consoleClear();
        setXY(text, 7, died);
        iprintf("*** GAME OVER ***");
        died++;
        if(died == 3){
            for(int i = 0; i < 60; i++) swiWaitForVBlank();
            break;
        }

        for(int i = 0; i < 60; i++) swiWaitForVBlank();
    }
    sleepFrames(1);

    int music = soundPlaySample(game_over_2_bin, SoundFormat_8Bit, game_over_2_bin_size, 16000, 127, 64, true, 0);

    int scroll = 0;

    for(;;){
        consoleClear();

        setXY(text, 7, 2);
        printf("*** GAME OVER ***");

        setXY(text, 5, 5);
        printf("CONTINUE");
        setXY(text, 5, 7);
        printf("SAVE GAME");
        setXY(text, 5, 9);
        printf("EXIT");

        scanKeys();

        if(keysDown() & KEY_DOWN){
            soundPlaySample(cursor_bin, SoundFormat_8Bit, cursor_bin_size, 16000, 127, 64, false, 0);
            scroll++;
            if(scroll > 2) scroll = 0;
        }else if(keysDown() & KEY_UP){
            soundPlaySample(cursor_bin, SoundFormat_8Bit, cursor_bin_size, 16000, 127, 64, false, 0);
            scroll--;
            if(scroll < 0) scroll = 2;
        }

        switch(scroll){
            case 0:{
                setXY(text, 2, 5);
                iprintf("->");
                break;
            }
            case 1:{
                setXY(text, 2, 7);
                iprintf("->");
                break;
            }
            case 2:{
                setXY(text, 2, 9);
                iprintf("->");
                break;
            }
        }

        if(keysDown() & KEY_A){

            if(!scroll){
                consoleClear();
                soundKill(music);
                sleepFrames(2);
                game(name, choice, 3, player->coins, FATEnabled);
            }

            if(scroll == 1){
                if(!FATEnabled){
                    setXY(text, 0, 13);
                    printf("***\n\nSAVE FEATURE NOT SUPPORTED\nON THIS DEVICE\n\n***");
                    sleepFrames(2);
                    continue;
                }

                char file_name[32];
                sprintf(file_name, "/runningA2-%i.sav", choice);
                FILE* exists = fopen(file_name, "rb");
                bool confirm = false;
                if(exists != NULL){
                    for(;;){
                        consoleClear();
                        scanKeys();
                        setXY(text, 7, 2);
                        printf("*** GAME OVER ***");

                        setXY(text, 5, 5);
                        printf("OK [PRESS A]");
                        setXY(text, 5, 7);
                        printf("NO [PRESS B]");

                        if(keysDown() & KEY_A){ confirm =  true; break; }
                        if(keysDown() & KEY_B){ confirm = false; break; }

                        setXY(text, 0, 13);
                        printf("***\n\nFILE EXISTS. OK TO OVERWRITE?\n\n***");
                        swiWaitForVBlank();
                    }
                    if(!confirm) continue;
                }
                fclose(exists);

                int IV = (rand() % 500) + 1;
                int encrypted_lives = encrypt(3, 123, IV);
                int encrypted_coins = encrypt(player->coins, 123, IV);
                char encoded[32];
                sprintf(encoded, "%s:%i:%i:%i", name, encrypted_lives, encrypted_coins, IV);
                
                FILE* data = fopen(file_name, "wb");
                fwrite(encoded, 1, sizeof(encoded), data);
                fclose(data);

                setXY(text, 0, 13);
                printf("***\n\nFILE SAVED.\n\n***");
                sleepFrames(2);
                consoleClear();
                soundKill(music);
                sleepFrames(2);
                mainMenu(FATEnabled);

            }

            if(scroll == 2){
                consoleClear();
                soundKill(music);
                sleepFrames(2);
                mainMenu(FATEnabled);
            }

        }

        swiWaitForVBlank();
    }
}
