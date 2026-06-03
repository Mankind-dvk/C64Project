#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <peekpoke.h>
#include <joystick.h>

#define SCREEN_W 40
#define SCREEN_H 25

// 游戏区域边框
#define GAME_LEFT   1
#define GAME_RIGHT  27
#define GAME_TOP    1
#define GAME_BOTTOM 23

// UI 区域起点
#define UI_LEFT     30

// 字符定义
#define PLAYER_CHAR   '@'
#define EMPTY_CHAR    ' '
#define BORDER_CHAR   '#'
#define PLATFORM_CHAR '='


// 数值越大，下落越慢
#define FALL_DELAY 12
#define SCROLL_DELAY 20
// 数值越大，摇杆移动越慢
#define JOY_MOVE_DELAY 8

// 平台数量
#define PLATFORM_COUNT 6

// 平台长度范围
#define PLATFORM_MIN_LEN 5
#define PLATFORM_MAX_LEN 10

unsigned char player_x;
unsigned char player_y;

unsigned char joy_move_counter = 0;
unsigned char fall_counter = 0;
unsigned char scroll_counter = 0;
unsigned int score = 0;
unsigned char life = 3;
unsigned char game_over = 0;

// 平台数据
unsigned char platform_x[PLATFORM_COUNT];  // 平台起始 x 坐标
unsigned char platform_y[PLATFORM_COUNT];  // 平台 y 坐标
unsigned char platform_len[PLATFORM_COUNT]; // 平台长度

// 绘制边框
void draw_border(void) {
    unsigned char x, y;

    for (x = GAME_LEFT; x <= GAME_RIGHT; x++) {
        gotoxy(x, GAME_TOP);
        cputc(BORDER_CHAR);

        gotoxy(x, GAME_BOTTOM);
        cputc(BORDER_CHAR);
    }

    for (y = GAME_TOP; y <= GAME_BOTTOM; y++) {
        gotoxy(GAME_LEFT, y);
        cputc(BORDER_CHAR);

        gotoxy(GAME_RIGHT, y);
        cputc(BORDER_CHAR);
    }
}


// renew score display in UI (called when score changes)
void update_score_ui(void) {
    gotoxy(UI_LEFT, 5);

    // fist use space to clear previous score (in case new score has fewer digits than old score)
    cputs("     ");

    gotoxy(UI_LEFT, 5);
    cprintf("%u", score);
}

void update_life_ui(void) {
    gotoxy(UI_LEFT, 8);

    // clear previous life display
    cputs("     ");

    gotoxy(UI_LEFT, 8);
    cprintf("%u", life);
}

// 绘制右侧 UI
void draw_ui(void) {
    gotoxy(UI_LEFT, 2);
    cputs("NS-SHAFT");

    gotoxy(UI_LEFT, 4);
    cputs("SCORE");

    update_score_ui();

    gotoxy(UI_LEFT, 7);
    cputs("LIFE");

    update_life_ui();
}

// draw a platform at (x, y) with given length
void draw_platform(unsigned char x, unsigned char y, unsigned char length) {
    unsigned char i;

    for (i = 0; i < length; i++) {
        gotoxy(x + i, y); 
        cputc(PLATFORM_CHAR);  // print platform character
    }
}

// erase a platform at (x, y) with given length
void erase_platform(unsigned char x, unsigned char y, unsigned char length) {
    unsigned char i;

    for (i = 0; i < length; i++) {
        gotoxy(x + i, y);
        cputc(EMPTY_CHAR);
    }
}

// generate a random number between min and max (inclusive)
unsigned char random_range(unsigned char min, unsigned char max) {
    return min + (rand() % (max - min + 1));
}

// randomly generate platforms within the game area
void generate_platforms(void) {
    unsigned char i;
    unsigned char max_x;

    for (i = 0; i < PLATFORM_COUNT; i++) {
        platform_len[i] = random_range(PLATFORM_MIN_LEN, PLATFORM_MAX_LEN);

        max_x = GAME_RIGHT - platform_len[i];  // calculate max x to prevent platform from going out of right border

        platform_x[i] = random_range(GAME_LEFT + 1, max_x);
        platform_y[i] = GAME_TOP + 4 + i * 3; // spread platforms vertically with some spacing
    }
}

// draw all platforms
void draw_platforms(void) {
    unsigned char i;

    for (i = 0; i < PLATFORM_COUNT; i++) {
        draw_platform(platform_x[i], platform_y[i], platform_len[i]);
    }
}

// draw the player at current position
void draw_player(void) {
    gotoxy(player_x, player_y);
    cputc(PLAYER_CHAR);
}

// erase the player at current position
void erase_player(void) {
    gotoxy(player_x, player_y);
    cputc(EMPTY_CHAR);
}

// check if there is a platform at (x, y)
unsigned char is_platform_at(unsigned char x, unsigned char y) {
    char ch;

    gotoxy(x, y);
    ch = cpeekc(); //read character at (x, y) without moving cursor

    if (ch == PLATFORM_CHAR) {
        return 1;
    }

    return 0;
}

// check if player is standing on a platform (i.e., there is a platform right below the player)
unsigned char player_on_platform(void) {
    return is_platform_at(player_x, player_y + 1);
}

// move player to new position (new_x, new_y)
void move_player(unsigned char new_x, unsigned char new_y) {
    erase_player();

    player_x = new_x;
    player_y = new_y;

    draw_player();
}

// erase all platforms (used before platforms move up)
void erase_platforms(void) {
    unsigned char i;

    for (i = 0; i < PLATFORM_COUNT; i++) {
        erase_platform(platform_x[i], platform_y[i], platform_len[i]);
    }
}

