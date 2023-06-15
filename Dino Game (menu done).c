#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 800
#define DINO_WIDTH 100
#define DINO_HEIGHT 100
#define CACTUS_WIDTH 100
#define CACTUS_HEIGHT 100
#define CROW_WIDTH 100
#define WIDTH 1600
#define HEIGHT 800
#define MENU_WIDTH 1600
#define MENU_HEIGHT 800
#define DISTANCE_BETWEEN_OBSTACLES 600 // 增加距離


ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_FONT* font = NULL;
ALLEGRO_BITMAP* background = NULL;
ALLEGRO_DISPLAY *display;
ALLEGRO_BITMAP *dino;
ALLEGRO_BITMAP *cactus;
ALLEGRO_BITMAP* crow;
ALLEGRO_BITMAP *menu;
ALLEGRO_TIMER *timer;
ALLEGRO_EVENT_QUEUE *event_queue;
ALLEGRO_SAMPLE *sample = NULL;
ALLEGRO_SAMPLE_INSTANCE *startSound = NULL;

typedef enum {
    LOW,
    MEDIUM,
    HIGH
} CrowHeight;

CrowHeight getCrowHeight() {
    int random = rand() % 3;
    if (random == 0) {
        return LOW;
    }
    else if (random == 1) {
        return MEDIUM;
    }
    else {
        return HIGH;
    }
}

float getCrowY(CrowHeight height) {
    switch (height) {
    case LOW:
        return SCREEN_HEIGHT - CACTUS_HEIGHT;
    case MEDIUM:
        return SCREEN_HEIGHT - DINO_HEIGHT / 2 - CROW_WIDTH - 50; // 增加高度
    case HIGH:
        return SCREEN_HEIGHT - DINO_HEIGHT - CROW_WIDTH - 100; // 增加高度
    default:
        return 0;
    }
}

float dinoX, dinoY;
float cactusX, cactusY;
float crowX, crowY;
float backgroundX;
float gravity, jump;
bool game_over;

int score = 0;
typedef struct Button {
    int x, y, width, height;
    char text[20];
} Button;

Button startButton, aboutButton, exitButton;

enum GameState {
    MENU,
    ABOUT,
    GAMEPLAY,
};

enum GameState currentState = MENU;

void game_init();
void game_begin();
void game_about();
void game_play();
void game_over1();
void game_destroy();
void handle_mouse_click(int x, int y);

int main(int argc, char* argv[]) {
    game_init();
    game_begin();
    game_destroy();
    return 0;
}

void game_init() {

    if (!al_init()) {
        fprintf(stderr, "Failed to initialize Allegro!\n");
        return;
    }

    if (!al_install_keyboard()) {
        fprintf(stderr, "Failed to install keyboard!\n");
        return;
    }

    if (!al_install_mouse()) {
        fprintf(stderr, "Failed to install mouse!\n");
        return;
    }

    if (!al_init_font_addon()) {
        fprintf(stderr, "Failed to initialize font addon!\n");
        return;
    }

    if (!al_init_ttf_addon()) {
        fprintf(stderr, "Failed to initialize TTF addon!\n");
        return;
    }

    if (!al_init_image_addon()) {
        fprintf(stderr, "Failed to initialize image addon!\n");
        return;
    }

    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Failed to initialize primitives addon!\n");
        return;
    }

    display = al_create_display(MENU_WIDTH, MENU_HEIGHT);
    if (!display) {
        fprintf(stderr, "Failed to create display window!\n");
        return;
    }

    al_set_window_position(display, 0, 0);

    startButton.x = WIDTH / 2 - 50;
    startButton.y = HEIGHT / 2 - 50;
    startButton.width = 100;
    startButton.height = 30;
    strcpy(startButton.text, "Start");

    aboutButton.x = WIDTH / 2 - 50;
    aboutButton.y = HEIGHT / 2 + 10;
    aboutButton.width = 100;
    aboutButton.height = 30;
    strcpy(aboutButton.text, "About");

    exitButton.x = WIDTH / 2 - 50;
    exitButton.y = HEIGHT / 2 + 70;
    exitButton.width = 100;
    exitButton.height = 30;
    strcpy(exitButton.text, "Exit");

    font = al_load_ttf_font("pirulen.ttf", 20, 0);
    if (!font) {
        fprintf(stderr, "Failed to load font!\n");
        return;
    }

    menu = al_load_bitmap("menu.jpg");
    if (!menu) {
        fprintf(stderr, "Failed to load menu image!\n");
        return;
    }

    event_queue = al_create_event_queue();
    if (!event_queue) {
        fprintf(stderr, "Failed to create event queue!\n");
        return;
    }

}

