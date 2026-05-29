#include <conio.h>

#define SCREEN_W 40
#define SCREEN_H 25

// 游戏区域边框
#define GAME_LEFT   1
#define GAME_RIGHT  27
#define GAME_TOP    1
#define GAME_BOTTOM 23

// UI 区域起点
#define UI_LEFT     30

#define PLAYER_CHAR '@'
#define EMPTY_CHAR  ' '
#define BORDER_CHAR '#'

#define FALL_DELAY 12

unsigned char player_x;
unsigned char player_y;

void draw_border(void) {
    unsigned char x, y;

    // 游戏窗口上下边界
    for (x = GAME_LEFT; x <= GAME_RIGHT; x++) {
        gotoxy(x, GAME_TOP);
        cputc(BORDER_CHAR);

        gotoxy(x, GAME_BOTTOM);
        cputc(BORDER_CHAR);
    }

    // 游戏窗口左右边界
    for (y = GAME_TOP; y <= GAME_BOTTOM; y++) {
        gotoxy(GAME_LEFT, y);
        cputc(BORDER_CHAR);

        gotoxy(GAME_RIGHT, y);
        cputc(BORDER_CHAR);
    }
}

void draw_ui(void) {
    gotoxy(UI_LEFT, 2);
    cputs("NS-SHAFT");

    gotoxy(UI_LEFT, 4);
    cputs("SCORE");
    gotoxy(UI_LEFT, 5);
    cputs("0000");

    gotoxy(UI_LEFT, 7);
    cputs("LIFE");
    gotoxy(UI_LEFT, 8);
    cputs("0000");

    gotoxy(UI_LEFT, 10);
    cputs("A LEFT");
    gotoxy(UI_LEFT, 11);
    cputs("D RIGHT");
}

void draw_player(void) {
    gotoxy(player_x, player_y);
    cputc(PLAYER_CHAR);
}

void erase_player(void) {
    gotoxy(player_x, player_y);
    cputc(EMPTY_CHAR);
}

void handle_input(void) {
    char key;

    if (kbhit()) {
        key = cgetc();

        erase_player();

        if (key == 'a' || key == 'A') {
            if (player_x > GAME_LEFT + 1) {
                player_x--;
            }
        }

        if (key == 'd' || key == 'D') {
            if (player_x < GAME_RIGHT - 1) {
                player_x++;
            }
        }

        draw_player();
    }
}

void main(void) {
    clrscr();
    cursor(0);

    // 玩家初始位置：游戏区域中央
    player_x = (GAME_LEFT + GAME_RIGHT) / 2;
    player_y = (GAME_TOP + GAME_BOTTOM) / 2;

    draw_border();
    draw_ui();
    draw_player();

    while (1) {
        handle_input();
    }
}