/*
    running W.I.P
    V. Alpha 2
    JuanR4140

    Author notes:

        TBD

*/

#include <stdio.h>
#include <time.h>
#include <nds.h>
#include <filesystem.h>
#include <gl2d.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/input.h>
#include <nds/arm9/video.h>
#include <nds/arm9/sound.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include <nds/dma.h>

/* SPRITES */
// grit [file].png -gB8 -gt -gT 000000 (transparency)
#include "../../assets/sprites/player.h"
#include "../../assets/sprites/enemy.h"
#include "../../assets/sprites/sword.h"
#include "../../assets/sprites/coin.h"
#include "../../assets/sprites/heart.h"
#include "../../assets/sprites/grass.h"
#include "../../assets/sprites/platform.h"

/* BACKGROUNDS */
// grit [file].png -gB8 -gt -gT! (no transparency)
#include "../../assets/backgrounds/bg.h"

/* SOUND */
#include "pickup_bin.h"
#include "jump_bin.h"
#include "slash_bin.h"
#include "hit_bin.h"
#include "overworld_bin.h"

#include "gameRef.h"

int onCollisionf(SpriteEntry* sprite1, int sprite1W, int sprite1H, SpriteEntry* sprite2, int sprite2W, int sprite2H){
    return ( sprite1->x < sprite2->x + sprite2W && 
             sprite1->x + sprite1W > sprite2->x &&
             sprite1->y < sprite2->y + sprite2H &&
             sprite1H + sprite1->y > sprite2->y );
}

void updateLivesf(int lives, SpriteEntry* hearts[]){
    for(int i = 0; i < 3; i++) hearts[i]->isHidden = true;

    for(int i = 0; i < lives; i++){
        hearts[i]->isHidden = false;
        hearts[i]->x = (i * 32);
    }
}

const int hifreqf = 44100; // high fidelity frequency used for tracks
const int lofreqf = 16000; //  low fidelity frequency used for tracks

