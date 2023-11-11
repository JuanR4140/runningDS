#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#include <nds.h>
#include <nds/arm9/video.h>
#include <nds/arm9/sound.h>
#include <nds/arm9/input.h>
#include <nds/arm9/console.h>

#include "fairy_bin.h"
#include "cursor_bin.h"

#include "../utils/misc.h"
#include "../views/game.h"
#include "../views/titleScreen.h"

void scanSaves(bool* available){
    for(int i = 0; i < 3; i++){
        available[i] = true;
        char file_name[32];
        sprintf(file_name, "/runningA2-%i.sav", i);
        FILE* exists = fopen(file_name, "rb");
        if(exists == NULL){
            available[i] = false;
        }
        fclose(exists);
    }
}

char* getSaveName(int save){
    // Previous scanSaves confirms existence of file.
    // It is safe to assume save exists.
    char file_name[32];
    sprintf(file_name, "/runningA2-%i.sav", save);
    FILE* data = fopen(file_name, "rb");
    char buffer[32];
    fread(buffer, 32, 1, data);
    fclose(data);

    char* str = calloc(sizeof(char), 16);
    for(int i = 0; i < 10; i++){
        if(buffer[i] == ':') break; // <-- This is not good !!!
        str[i] = buffer[i];
    }
    return str;

}

void createSave(PrintConsole* text, int choice){
    char name[11] = { 0 };
    int name_pos = 0;
    int row = 0;
    int column = 0;
    for(;;){
        consoleClear();
        setXY(text, 0, 0);
        setXY(text, 4, 1);
        printf("*** CHARACTER CREATE ***");
        scanKeys();

        setXY(text, 10, 4);
        printf("%s", name);
        setXY(text, 0, 7);
        printf("   [A] - ENTER CHARACTER\n\n   [B] - DELETE CHARACTER\n\n   [START] - SAVE DATA");

        setXY(text, 3, 15);
        for(int i = 0; i < 13; i++){
            printf("%c ", 65+i);
        }
        setXY(text, 3, 17);
        for(int i = 0; i < 13; i++){
            printf("%c ", 78+i);
        }

        if(keysHeld() & KEY_UP){
            soundPlaySample(cursor_bin, SoundFormat_8Bit, cursor_bin_size, 16000, 127, 64, false, 0);
            column--;
            if(column < 0) column = 1;
            for(int i = 0; i < 5; i++) swiWaitForVBlank();
        }
        if(keysHeld() & KEY_DOWN){
            soundPlaySample(cursor_bin, SoundFormat_8Bit, cursor_bin_size, 16000, 127, 64, false, 0);
            column++;
            if(column > 1) column = 0;
            for(int i = 0; i < 5; i++) swiWaitForVBlank();
        }
        if(keysHeld() & KEY_RIGHT){
            soundPlaySample(cursor_bin, SoundFormat_8Bit, cursor_bin_size, 16000, 127, 64, false, 0);
            row++;
            if(row > 13) row = 0;
            for(int i = 0; i < 5; i++) swiWaitForVBlank();
        }
        if(keysHeld() & KEY_LEFT){
            soundPlaySample(cursor_bin, SoundFormat_8Bit, cursor_bin_size, 16000, 127, 64, false, 0);
            row--;
            if(row < 0) row = 13;
            for(int i = 0; i < 5; i++) swiWaitForVBlank();
        }

        setXY(text, (2 + (row * 2)), ( column ? 17 : 15 ));
        printf(">");

        if(keysDown() & KEY_A){
            if(name_pos < 10){
                char letter = 65 + (row) + ( column ? 13 : 0 );
                name[name_pos] = letter;
                name_pos++;
            }
        }

        if(keysDown() & KEY_B){
            if(name_pos > 0){
                name_pos--;
                name[name_pos] = '\0';
            }
        }

        if(keysDown() & KEY_START){
            if(name_pos){
                int IV = (rand() % 500) + 1;
                int encrypted_lives = encrypt(3, 123, IV);
                int encrypted_coins = encrypt(0, 123, IV);
                char encoded[32];
                sprintf(encoded, "%s:%i:%i:%i", name, encrypted_lives, encrypted_coins, IV);

                char file_name[32];
                sprintf(file_name, "/runningA2-%i.sav", choice);
                FILE* data = fopen(file_name, "wb");
                fwrite(encoded, 1, sizeof(encoded), data);
                fclose(data);
                setXY(text, 7, 20);
                printf("DATA SAVED");
                sleepFrames(2);
                consoleClear();
                break;

            }else{
                setXY(text, 7, 20);
                printf("MUST ENTER NAME");
                sleepFrames(2);
                continue;
            }
        }

        swiWaitForVBlank();
    }
}

