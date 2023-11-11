/*
    running W.I.P
    V. Alpha 2
    JuanR4140

    Author notes:

        Well I added everything that I wanted to add for Alpha 2.
        I would add more to the game, and I had more planned as well
        (such as dungeons, bosses, and the goal of getting some "triforce".)

        However, school is school and I am also working on some other coding stuffs. :)

        I improved the structure a little from previously, but nothing is perfect.
        Maybe one day I'll return and do some Alpha 3. Beta? GAMMA?

        NOTE: I am reaching the 5MB limit that comes with developing nDS roms. Reason? Audio.
        I wanted to store the audio data in the ROM itself so I wouldn't have to deal with storing them
        in the filesystem, but I may have to end up doing that anyways if I want to continue on this.

        This release is pretty major though, title screen and SAVING DATA as well! Pretty proud I think.

        That is it for now, I do hope someday I can return to this. Hasta luego!

        - juanr4140
        Nov 11, 2023. (My birthday is on the 12th! Wish me a happy birthday!)

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#include <nds/arm9/console.h>
#include <nds/dma.h>

/* SPRITES */
// grit [file].png -gB8 -gt -gT 000000 (transparency)
#include "../../assets/sprites/player.h"
#include "../../assets/sprites/enemy.h"
#include "../../assets/sprites/spider.h"
#include "../../assets/sprites/sword.h"
#include "../../assets/sprites/bow.h"
#include "../../assets/sprites/arrow.h"
#include "../../assets/sprites/coin.h"
#include "../../assets/sprites/heart.h"
#include "../../assets/sprites/grass.h"
#include "../../assets/sprites/platform.h"
#include "../../assets/sprites/chest.h"
#include "../../assets/sprites/cave.h"
#include "../../assets/sprites/exit.h"
#include "../../assets/sprites/spike.h"
#include "../../assets/sprites/pillar.h"

/* BACKGROUNDS */
// grit [file].png -gB8 -gt -gT! (no transparency)
#include "../../assets/backgrounds/bg.h"

/* SOUND */
#include "pickup_bin.h"
#include "jump_bin.h"
#include "slash_bin.h"
#include "hit_bin.h"
#include "overworld_bin.h"
#include "entering_bin.h"
#include "hurt_bin.h"
#include "low_bin.h"

#include "gameRef.h"
#include "game.h"
#include "death.h"
#include "shop.h"
#include "../utils/misc.h"
#include "../utils/collisions.h"

void updateDebounceHandler(debounce_obj* debounce_handler[], int len){
    for(int i = 0; i < len; i++){
        if(debounce_handler[i]->shouldRun){
            debounce_handler[i]->value++;
            debounce_handler[i]->isDone = false;
            if(debounce_handler[i]->value > debounce_handler[i]->max){
                debounce_handler[i]->value = 0;
                debounce_handler[i]->isDone = true;
            }
        }
    }
}

void loadPallete(int slot, const unsigned short source[256]){
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[slot][i] = source[i];
    }
}

void unloadPallete(int slot){
    for(int i = 0; i < 256; i++){
        VRAM_F_EXT_SPR_PALETTE[slot][i] = 0;
    }
}

Sprite* createSprite(int id, OamState* oam, SpriteSize size, SpriteColorFormat color, const void* spriteGFX, int pallete_slot){
    Sprite* sprite = malloc(sizeof(Sprite));

    oamSet(oam, id, 0, 0, 0, pallete_slot, size, color, spriteGFX, -1, false, false, false, false, false);
    sprite->gfx = &oam->oamMemory[id];
    sprite->x = sprite->gfx->x;
    sprite->y = sprite->gfx->y;
    sprite->width = 0;
    sprite->height = 0;
    sprite->xvel = 0;
    sprite->yvel = 0;
    sprite->grav = 0;

    sprite->lives = 0;
    sprite->coins = 0;
    
    return sprite;
}

void setSpriteOpts(Sprite* sprite, u8 width, u8 height, u8 xvel, u8 yvel, u8 grav, u8 lives, u8 coins){
    sprite->width = width;
    sprite->height = height;
    sprite->xvel = xvel;
    sprite->yvel = yvel;
    sprite->grav = grav;
    sprite->lives = lives;
    sprite->coins = coins;
}

