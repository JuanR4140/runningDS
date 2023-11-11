#include <nds.h>
#include <nds/arm9/sprite.h>

void game(char* name, int choice, int player_lives, int player_coins, bool FATEnabled);

typedef struct debounce_obj {
    int value;
    int max;
    bool shouldRun;
    bool isDone;
} debounce_obj;

typedef struct Sprite {
    SpriteEntry* gfx;
    u16 x;
    u16 y;
    u8 width;
    u8 height;
    u8 xvel;
    u8 yvel;
    u8 grav;

    u8 lives;
    int coins;
} Sprite;