void mainMenu(bool FATEnabled){
    srand(time(NULL));

    PrintConsole* text = consoleInit(0, 0, BgType_Text4bpp, BgSize_T_256x256, 4, 0, true, true);
    int menu = soundPlaySample(fairy_bin, SoundFormat_8Bit, fairy_bin_size, 16000, 127, 64, true, 0);

    int choice = 0;
    bool available[3] = { true, true, true };
    if(FATEnabled) scanSaves(available);
    for(;;){
        consoleClear();
        setXY(text, 0, 0);
        setXY(text, 7, 1);
        printf("*** MAIN MENU ***");

        if(!FATEnabled){
            setXY(text, 5, 5);
            printf("[EMPTY]");
            setXY(text, 5, 7);
            printf("[EMPTY]");
            setXY(text, 5, 9);
            printf("[EMPTY]");
        }else{
            setXY(text, 5, 5);
            if(available[0]) printf("%s", getSaveName(0));
            else printf("[EMPTY]");
            setXY(text, 5, 7);
            if(available[1]) printf("%s", getSaveName(1));
            else printf("[EMPTY]");
            setXY(text, 5, 9);
            if(available[2]) printf("%s", getSaveName(2));
            else printf("[EMPTY]");

            setXY(text, 5, 20);
            printf("* [X] - DELETE SAVE");
        }

        scanKeys();

        if(keysDown() & KEY_DOWN){
            soundPlaySample(cursor_bin, SoundFormat_8Bit, cursor_bin_size, 16000, 127, 64, false, 0);
            choice++;
            if(choice > 2) choice = 0;
        }else if(keysDown() & KEY_UP){
            soundPlaySample(cursor_bin, SoundFormat_8Bit, cursor_bin_size, 16000, 127, 64, false, 0);
            choice--;
            if(choice < 0) choice = 2;
        }

        switch(choice){
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

        if(keysDown() & KEY_A && available[choice]){
            if(!FATEnabled){ soundKill(menu); consoleClear(); sleepFrames(2); game("PLAYER", 0, 3, 0, FATEnabled); }

            char file_name[32];
            sprintf(file_name, "/runningA2-%i.sav", choice);
            FILE* exists = fopen(file_name, "rb");
            if(exists == NULL){
                setXY(text, 0, 13);
                printf("***\n\nGAME DATA NOT FOUND\n\n***");
                sleepFrames(2);
                continue;
            }
            fclose(exists);

            FILE* data = fopen(file_name, "rb");
            char buffer[32];
            fread(buffer, 32, 1, data);
            fclose(data);

            char tokens[4][32];
            int i = 0;
            char* token = strtok(buffer, ":");
            while(token != NULL){
                strcpy(tokens[i], token);
                i++;
                token = strtok(NULL, ":");
            }
            
            int user_iv = atoi(tokens[3]);
            int decrypted_lives = decrypt(atoi(tokens[1]), 123, user_iv);
            int decrypted_coins = decrypt(atoi(tokens[2]), 123, user_iv);
            if(decrypted_lives < 0 || decrypted_coins < 0){
                setXY(text, 0, 13);
                printf("***\n\nINVALID GAME DATA.\n\n***");
                sleepFrames(2);
                continue;
            }

            consoleClear();
            soundKill(menu);
            sleepFrames(2);

            game(tokens[0], choice, decrypted_lives, decrypted_coins, FATEnabled);

        }

        if(keysDown() & KEY_A && !available[choice]){
            createSave(text, choice);
            scanSaves(available);
            continue;
        }

        if(keysDown() & KEY_B){
            consoleClear();
            soundKill(menu);
            sleepFrames(2);
            runTitle(FATEnabled, true);
        }

        if(keysDown() & KEY_X && available[choice] && FATEnabled){

            bool confirm = false;
            for(;;){
                consoleClear();
                scanKeys();
                setXY(text, 7, 1);
                printf("*** MAIN MENU ***");

                setXY(text, 5, 5);
                printf("OK [PRESS A]");
                setXY(text, 5, 7);
                printf("NO [PRESS B]");

                if(keysDown() & KEY_A){ confirm =  true; break; }
                if(keysDown() & KEY_B){ confirm = false; break; }

                setXY(text, 0, 13);
                printf("***\n\nARE YOU SURE YOU WANT TO\n\nDELETE THIS SAVE?\n\n***");
                swiWaitForVBlank();
            }
            if(!confirm) continue;

            char file_name[32];
            sprintf(file_name, "/runningA2-%i.sav", choice);
            remove(file_name);
            setXY(text, 0, 13);
            printf("***\n\nFILE DELETED.\n\n***");
            sleepFrames(2);
            consoleClear();
            scanSaves(available);
            continue;
        }

        swiWaitForVBlank();
    }
}