void gameRef(){
    srand(time(NULL));
    char* FILE = "nitro:/runningA2.nds";
    char** NAME = &FILE;
    nitroFSInit(NAME);
    soundEnable();

    soundPlaySample(overworld_bin, SoundFormat_8Bit, overworld_bin_size, 16000, 127, 64, true, 0);

    videoSetMode(MODE_5_2D);
    vramSetBankB(VRAM_B_MAIN_BG_0x06000000);
    int bg1 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
    dmaCopy(bgBitmap, bgGetGfxPtr(bg1), bgBitmapLen);
    dmaCopy(bgPal, BG_PALETTE, bgPalLen);

    int bg2 = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
    int bg1X = 0;
    int bg2X = 256;

    // OAM
    /*// 0-4 - Reserved for player & collectibles*/
    // 0 - Reserved for player
    // 1 - Reserved for sword
    // 2 - Reserved for coin
    // 5-9 - Reserved for enemies
    // 10-14 - Reserved for grass tiles
    // 15-19 - Reserved for platform tiles
    // 20-23 - Reserved for hearts
    vramSetBankA(VRAM_A_MAIN_SPRITE);
    oamInit(&oamMain, SpriteMapping_1D_32, true);
    vramSetBankF(VRAM_F_LCD);
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[0][i] = playerPal[i];
    }
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[1][i] = swordPal[i];
    }
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[2][i] = coinPal[i];
    }
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[3][i] = heartPal[i];
    }
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[4][i] = enemyPal[i];
    }
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[5][i] = grassPal[i];
    }
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[6][i] = platformPal[i];
    }
    vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
    u16* playerGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(playerTiles, playerGFX, playerTilesLen);
    oamSet(&oamMain, 0, 50, 50, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, playerGFX, -1, false, false, false, false, false);
    SpriteEntry* player = &oamMain.oamMemory[0];
    player->y = 168;
    int grav = -1;
    int yvel = 0;
    int coins = 0;
    bool isJumping = false;
    int lives = 3;

    u16* swordGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(swordTiles, swordGFX, swordTilesLen);
    oamSet(&oamMain, 1, 0, 0, 0, 1, SpriteSize_16x16, SpriteColorFormat_256Color, swordGFX, -1, false, false, false, false, false);
    SpriteEntry* sword = &oamMain.oamMemory[1];
    bool isSwordOut = false;
    int swordOutDurationMax = 45;
    int swordOutDuration = 0;

    u16* coinGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(coinTiles, coinGFX, coinTilesLen);
    oamSet(&oamMain, 2, 0, 0, 0, 2, SpriteSize_16x16, SpriteColorFormat_256Color, coinGFX, -1, false, false, false, false, false);
    SpriteEntry* coin = &oamMain.oamMemory[2];
    bool coinCollected = false;
    
    u16* enemyGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(enemyTiles, enemyGFX, enemyTilesLen);
    oamSet(&oamMain, 3, 0, 0, 0, 4, SpriteSize_16x16, SpriteColorFormat_256Color, enemyGFX, -1, false, false, false, false, false);
    SpriteEntry* enemy = &oamMain.oamMemory[3];
    enemy->y = 168;

    SpriteEntry* grasses[5];
    for(int i = 10; i < 15; i++){
        u16* grassGFX = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
        dmaCopy(grassTiles, grassGFX, grassTilesLen);
        oamSet(&oamMain, i, 0, 0, 0, 5, SpriteSize_32x32, SpriteColorFormat_256Color, grassGFX, -1, false, false, false, false, false);
        grasses[i-10] = &oamMain.oamMemory[i];
    }

    SpriteEntry* platforms[5];
    for(int i = 15; i < 20; i++){
        u16* platformGFX = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_256Color);
        dmaCopy(platformTiles, platformGFX, platformTilesLen);
        oamSet(&oamMain, i, 0, 0, 0, 6, SpriteSize_64x32, SpriteColorFormat_256Color, platformGFX, -1, false, false, false, false, false);
        platforms[i-15] = &oamMain.oamMemory[i];
        platforms[i-15]->isHidden = true;
    }

    SpriteEntry* hearts[3];
    for(int i = 20; i < 23; i++){
        u16* heartGFX = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
        dmaCopy(heartTiles, heartGFX, heartTilesLen);
        oamSet(&oamMain, i, 0, 0, 0, 3, SpriteSize_32x32, SpriteColorFormat_256Color, heartGFX, -1, false, false, false, false, false);
        hearts[i-20] = &oamMain.oamMemory[i];
        hearts[i-20]->isHidden = true;
    }

    bgSetScroll(bg1, 0, 60);
    bgSetScroll(bg2, -256, 60);

    int jump = 3;

    consoleDemoInit();
    while(1){
        if(isSwordOut) swordOutDuration++;
        if(swordOutDuration >= swordOutDurationMax){ isSwordOut = false; swordOutDuration = 0; }

        scanKeys();

        int keys = keysHeld();

        if(keys & KEY_RIGHT){
            if(player->x < 128 - 32){
                player->x++;
            }else{
                for(int i = 0; i < 5; i++) grasses[i]->x--;
                enemy->x--;
                coin->x--;
                if(bg1X < -255){
                    bgSetScroll(bg1, -255, 60);
                    bg1X = 255;
                }

                if(bg2X < -255){
                    bgSetScroll(bg2, -255, 60);
                    bg2X = 255;
                }

                bgScroll(bg1, 1, 0);
                bg1X--;
                bgScroll(bg2, 1, 0);
                bg2X--;


            }
            player->hFlip = false;
        }

        if(keys & KEY_LEFT){
            if(player->x > 0){
                player->x--;
            }
            player->hFlip = true;
        }

        consoleClear();
        printf("COINS: %i\n\n\n\n*******DEBUG INFORMATION********", coins);
        printf("\n\n\n\n");
        printf("YVEL IS %i\n", yvel);
        printf("GRAV IS %i\n", grav);
        printf("YPOS IS %i\n", player->y);
        printf("JUMP IS %i\n\n", jump);
        printf("isJumping IS %s\n", ( isJumping ? "TRUE" : "FALSE" ));
        printf("ENEMY X IS %i\n", enemy->x);
        printf("ENEMY isHidden IS %s\n", ( enemy->isHidden ? "TRUE" : "FALSE" ));
        printf("SWORD DURATION IS %i OUT OF %i\n", swordOutDuration, swordOutDurationMax);
        jump--;
        if((keysDown() & KEY_A) && !isJumping){
            soundPlaySample(jump_bin, SoundFormat_16Bit, jump_bin_size, hifreqf, 64, 64, false, 0);;
            player->y -= 5;
            yvel = -11;
        }
            
        if((keysHeld() & KEY_B) && !isSwordOut){
            soundPlaySample(slash_bin, SoundFormat_16Bit, slash_bin_size, hifreqf, 64, 64, false, 0);
            isSwordOut = true;
            sword->isHidden = false;
        }
        if(isSwordOut){
            sword->hFlip = player->hFlip;
            sword->y = player->y;
            sword->x = ( sword->hFlip ? player->x - 16 : player->x + 16 );
        }else{
            sword->isHidden = true;
            sword->x = 0;
            sword->y = 0;
        }

        for(int i = 0; i < 5; i++){
            if(grasses[i]->x < 1){
                int x = (rand() % (256 - 32)) + 256;
                int y = rand() % (192 - 64) + 32;
                grasses[i]->x = x;
                grasses[i]->y = y;
            }
        }
        if(coin->x < 1){
            coinCollected = false;
            coin->isHidden = false;
            int grass = rand() % 5;
            coin->x = grasses[grass]->x +  8;
            coin->y = grasses[grass]->y - 16;
        }
        if(onCollisionf(player, 16, 16, coin, 16, 16) && !coinCollected){
            soundPlaySample(pickup_bin, SoundFormat_16Bit, pickup_bin_size, hifreqf, 64, 64, false, 0);
            coinCollected = true;
            coins++;
            coin->isHidden = true;
        }
        
        if(enemy->x < 1){
            enemy->x = (rand() % 256) + 256;
            enemy->isHidden = false;
        }
        if(onCollisionf(sword, 16, 16, enemy, 16, 16)){
            soundPlaySample(hit_bin, SoundFormat_16Bit, hit_bin_size, hifreqf, 100, 64, false, 0);
            enemy->x = -16;
        }
        if(onCollisionf(player, 16, 16, enemy, 16, 16)){
            player->x = 50;
            player->y = 50;
            coins -= 5;
            if(coins < 0) coins = 0;
            lives--;
            if(lives < 1) lives = 3;
        }

        if(!jump){
            if(enemy->x < player->x){ enemy->x++; enemy->hFlip = false; }else{ enemy->x--; enemy->hFlip = true; }

            jump = 3;
            player->y += yvel;
            yvel -= grav;
            if(player->y >= 168){
                player->y = 168;
                yvel = 0;
            }

            if(player->y < 10){
                player->y = 5;
                yvel = 5;
            }

            for(int i = 0; i < 5; i++){
                if(onCollisionf(player, 16, 16, grasses[i], 32, 32)){
                    player->y = grasses[i]->y-15;
                    yvel = 0;
                }
                if(onCollisionf(enemy, 16, 16, grasses[i], 32, 32)){
                    enemy->x += 2;
                }
            }

        }
        isJumping = ( yvel ? true : false );

        updateLivesf(lives, hearts);

        bgUpdate();
        swiWaitForVBlank();
        oamUpdate(&oamMain);
    }
}