void game_begin() {
    ALLEGRO_EVENT_QUEUE* eventQueue = al_create_event_queue();
    al_register_event_source(eventQueue, al_get_display_event_source(display));
    al_register_event_source(eventQueue, al_get_mouse_event_source());

    while (1) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(eventQueue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            int mouseX = ev.mouse.x;
            int mouseY = ev.mouse.y;

            if (currentState == MENU) {
                     al_init();
                     al_install_audio();
                     al_init_acodec_addon();
                     al_reserve_samples(1);
                     sample = al_load_sample("background_music.wav");


                if (mouseX >= startButton.x && mouseX <= startButton.x + startButton.width &&
                    mouseY >= startButton.y && mouseY <= startButton.y + startButton.height) {
                    currentState = GAMEPLAY;
                }
                else if (mouseX >= aboutButton.x && mouseX <= aboutButton.x + aboutButton.width &&
                    mouseY >= aboutButton.y && mouseY <= aboutButton.y + aboutButton.height) {
                    currentState = ABOUT;
                }
                else if (mouseX >= exitButton.x && mouseX <= exitButton.x + exitButton.width &&
                    mouseY >= exitButton.y && mouseY <= exitButton.y + exitButton.height) {
                    break;
                }
            }
            else if (currentState == ABOUT || currentState == GAMEPLAY) {
                if (mouseX >= exitButton.x && mouseX <= exitButton.x + exitButton.width &&
                    mouseY >= exitButton.y && mouseY <= exitButton.y + exitButton.height) {
                    currentState = MENU;
                }
            }
            startSound = al_create_sample_instance(sample);
            al_set_sample_instance_playmode(startSound, ALLEGRO_PLAYMODE_LOOP);

            // Attach sample instance to the default mixer
           al_attach_sample_instance_to_mixer(startSound, al_get_default_mixer());

          // Play background music
          al_play_sample_instance(startSound);
          al_rest(5.0);

          // Stop playing background music
          al_stop_sample_instance(startSound);
        }

        al_clear_to_color(al_map_rgb(0, 0, 0));

        if (currentState == MENU) {
            al_draw_scaled_bitmap(menu, 0, 0, al_get_bitmap_width(menu), al_get_bitmap_height(menu), 0, 0, MENU_WIDTH, MENU_HEIGHT, 0);


            al_draw_text(font, al_map_rgb(255, 0, 0), WIDTH / 2, HEIGHT / 2 - al_get_font_line_height(font) / 2 - 80, ALLEGRO_ALIGN_CENTER, "Menu");

            al_draw_filled_rectangle(startButton.x, startButton.y, startButton.x + startButton.width, startButton.y + startButton.height, al_map_rgb(255, 255, 255));
            al_draw_text(font, al_map_rgb(0, 0, 0), startButton.x + startButton.width / 2, startButton.y + (startButton.height - al_get_font_line_height(font)) / 2, ALLEGRO_ALIGN_CENTER, startButton.text);

            al_draw_filled_rectangle(aboutButton.x, aboutButton.y, aboutButton.x + aboutButton.width, aboutButton.y + aboutButton.height, al_map_rgb(255, 255, 255));
            al_draw_text(font, al_map_rgb(0, 0, 0), aboutButton.x + aboutButton.width / 2, aboutButton.y + (aboutButton.height - al_get_font_line_height(font)) / 2, ALLEGRO_ALIGN_CENTER, aboutButton.text);

            al_draw_filled_rectangle(exitButton.x, exitButton.y, exitButton.x + exitButton.width, exitButton.y + exitButton.height, al_map_rgb(255, 255, 255));
            al_draw_text(font, al_map_rgb(0, 0, 0), exitButton.x + exitButton.width / 2, exitButton.y + (exitButton.height - al_get_font_line_height(font)) / 2, ALLEGRO_ALIGN_CENTER, exitButton.text);
        }
        else if (currentState == ABOUT) {
            al_draw_scaled_bitmap(menu, 0, 0, al_get_bitmap_width(menu), al_get_bitmap_height(menu), 0, 0, MENU_WIDTH, MENU_HEIGHT, 0);

            al_draw_text(font, al_map_rgb(255, 0, 0), WIDTH / 2, HEIGHT / 2 - al_get_font_line_height(font) / 2 - 80, ALLEGRO_ALIGN_CENTER, "About");

            al_draw_filled_rectangle(exitButton.x, exitButton.y, exitButton.x + exitButton.width, exitButton.y + exitButton.height, al_map_rgb(255, 255, 255));
            al_draw_text(font, al_map_rgb(0, 0, 0), exitButton.x + exitButton.width / 2, exitButton.y + (exitButton.height - al_get_font_line_height(font)) / 2, ALLEGRO_ALIGN_CENTER, exitButton.text);
        }
        else if (currentState == GAMEPLAY) {
        int countdown = 3;  
    int count = 3;
            while (countdown > 0) {
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 0, 0), WIDTH / 2, HEIGHT / 2 - al_get_font_line_height(font) / 2 - 80, ALLEGRO_ALIGN_CENTER, "%d", countdown);
                al_flip_display();
                al_rest(1.0); 
                countdown--;
            }
            while (count>0){
            al_draw_text(font, al_map_rgb(255, 0, 0), WIDTH / 2, HEIGHT / 2 - al_get_font_line_height(font) / 2 - 80, ALLEGRO_ALIGN_CENTER, "Gameplay");
            count--;}
            al_draw_filled_rectangle(exitButton.x, exitButton.y, exitButton.x + exitButton.width, exitButton.y + exitButton.height, al_map_rgb(255, 255, 255));
            al_draw_text(font, al_map_rgb(0, 0, 0), exitButton.x + exitButton.width / 2, exitButton.y + (exitButton.height - al_get_font_line_height(font)) / 2, ALLEGRO_ALIGN_CENTER, exitButton.text);\
            timer = al_create_timer(3.0);
            al_register_event_source(eventQueue, al_get_timer_event_source(timer));
            al_rest(1.0);
            game_play();
        }

        al_flip_display();

    }

    al_destroy_event_queue(eventQueue);
}

