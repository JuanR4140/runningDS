#include <stdio.h>
#include <stdlib.h>
#include <nds.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>
#include <nds/arm9/console.h>

#include "entering_bin.h"
#include "item_bin.h"

#include "game.h"
#include "../utils/misc.h"
#include "../utils/collisions.h"

void enterShop(char* name, Sprite* player, void(*updateLives)(int, Sprite*[]), Sprite* hearts[], bool* hasBow){
    player->gfx->x = 32;
    player->gfx->y = 168;
    player->gfx->hFlip = false;

    Sprite* exit = malloc(sizeof(Sprite));
    exit->width = 32;
    exit->height = 32;
    exit->gfx = &oamMain.oamMemory[21];
    exit->gfx->isHidden = false;

    /*Sprite* pillar = malloc(sizeof(Sprite));
    pillar->width = 32;
    pillar->height = 64;
    pillar->gfx = &oamMain.oamMemory[22];
    pillar->gfx->isHidden = false;*/

    Sprite* heartShop = malloc(sizeof(Sprite));
    heartShop->width = 16;
    heartShop->height = 16;
    heartShop->gfx = &oamMain.oamMemory[23];
    heartShop->gfx->isHidden = false;

    Sprite* bowShop = malloc(sizeof(Sprite));
    bowShop->width = 16;
    bowShop->height = 16;
    bowShop->gfx = &oamMain.oamMemory[24];
    bowShop->gfx->isHidden = false;

    oamUpdate(&oamMain);

    bool typeOnce = true;
    for(;;){
        consoleClear();
        iprintf("\x1b[%d;%dH", 0, 0);
        printf("%s\nCOINS: %i\n\n\n\n", name, player->coins);
        iprintf("\x1b[%d;%dH", 5, 0);
        if(typeOnce){ typewrite("Welcome to the shop.\nTake a look around.", 1); typeOnce = false; }
        scanKeys();

        if(keysHeld() & KEY_RIGHT){
            if(player->gfx->x < 240) player->gfx->x++;
            player->gfx->hFlip = false;
        }

        if(keysHeld() & KEY_LEFT){
            if(player->gfx->x > 0) player->gfx->x--;
            player->gfx->hFlip = true;
        }

        if(keysHeld() & KEY_A && onCollision(player, heartShop)){
            typewrite("Ah, a heart.\nThat'll be 25 coins.\n", 1);
            printf("[A] - Buy\n[B] - Nah.");
            iprintf("\x1b[%d;%dH", 0, 0);
            printf("%s\nCOINS: %i\n\n\n\n", name, player->coins);
            for(;;){
                scanKeys();
                if(keysHeld() & KEY_B) break;
                if(keysHeld() & KEY_A){
                    if(player->lives > 4){
                        typewrite("5 is enough, no?", 1);
                        break;
                    }
                    if(player->coins > 24){
                        player->coins -= 25;
                        player->lives++;
                        updateLives(player->lives, hearts);
                        oamUpdate(&oamMain);
                        soundPlaySample(item_bin, SoundFormat_8Bit, item_bin_size, 16000, 127, 64, false, 0);
                        typewrite("Nice purchase.", 1);
                        break;
                    }else{
                        typewrite("Oh, you don't have enough coins.", 1);
                        break;
                    }
                }
                swiWaitForVBlank();
            }
        }

        if(keysHeld() & KEY_A && onCollision(player, bowShop)){
            typewrite("Ah, a bow.\nThat'll be 50 coins.\n", 1);
            printf("[A] - Buy\n[B] - Nah.");
            iprintf("\x1b[%d;%dH", 0, 0);
            printf("%s\nCOINS: %i\n\n\n\n", name, player->coins);
            for(;;){
                scanKeys();
                if(keysHeld() & KEY_B) break;
                if(keysHeld() & KEY_A){
                    if(*hasBow){
                        typewrite("You already have a bow.", 1);
                        break;
                    }
                    if(player->coins > 49){
                        player->coins -= 50;
                        *hasBow = true;
                        soundPlaySample(item_bin, SoundFormat_8Bit, item_bin_size, 16000, 127, 64, false, 0);
                        typewrite("Nice purchase.", 1);
                        typewrite("Press [X] to fire the bow.\nEach arrow costs 1 coin.", 2);
                        break;
                    }else{
                        typewrite("Oh, you don't have enough coins.", 1);
                        break;
                    }
                }
                swiWaitForVBlank();
            }
        }

        if(onCollision(player, exit)){
            exit->gfx->isHidden = true;
            heartShop->gfx->isHidden = true;
            bowShop->gfx->isHidden = true;
            free(exit);
            free(heartShop);
            free(bowShop);
            soundPlaySample(entering_bin, SoundFormat_8Bit, entering_bin_size, 16000, 127, 64, false, 0);
            sleepFrames(1);
            break;
        }

        swiWaitForVBlank();
        oamUpdate(&oamMain);
    }
}