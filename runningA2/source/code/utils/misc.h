#include <nds/arm9/console.h>

void sleepFrames(int seconds);
int getSleepFrames(int seconds);
void setXY(PrintConsole* text, int x, int y);

int encrypt(int value, int key, int IV);
int decrypt(int encrypted_value, int key, int IV); 

void typewrite(char* text, int secondsToSleep);