void drawGame() {
    // 繪製背景
    al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
        backgroundX, 0, al_get_bitmap_width(background), SCREEN_HEIGHT, 0);

    // 繪製背景的副本以實現循環重複
    al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
        backgroundX + al_get_bitmap_width(background), 0, al_get_bitmap_width(background), SCREEN_HEIGHT, 0);

    // 繪製恐龍圖像
    al_draw_scaled_bitmap(dino, 0, 0, al_get_bitmap_width(dino), al_get_bitmap_height(dino),
        dinoX, dinoY, DINO_WIDTH, DINO_HEIGHT, 0);

    // 繪製仙人掌圖像
    al_draw_scaled_bitmap(cactus, 0, 0, al_get_bitmap_width(cactus), al_get_bitmap_height(cactus),
        cactusX, cactusY, CACTUS_WIDTH, CACTUS_HEIGHT, 0);
        al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH - 10, 10, ALLEGRO_ALIGN_RIGHT, "Score: %d", score);//����code1��

    // 繪製烏鴉圖像
    al_draw_scaled_bitmap(crow, 0, 0, al_get_bitmap_width(crow), al_get_bitmap_height(crow),
        crowX, crowY, CROW_WIDTH, CACTUS_HEIGHT, 0);

    al_flip_display();
}