void updateLives(int lives, Sprite* hearts[]){
    for(int i = 0; i < 5; i++) hearts[i]->gfx->isHidden = true;

    for(int i = 0; i < lives; i++){
        hearts[i]->gfx->isHidden = false;
        hearts[i]->gfx->x = (i * 16);
    }
}

const int hifreq = 44100; // high fidelity frequency used for tracks
const int lofreq = 16000; //  low fidelity frequency used for tracks

void game(char* name, int choice, int player_lives, int player_coins, bool FATEnabled){
    srand(time(NULL));
    int overworld_track = soundPlaySample(overworld_bin, SoundFormat_8Bit, overworld_bin_size, lofreq, 127, 64, true, 0);

    videoSetMode(MODE_5_2D);
    vramSetBankB(VRAM_B_MAIN_BG_0x06000000);
    int bg1 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 1, 0);
    dmaCopy(bgBitmap, bgGetGfxPtr(bg1), bgBitmapLen);
    dmaCopy(bgPal, BG_PALETTE, bgPalLen);

    
    int bg2 = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 1, 0);
    int bg1X = 0;
    int bg2X = 256;

    vramSetBankA(VRAM_A_MAIN_SPRITE);
    oamInit(&oamMain, SpriteMapping_1D_32, true);
    vramSetBankF(VRAM_F_LCD);

    /* OAM LAYOUT */

    /* PLAYER RELATED*/
    // 0 - Player
    // 1-4 - Weapons
    //   1 - Sword
    //   2 - Bow
    //   3 - Arrow
    //   4 - ???
    // 5-9 Hearts
    
    /* COLLECTABLES */
    // 10-14 - Collectables
    //    10 - Coin
    //    11 - Chest
    //    12 - ???
    //    13-14 ???

    /* PLATFORMS */
    // 15-19 - Grass platforms

    /* MISCELLANEOUS */
    // 20 - Cave/Shop
    // 21 - Exit Cave/Shop
    // 22 - Pillar
    // 23 - Heart for Sale
    // 24 - Bow for sale

    /* 21-49 FREE SPACE */
    
    /* ENEMIES */
    // 50-54 - Basic Enemy ( Follows player, no tricks )
    // 55-59 -  Jump Enemy ( Jumps & Follows player )

    /* HARMFUL OBJECTS */
    // 70-72 - Spikes

    u16* playerGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(playerTiles, playerGFX, playerTilesLen);

    u16* swordGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(swordTiles, swordGFX, swordTilesLen);

    u16* bowGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(bowTiles, bowGFX, bowTilesLen);

    u16* arrowGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(arrowTiles, arrowGFX, arrowTilesLen);

    u16* coinGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(coinTiles, coinGFX, coinTilesLen);

    u16* heartGFX = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
    dmaCopy(heartTiles, heartGFX, heartTilesLen);

    u16* enemyGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(enemyTiles, enemyGFX, enemyTilesLen);

    u16* spiderGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(spiderTiles, spiderGFX, spiderTilesLen);

    u16* grassGFX = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
    dmaCopy(grassTiles, grassGFX, grassTilesLen);

    u16* chestGFX = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
    dmaCopy(chestTiles, chestGFX, chestTilesLen);

    u16* shopGFX = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
    dmaCopy(caveTiles, shopGFX, caveTilesLen);

    u16* exitGFX = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
    dmaCopy(exitTiles, exitGFX, exitTilesLen);

    u16* spikeGFX = oamAllocateGfx(&oamMain, SpriteSize_32x8, SpriteColorFormat_256Color);
    dmaCopy(spikeTiles, spikeGFX, spikeTilesLen);

    const unsigned short* palletes[13] = {
        playerPal,  // We can use the playerPal for the spider pallete; they're both white ;)
        swordPal,
        coinPal,
        heartPal,
        enemyPal,
        grassPal,
        platformPal,
        chestPal,
        cavePal,
        exitPal,
        spikePal,
        bowPal,
        arrowPal
    };
    short palletes_size = sizeof(palletes)/sizeof(palletes[0]);
    // Cleaner and more manageable way to load palletes ;-)
    for(int i = 0; i < palletes_size; i++) loadPallete(i, palletes[i]);
    vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

    Sprite* player = createSprite(0, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, playerGFX, 0);
    setSpriteOpts(player, 16, 16, 0, 0, -1, 3, 0);
    player->gfx->y = 168;
    player->lives = player_lives;
    player->coins = player_coins;
    bool hasBow = false;

    Sprite* sword = createSprite(1, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, swordGFX, 1);
    setSpriteOpts(sword, 16, 16, 0, 0, 0, 0, 0);

    Sprite* bow = createSprite(2, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, bowGFX, 11);
    setSpriteOpts(bow, 16, 16, 0, 0, 0, 0, 0);
    bow->gfx->isHidden = true;

    Sprite* arrow = createSprite(3, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, arrowGFX, 3);
    setSpriteOpts(arrow, 16, 16, 0, 0, 0, 0, 0);
    arrow->gfx->isHidden = true;

    Sprite* hearts[5];
    for(int i = 5; i < 10; i++){
        hearts[i-5] = createSprite(i, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, heartGFX, 3);
        hearts[i-5]->gfx->isHidden = true;
    }
    Sprite* coin = createSprite(10, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, coinGFX, 2);
    setSpriteOpts(coin, 16, 16, 0, 0, 0, 0, 0);
    coin->gfx->isHidden = true;

    Sprite* chest = createSprite(11, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, chestGFX, 7);
    setSpriteOpts(chest, 16, 16, 0, 0, 0, 0, 0);
    chest->gfx->isHidden = true;


    Sprite* enemies[5];
    for(int i = 50; i < 55; i++){
        enemies[i-50] = createSprite(i, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, enemyGFX, 4);
        setSpriteOpts(enemies[i-50], 16, 16, 0, 0, 0, 0, 0);
        enemies[i-50]->gfx->y = 168;
    }
    
    Sprite* spiders[5];
    for(int i = 55; i < 60; i++){
        spiders[i-55] = createSprite(i, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, spiderGFX, 5);
        setSpriteOpts(spiders[i-55], 16, 16, 0, 0, -1, 0, 0);
        spiders[i-55]->yvel = -2;
        spiders[i-55]->gfx->y = 168;
    }

    Sprite* grasses[5];
    for(int i = 15; i < 20; i++){
        grasses[i-15] = createSprite(i, &oamMain, SpriteSize_32x32, SpriteColorFormat_256Color, grassGFX, 5);
        setSpriteOpts(grasses[i-15], 32, 32, 0, 0, 0, 0, 0);
    }

    Sprite* shop = createSprite(20, &oamMain, SpriteSize_32x32, SpriteColorFormat_256Color, shopGFX, 8);
    setSpriteOpts(shop, 32, 32, 0, 0, 0, 0, 0);
    shop->gfx->y = 152;
    shop->gfx->isHidden = true;

    Sprite* exit = createSprite(21, &oamMain, SpriteSize_32x32, SpriteColorFormat_256Color, exitGFX, 9);
    setSpriteOpts(exit, 32, 32, 0, 0, 0, 0, 0);
    exit->gfx->y = 152;
    exit->gfx->x = 0;
    exit->gfx->isHidden = true;

    Sprite* spikes[3];
    for(int i = 70; i < 73; i++){
        spikes[i-70] = createSprite(i, &oamMain, SpriteSize_32x8, SpriteColorFormat_256Color, spikeGFX, 10);
        setSpriteOpts(spikes[i-70], 32, 8, 0, 0, 0, 0, 0);
        spikes[i-70]->gfx->y = 176;
    }

    Sprite* heartShop = createSprite(23, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, heartGFX, 3);
    setSpriteOpts(heartShop, 16, 16, 0, 0, 0, 0, 0);
    heartShop->gfx->y = 168;
    heartShop->gfx->x = 104;
    heartShop->gfx->isHidden = true;

    Sprite* bowShop = createSprite(24, &oamMain, SpriteSize_16x16, SpriteColorFormat_256Color, bowGFX, 11);
    setSpriteOpts(bowShop, 16, 16, 0, 0, 0, 0, 0);
    bowShop->gfx->y = 168;
    bowShop->gfx->x = 136;
    bowShop->gfx->isHidden = true;

    bgSetScroll(bg1, 0, 60);
    bgSetScroll(bg2, -256, 60);

    // I liked the part where I said it's debouncin' time!
    debounce_obj frame_skip = { 0, 3, true, false };
    debounce_obj sword_out  = { 0, 45, false, true };
    debounce_obj bow_out    = { 0, 30, false, true };
    debounce_obj invincible = { 0, 100, false, true };
    debounce_obj low_health = { 0, 30, false, true };

    debounce_obj* debounce_handler[5] = {
        &frame_skip,
        &sword_out,
        &bow_out,
        &invincible,
        &low_health
    };

    consoleDemoInit();
    for(;;){
        updateDebounceHandler(debounce_handler, sizeof(debounce_handler)/sizeof(debounce_handler[0]));

        /* MOVEMENT AND INPUT LOGIC*/
        scanKeys();
        int keys = keysHeld();

        // Pausing :p
        if(keysDown() & KEY_START){
            soundKill(overworld_track);
            for(;;){
                scanKeys();
                if(keysDown() & KEY_START){
                    overworld_track = soundPlaySample(overworld_bin, SoundFormat_8Bit, overworld_bin_size, lofreq, 127, 64, true, 0);
                    break;
                }
                swiWaitForVBlank();
            }
        }

        if(keys & KEY_RIGHT){
            if(player->gfx->x < (128 - 32)){
                player->gfx->x++;
            }else{
                /*for(int i = 0; i < 5; i++) grasses[i]->gfx->x--;
                for(int i = 0; i < 5; i++) enemies[i]->gfx->x--;*/
                for(int i = 0; i < 5; i++){
                    grasses[i]->gfx->x--;
                    enemies[i]->gfx->x--;
                    spiders[i]->gfx->x--;
                }
                for(int i = 0; i < 3; i++) spikes[i]->gfx->x--;
                coin->gfx->x--;
                chest->gfx->x--;
                shop->gfx->x--;

                if(bg1X < -255){ bgSetScroll(bg1, -255, 60); bg1X = 255; }
                if(bg2X < -255){ bgSetScroll(bg2, -255, 60); bg2X = 255; }

                bgScroll(bg1, 1, 0); bg1X--;
                bgScroll(bg2, 1, 0); bg2X--;
            }
            player->gfx->hFlip = false;
        }

        if(keys & KEY_LEFT){
            if(player->gfx->x > 0){
                player->gfx->x--;
            }
            player->gfx->hFlip = true;
        }

        if((keysDown() & KEY_A) && !player->yvel){
            soundPlaySample(jump_bin, SoundFormat_16Bit, jump_bin_size, hifreq, 64, 64, false, 0);
            player->gfx->y -= 5;
            player->yvel = -11;
        }

        if((keysDown() & KEY_B) && (!sword_out.shouldRun) && (!bow_out.shouldRun)){
            sword_out.isDone = false;
            sword_out.shouldRun = true;
            soundPlaySample(slash_bin, SoundFormat_16Bit, slash_bin_size, hifreq, 64, 64, false, 0);
            sword->gfx->isHidden = false;
        }

        if(keysDown() & KEY_X && hasBow && (!bow_out.shouldRun) && (!sword_out.shouldRun) && arrow->gfx->isHidden && player->coins){
            player->coins--;
            bow_out.isDone = false;
            bow_out.shouldRun = true;
            bow->gfx->isHidden = false;
        }

        if(!sword_out.isDone && sword_out.shouldRun){
            sword->gfx->hFlip = player->gfx->hFlip;
            sword->gfx->y = player->gfx->y;
            sword->gfx->x = ( sword->gfx->hFlip ? player->gfx->x - 16 : player->gfx->x + 16 );
        }else{
            sword_out.shouldRun = false;
            sword->gfx->isHidden = true;
            sword->gfx->x = 0;
            sword->gfx->y = 0;
        }

        if(!bow_out.isDone && bow_out.shouldRun){
            bow->gfx->hFlip = player->gfx->hFlip;
            bow->gfx->y = player->gfx->y;
            bow->gfx->x = ( bow->gfx->hFlip ? player->gfx->x - 16 : player->gfx->x + 16 );

            arrow->gfx->isHidden = false;
            arrow->gfx->hFlip = bow->gfx->hFlip;
            arrow->gfx->y = bow->gfx->y;
            arrow->gfx->x = ( bow->gfx->hFlip ? bow->gfx->x - 8 : bow->gfx->x + 8 );
        }else{
            bow_out.shouldRun = false;
            bow->gfx->isHidden = true;
            bow->gfx->x = 0;
            bow->gfx->y = 0;
        }

        if(!arrow->gfx->isHidden){
            arrow->gfx->x += (arrow->gfx->hFlip ? -1 : 1);
        }
        if(arrow->gfx->x < 1 || arrow->gfx->x > 256){
            arrow->gfx->isHidden = true;
            arrow->gfx->x = 0;
            arrow->gfx->y = 0;
        }

        if(invincible.isDone){
            invincible.shouldRun = false;
        }

        if(low_health.isDone){
            low_health.shouldRun = false;
            soundSetVolume(overworld_track, 127);
        }

        /* GAME LOGIC */

        for(int i = 0; i < 5; i++){
            if(grasses[i]->gfx->x < 1){
                int x = ((rand() % 8) * 32) + 256;
                int y = ((rand() % 4) * 32) + 32 + 16;

                grasses[i]->gfx->x = x;
                grasses[i]->gfx->y = y;
            }
            if(onCollision(arrow, grasses[i])){
                arrow->gfx->isHidden = true;
                arrow->gfx->x = 0;
                arrow->gfx->y = 0;
            }
        }

        for(int i = 0; i < 5; i++){
            if(enemies[i]->gfx->x < 1){
                enemies[i]->gfx->x = (rand() % 256) + 256;
                if((rand() % 10) < 3){
                    enemies[i]->gfx->isHidden = false;
                }else{
                    enemies[i]->gfx->isHidden = true;
                }
            }

            if(onCollision(sword, enemies[i]) && !enemies[i]->gfx->isHidden){
                soundPlaySample(hit_bin, SoundFormat_16Bit, hit_bin_size, hifreq, 100, 64, false, 0);
                enemies[i]->gfx->x = -16;
            }

            if(onCollision(arrow, enemies[i]) && !enemies[i]->gfx->isHidden){
                // soundPlaySample(hit_bin, SoundFormat_16Bit, hit_bin_size, hifreq, 100, 64, false, 0);
                enemies[i]->gfx->x = -16;
                arrow->gfx->isHidden = true;
                arrow->gfx->x = 0;
                arrow->gfx->y = 0;
            }

            if(onCollision(player, enemies[i]) && !enemies[i]->gfx->isHidden && (invincible.isDone)){
                soundPlaySample(hurt_bin, SoundFormat_8Bit, hurt_bin_size, 16000, 127, 64, false, 0);
                player->gfx->y -= 20;
                player->yvel = -5;
                player->coins -= 5;
                if(player->coins < 0) player->coins = 0;
                player->lives--;

                // Grant player invincibility!
                invincible.isDone = false;
                invincible.shouldRun = true;

            }


            // spiders
            if(spiders[i]->gfx->x < 1){
                spiders[i]->gfx->x = (rand() % 256) + 256;
                if(((rand() % 10) == 0)){
                    spiders[i]->gfx->isHidden = false;
                }else{
                    spiders[i]->gfx->isHidden = true;
                }
            }
            if(onCollision(sword, spiders[i]) && !spiders[i]->gfx->isHidden){
                soundPlaySample(hit_bin, SoundFormat_16Bit, hit_bin_size, hifreq, 100, 64, false, 0);
                spiders[i]->gfx->x = -16;
            }
            if(onCollision(arrow, spiders[i]) && !spiders[i]->gfx->isHidden){
                // soundPlaySample(hit_bin, SoundFormat_16Bit, hit_bin_size, hifreq, 100, 64, false, 0);
                spiders[i]->gfx->x = -16;
                arrow->gfx->isHidden = true;
                arrow->gfx->x = 0;
                arrow->gfx->y = 0;
            }
            if(onCollision(player, spiders[i]) && !spiders[i]->gfx->isHidden && (invincible.isDone)){
                soundPlaySample(hurt_bin, SoundFormat_8Bit, hurt_bin_size, 16000, 127, 64, false, 0);
                player->gfx->y -= 20;
                player->yvel = -5;
                player->coins -= 5;
                if(player->coins < 0) player->coins = 0;
                player->lives--;

                // Grant player invincibility!
                invincible.isDone = false;
                invincible.shouldRun = true;
            }
        }

        if(coin->gfx->x < 1 || onCollisionAll(coin, grasses, 5) || onCollision(coin, chest)){
            for(int i = 0; i < 5; i++){
                if(grasses[i]->gfx->x > 256){
                    coin->gfx->isHidden = false;
                    coin->gfx->x = grasses[i]->gfx->x + 8;
                    coin->gfx->y = grasses[i]->gfx->y - 16;
                }
            }
        }

        if(onCollision(player, coin) && !coin->gfx->isHidden){
            soundPlaySample(pickup_bin, SoundFormat_16Bit, pickup_bin_size, hifreq, 64, 64, false, 0);
            player->coins++;
            coin->gfx->isHidden = true;
        }

        if(chest->gfx->x < 1 || onCollisionAll(chest, grasses, 5) || onCollision(chest, coin)){
            if((rand() % 100) < 10){
                for(int i = 0; i < 5; i++){
                    if(grasses[i]->gfx->x > 256){
                        chest->gfx->isHidden = false;
                        chest->gfx->x = grasses[i]->gfx->x + 8;
                        chest->gfx->y = grasses[i]->gfx->y - 16;
                    }
                }
            }
        }

        if(onCollision(player, chest) && !chest->gfx->isHidden){
            soundPlaySample(pickup_bin, SoundFormat_16Bit, pickup_bin_size, hifreq, 64, 64, false, 0);
            player->coins += 10;
            chest->gfx->isHidden = true;
        }

        for(int i = 0; i < 3; i++){
            if(spikes[i]->gfx->x < 1){
                spikes[i]->gfx->x = ((rand() % 8) * 32) + 256;
            }

            if(onCollision(player, spikes[i])){
                soundPlaySample(hurt_bin, SoundFormat_8Bit, hurt_bin_size, 16000, 127, 64, false, 0);
                player->gfx->y -= 20;
                player->yvel = -5;
                player->coins--;
                if(player->coins < 0) player->coins = 0;
                player->lives--;

                // Grant player invincibility!
                invincible.isDone = false;
                invincible.shouldRun = true;
            }
        }

        if(shop->gfx->x < 1){
            shop->gfx->x = 475;
            shop->gfx->isHidden = true;
            if(rand() % 5 == 0){
                shop->gfx->isHidden = false;
            }
        }
        if((onCollisionAll(shop, grasses, 5) || onCollisionAll(shop, spikes, 3)) && !shop->gfx->isHidden){
            shop->gfx->x += 64;
        }

        if(onCollision(player, shop) && !shop->gfx->isHidden){
            soundKill(overworld_track);
            soundPlaySample(entering_bin, SoundFormat_8Bit, entering_bin_size, lofreq, 127, 64, false, 0);
            sleepFrames(1);
            shop->gfx->isHidden = true;        
            /*for(int i = 0; i < 5; i++) grasses[i]->gfx->isHidden = true;
            for(int i = 0; i < 5; i++) enemies[i]->gfx->isHidden = true;*/
            for(int i = 0; i < 5; i++){
                grasses[i]->gfx->isHidden = true;
                enemies[i]->gfx->isHidden = true;
                spiders[i]->gfx->isHidden = true;
            }
            for(int i = 0; i < 3; i++) spikes[i]->gfx->isHidden = true;
            sword->gfx->isHidden = true;
            bow->gfx->isHidden = true;
            arrow->gfx->isHidden = true;
            bool prevCoin = coin->gfx->isHidden;
            coin->gfx->isHidden = true;
            bool prevChest = chest->gfx->isHidden;
            chest->gfx->isHidden = true;
            bgHide(bg1);
            bgHide(bg2);
            u8 prevX = player->gfx->x;
            bool prevhFlip = player->gfx->hFlip;
            oamUpdate(&oamMain);

            enterShop(name, player, &updateLives, hearts, &hasBow);

            overworld_track = soundPlaySample(overworld_bin, SoundFormat_8Bit, overworld_bin_size, lofreq, 127, 64, true, 0);
            player->gfx->x = prevX;
            player->gfx->hFlip = prevhFlip;
            bgShow(bg1);
            bgShow(bg2);
            for(int i = 0; i < 5; i++) grasses[i]->gfx->isHidden = false;
            for(int i = 0; i < 3; i++) spikes[i]->gfx->isHidden = false;
            coin->gfx->isHidden = prevCoin;
            chest->gfx->isHidden = prevChest;
            oamUpdate(&oamMain);
        }

        if(frame_skip.isDone){
            frame_skip.isDone = false;
            frame_skip.shouldRun = true;

            player->gfx->y += player->yvel;
            player->yvel -= player->grav;

            if(player->gfx->y >= 168){
                player->gfx->y = 168;
                player->yvel = 0;
            }
            if(player->gfx->y < 16){ // 10
                player->gfx->y = 16;
            }

            for(int i = 0; i < 5; i++){
                if(enemies[i]->gfx->x == player->gfx->x){
                    enemies[i]->gfx->hFlip = enemies[i]->gfx->hFlip;
                }else if(enemies[i]->gfx->x < player->gfx->x){
                    enemies[i]->gfx->x++;
                    enemies[i]->gfx->hFlip = false;
                }else{
                    enemies[i]->gfx->x--;
                    enemies[i]->gfx->hFlip = true;
                }

                for(int j = i + 1; j < 5; j++){
                    if(i == j) continue;

                    if(onCollision(enemies[j], enemies[i]) && !enemies[j]->gfx->isHidden && !enemies[i]->gfx->isHidden){
                        if(enemies[j]->gfx->hFlip) enemies[j]->gfx->x += 2;
                        else enemies[j]->gfx->x -= 2;
                    }

                }

                if(spiders[i]->gfx->x == player->gfx->x){
                    spiders[i]->gfx->hFlip = player->gfx->hFlip;
                }else if(spiders[i]->gfx->x < player->gfx->x){
                    spiders[i]->gfx->x++;
                    spiders[i]->gfx->hFlip = false;
                }else{
                    spiders[i]->gfx->x--;
                    spiders[i]->gfx->hFlip = true;
                }

                spiders[i]->gfx->y += spiders[i]->yvel;
                spiders[i]->yvel -= spiders[i]->grav;
                if(spiders[i]->gfx->y >= 168){
                    spiders[i]->gfx->y = 168;
                    spiders[i]->yvel = -8;
                }

            }

            for(int i = 0; i < 5; i++){
                if(onCollisionOffset(player, grasses[i], 0, 0, 0, -32)){
                    player->gfx->y = grasses[i]->gfx->y-15;
                    player->yvel = 0;
                }else if(onCollision(player, grasses[i])){
                    player->yvel = 5;
                    if(!player->gfx->hFlip && keysHeld() & KEY_RIGHT) player->gfx->x -= 4;
                    if(player->gfx->hFlip && keysHeld() & KEY_LEFT) player->gfx->x += 4;
                }

                for(int j = 0; j < 5; j++){
                    if(onCollision(enemies[j], grasses[i]) && !enemies[j]->gfx->isHidden){
                        if(enemies[j]->gfx->hFlip) enemies[j]->gfx->x += 2;
                        else enemies[j]->gfx->x -= 2;
                    }
                }

            }

        }
        
        updateLives(player->lives, hearts);
        if(player->lives == 1 && !low_health.shouldRun){
            soundSetVolume(overworld_track, 60);
            soundPlaySample(low_bin, SoundFormat_8Bit, low_bin_size, 16000, 127, 64, false, 0);
            low_health.isDone = false;
            low_health.shouldRun = true;
        }
        if(!player->lives){
            soundKill(overworld_track);
            sleepFrames(1);
            consoleClear();
            oamClear(&oamMain, 0, 0);
            bgHide(bg1);
            bgHide(bg2);
            death(name, choice, player, FATEnabled);
        }

        consoleClear();
        printf("%s\nCOINS: %i\n\n\n\n", name, player->coins);
        printf("*******DEBUG INFORMATION********\n\n\n\n");
        printf("Coords: (%i, %i)\n", player->gfx->x, player->gfx->y);
        printf("Sword: Iteration (%i/%i)\n", sword_out.value, sword_out.max);
        printf("isDone: %s\n", ( sword_out.isDone ? "TRUE" : "FALSE" ));
        printf("shouldRun: %s\n\n", ( sword_out.shouldRun ? "TRUE" : "FALSE" ));
        printf("Shop X: %i\n", shop->gfx->x);
        printf("Is Shop Visible? %s\n\n", ( !shop->gfx->isHidden ? "TRUE" : "FALSE" ));
        printf("I: Done: %i SR: %i (%i/%i)\n\n", invincible.isDone, invincible.shouldRun, invincible.value, invincible.max);
        printf("SPIKE1 X: %i\n", spikes[0]->gfx->x);
        printf("Owns bow? %s\n", ( hasBow ? "TRUE" : "FALSE" ));
        bgUpdate();
        swiWaitForVBlank();
        oamUpdate(&oamMain);
    }

}