// move all platforms up by one row
void move_platforms_up(void) {
    unsigned char i;

    for (i = 0; i < PLATFORM_COUNT; i++) {
        if (platform_y[i] > 0) {
            platform_y[i]--;
        }
    }
}

// if a platform reaches or goes past the top border, recycle it to the bottom with new random length and x position
void recycle_platforms(void) {
    unsigned char i;
    unsigned char max_x;

    for (i = 0; i < PLATFORM_COUNT; i++) {

        // if platform is at or above the top border, recycle it
        if (platform_y[i] <= GAME_TOP) {

            // regenerate random length
            platform_len[i] = random_range(PLATFORM_MIN_LEN, PLATFORM_MAX_LEN);

            // account for new length when calculating max x to prevent platform from going out of right border
            max_x = GAME_RIGHT - platform_len[i];

            // regenerate random x position
            platform_x[i] = random_range(GAME_LEFT + 1, max_x);

            // put platform at the bottom
            platform_y[i] = GAME_BOTTOM - 1;
        }
    }
}


/*
// detect player input and move left or right accordingly
void handle_input(void) {
    char key;

    if (kbhit()) {
        key = cgetc();

        if (key == 'a' || key == 'A') {
            if (player_x > GAME_LEFT + 1) {
                move_player(player_x - 1, player_y);
            }
        }

        if (key == 'd' || key == 'D') {
            if (player_x < GAME_RIGHT - 1) {
                move_player(player_x + 1, player_y);
            }
        }
    }
}
*/
// detect player input from joystick and move left or right accordingly
void handle_input(void) {
    unsigned char joy;

    joy_move_counter++;

    if (joy_move_counter < JOY_MOVE_DELAY) {
        return;
    }

    joy_move_counter = 0;

    // C64 usually uses port2 
    joy = joy_read(JOY_2);

    // Joystick left
    if (JOY_LEFT(joy)) {
        if (player_x > GAME_LEFT + 1) {
            move_player(player_x - 1, player_y);
        }
    }

    // Joystick right
    if (JOY_RIGHT(joy)) {
        if (player_x < GAME_RIGHT - 1) {
            move_player(player_x + 1, player_y);
        }
    }
}


// auto move function
// check if player should fall and move down if necessary
void update_fall(void) {
    fall_counter++;

    if (fall_counter >= FALL_DELAY) {
        fall_counter = 0;

        // if player is at or below the bottom border, do not move down
        // (player_y >= GAME_BOTTOM - 1) {
        //    return;
        //}

        // if there is a platform right below the player, do not move down
        if (player_on_platform()) {
            return;
        }

        // otherwise, move player down by one row
        move_player(player_x, player_y + 1);
    }
}

void update_scroll(void) {
    unsigned char standing;

    scroll_counter++;

    if (scroll_counter >= SCROLL_DELAY) {
        scroll_counter = 0;

        // before moving platforms, check if player is standing on a platform
        standing = player_on_platform();

        // erase player before moving platforms to prevent visual glitches
        erase_player();

        // erase platforms before moving them up
        erase_platforms();

        // platforms move up by one row
        move_platforms_up();

        // recycle platforms that have reached the top
        recycle_platforms();

        // if player is standing on a platform, move them up with the platform
        if (standing) {
            //if (player_y > GAME_TOP + 1) {
                player_y--;
            //}
        }

        // redraw borders and UI to prevent them from being overwritten by platforms
        draw_border();
        draw_ui();

        // redraw platforms and player at new positions
        draw_platforms();
        draw_player();
    }
}


// ganme over and reset function
// show game over message
void show_game_over(void) {
    gotoxy(10, 10);
    cputs("GAME OVER");

    gotoxy(5, 13);
    cputs("PRESS R TO RESTART");
}

// Check gameover condition: if player touches top or bottom border, set game_over flag and show game over message
void check_game_over(void) {
    if (player_y <= GAME_TOP + 1) {
        game_over = 1;
    }

    if (player_y >= GAME_BOTTOM - 1) {
        game_over = 1;
    }

    if (game_over) {
        show_game_over();
    }
}

// restart the game by resetting all variables and redrawing everything
void restart_game(void) {
    clrscr();

    game_over = 0;
    fall_counter = 0;
    scroll_counter = 0;

    player_x = (GAME_LEFT + GAME_RIGHT) / 2;
    player_y = 3;

    generate_platforms();

    draw_border();
    draw_ui();
    draw_platforms();
    draw_player();
}


// simple delay function to slow down the main loop (adjust the loop count as needed for desired speed)
void small_delay(void) {
    unsigned int i;

    for (i = 0; i < 300; i++) {
        // simple empty loop for delay
    }
}


void main(void) {
    clrscr();
    cursor(0);
    joy_install(joy_static_stddrv);

    // random seed from memory location 162 (can be changed to any other location that changes over time, such as a timer or joystick input)
    srand(PEEK(162));

    // player starts at the middle of the game area, 3 rows from the top
    player_x = (GAME_LEFT + GAME_RIGHT) / 2;
    player_y = 3;

    generate_platforms();

    draw_border();
    draw_ui();
    draw_platforms();
    draw_player();

while (1) {
    if (!game_over) {
        handle_input();
        update_fall();
        update_scroll();
        check_game_over();
    } else {
        if (kbhit()) {
            char key = cgetc();

            if (key == 'r' || key == 'R') {
                restart_game();
            }
        }
    }

    small_delay();
}
}