void game_play(){
    al_init_image_addon();

    al_install_keyboard();

    dino = al_load_bitmap("dino.bmp");
    cactus = al_load_bitmap("cactus.bmp");
    crow = al_load_bitmap("crow.bmp");
    background = al_load_bitmap("background_1.bmp");

    al_convert_mask_to_alpha(dino, al_map_rgb(255, 255, 255)); // 設置透明色為白色
    al_convert_mask_to_alpha(cactus, al_map_rgb(255, 255, 255)); // 設置透明色為白色
    al_convert_mask_to_alpha(crow, al_map_rgb(255, 255, 255)); // 設置透明色為白色

    dinoX = 50;
    dinoY = SCREEN_HEIGHT - DINO_HEIGHT;
    cactusX = SCREEN_WIDTH + rand() % DISTANCE_BETWEEN_OBSTACLES;
    cactusY = SCREEN_HEIGHT - CACTUS_HEIGHT;
    crowX = SCREEN_WIDTH + DISTANCE_BETWEEN_OBSTACLES + rand() % DISTANCE_BETWEEN_OBSTACLES;
    CrowHeight crowHeight = getCrowHeight();
    crowY = getCrowY(crowHeight);
    backgroundX = 0;
    gravity = 0.5;
    jump = 0;
    game_over = false;

    timer = al_create_timer(1.0 / 60);
    event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_start_timer(timer);

    srand(time(NULL));//

    while (!game_over) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            updateGame();
            drawGame();
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                if (dinoY == SCREEN_HEIGHT - DINO_HEIGHT) {
                    jump = 20; // 調整跳躍的力度
                }
            }
        }

        if (dinoY <= SCREEN_HEIGHT - DINO_HEIGHT) {
            dinoY -= jump;
            jump -= gravity;
        }
        else {
            dinoY = SCREEN_HEIGHT - DINO_HEIGHT;
            jump = 0;
        }

        if (cactusX <= dinoX + DINO_WIDTH && cactusX + CACTUS_WIDTH >= dinoX && cactusY <= dinoY + DINO_HEIGHT && cactusY + CACTUS_HEIGHT >= dinoY) {
            game_over = true;
        }

        if (crowX <= dinoX + DINO_WIDTH && crowX + CROW_WIDTH >= dinoX && crowY <= dinoY + DINO_HEIGHT && crowY + CACTUS_HEIGHT >= dinoY) {
            game_over = true;
        }
    }

    game_over1();

    al_destroy_bitmap(dino);
    al_destroy_bitmap(cactus);
    al_destroy_bitmap(crow);
    al_destroy_bitmap(background);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);
}

void game_over1() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_text(font, al_map_rgb(255, 0, 0), WIDTH / 2, HEIGHT / 2 - al_get_font_line_height(font) / 2 - 80, ALLEGRO_ALIGN_CENTER, "Game Over");
    al_flip_display();
    al_rest(2.0);  // Display "Game Over" for 2 seconds before exiting
}

void updateGame() {
    // 移動仙人掌
    cactusX -= 8;

    // 當仙人掌超出屏幕時，重置其位置
    if (cactusX + CACTUS_WIDTH < 0) {
        cactusX = SCREEN_WIDTH + rand() % DISTANCE_BETWEEN_OBSTACLES;
        cactusY = SCREEN_HEIGHT - CACTUS_HEIGHT;
    }

    // 移動烏鴉
    crowX -= 8;

    // 當烏鴉超出屏幕時，重置其位置並重新設定高度
    if (crowX + CROW_WIDTH < 0) {
        crowX = SCREEN_WIDTH + rand() % DISTANCE_BETWEEN_OBSTACLES;
        CrowHeight crowHeight = getCrowHeight();
        crowY = getCrowY(crowHeight);
    }

    // 移動背景
    backgroundX -= 5;

    // 當背景移出屏幕時，重置其位置
    if (backgroundX <= -al_get_bitmap_width(background)) {
        backgroundX = 0;
    }

    if (cactusX + CACTUS_WIDTH < 0) {
    score++;
    }
}

void game_destroy() {
    al_destroy_display(display);
    al_destroy_font(font);
    al_destroy_bitmap(background);
    al_destroy_sample_instance(startSound);
    al_destroy_sample(sample);
}

