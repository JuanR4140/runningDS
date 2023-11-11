#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <nds.h>
#include <nds/arm9/console.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sound.h>

#include "talk_bin.h"

void sleepFrames(int seconds){
    int frames = 60 * seconds;
    for(int i = 0; i < frames; i++){
        swiWaitForVBlank();
    }
}

int getSleepFrames(int seconds){
    return 60 * seconds;
}

void setXY(PrintConsole* text, int x, int y){
    text->cursorX = x;
    text->cursorY = y;
}

int encrypt(int value, int key, int IV){
    return ((IV + key) * value);
}

int decrypt(int encrypted_value, int key, int IV){
    int decrypted_value = encrypted_value / (IV + key);
    if(encrypted_value % (IV + key) == 0){
        return decrypted_value;
    }else{
        return -1;
    }
}

void typewrite(char* text, int secondsToSleep){
    consoleClear();
    iprintf("\x1b[%d;%dH", 5, 0);
    while(*text != '\0'){
        scanKeys();
        printf("%c", *text);
        soundPlaySample(talk_bin, SoundFormat_8Bit, talk_bin_size, 16000, 127, 64, false, 0);
        text++;
        int max = 5;
        if(keysHeld() & KEY_B) max = 2;
        for(int i = 0; i < max; i++) swiWaitForVBlank();
    }
    sleepFrames(secondsToSleep);
}
