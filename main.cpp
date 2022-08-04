#include <iostream>
#include <ctime>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>

using namespace std;

struct GameParameters
{
    // Definicao de FPS
    const double FPS = 8;
    // Definicao de Altura
    const int SCREEN_H = 700;
    // Definicao de largura
    const int SCREEN_W = 1000;
    // Definicao do tamanho referencia
    const int SIZE_REF = 26;
    // Numeros de samples
    const int SAMPLES = 7;
};

struct Songs
{
    ALLEGRO_SAMPLE *menu;
    ALLEGRO_SAMPLE *playing;
    ALLEGRO_SAMPLE *game_over;

    ALLEGRO_SAMPLE_INSTANCE *menu_instance;
    ALLEGRO_SAMPLE_INSTANCE *playing_instance;
    ALLEGRO_SAMPLE_INSTANCE *game_over_instance;
};

struct Effects
{
    ALLEGRO_SAMPLE *food;
    ALLEGRO_SAMPLE *spoiled_food;
    ALLEGRO_SAMPLE *special;
    ALLEGRO_SAMPLE *lost_game;

    ALLEGRO_SAMPLE_INSTANCE *food_instance;
    ALLEGRO_SAMPLE_INSTANCE *spoiled_food_instance;
    ALLEGRO_SAMPLE_INSTANCE *special_instance;
    ALLEGRO_SAMPLE_INSTANCE *lost_game_instance;
};

GameParameters game_parameters;
Songs songs = {NULL, NULL, NULL, NULL, NULL, NULL};
Effects effects = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

// timer
ALLEGRO_TIMER *timer = NULL;
// display
ALLEGRO_DISPLAY *display = NULL;
// mapa
ALLEGRO_BITMAP *map = NULL;
// cobra partes
ALLEGRO_BITMAP *snake = NULL;
// frutas
ALLEGRO_BITMAP *fruit = NULL;
// menu
ALLEGRO_BITMAP *game_background = NULL;
ALLEGRO_BITMAP *game_menu = NULL;
ALLEGRO_BITMAP *clouds = NULL;
// fila de eventos
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_FONT *game_font = NULL;
ALLEGRO_FONT *gameover_font = NULL;

// direcoes
bool move_up, move_down, move_left, move_right;

// matriz referencia
char MAP_REF[22][30] =
    {
        "11111111111111111111111111111",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "10000000000000000000000000001",
        "11111111111111111111111111111",
};

// contador de posicao atual da cobra
int position = 0;

// frag termina o jogo
bool game_running = false;

// frag redesenha o jogo
bool redraw = true;

// posicao da fruta no mapa
int fruit_line, fruit_collum;

// matriz referencia "onde cobra passou"
int MOVEMENT[22][30] = {0};

// matriz referencia partes da cobra
int INDEX_SNAKE[22][30] = {0};

// tamanho inicial da cobra
int snake_size = 4;

// index da fruta atual
int fruit_index = 0;

// Posição da cobra na matriz
int i = 11, j = 11;

// frag morreu
bool lost_game = false;

// frag menu
bool menu_running = true;

// frag fecha jogo
bool finish_game = false;

// mouse position
int mouse_x, mouse_y;

int points = 0;

bool gameover_running = false;

bool init()
{

    // verifica se e possivel carregar o allegro
    if (!al_init() || !al_init_image_addon() || !al_init_acodec_addon() ||
        !al_init_font_addon() || !al_init_ttf_addon())
    {
        cout << "ERRO! Nao foi possivel carregar os arquivos necessarios.\n";
        return false;
    }

    // verifica se os hardware necessario está disponível.
    if (!al_install_keyboard() || !al_install_audio() || !al_install_mouse())
    {
        cout << "ERRO! O hardware necessario nao esta disponivel.\n";
        return false;
    }

    // reserva samples de audio
    if (!al_reserve_samples(game_parameters.SAMPLES))
    {
        cout << "ERRO! Nao foi possivel reservar samples.\n";
    }

    // Inicializa o timer do jogo.
    timer = al_create_timer(1.0 / game_parameters.FPS);
    if (!timer)
    {
        cout << "ERRO! Nao foi possivel inicializar o temporarizador. \n";
        return false;
    }

    // Inicializa display
    display = al_create_display(game_parameters.SCREEN_W, game_parameters.SCREEN_H);
    if (!display)
    {
        cout << "ERRO! Nao foi possivel inicializar o display.\n";
        al_destroy_timer(timer);
        return false;
    }

    // carrega partes do jogo
    map = al_load_bitmap("./assets/images/map.png");
    snake = al_load_bitmap("./assets/images/snake.png");
    fruit = al_load_bitmap("./assets/images/fruits.png");
    game_background = al_load_bitmap("./assets/images/menu_back.png");
    game_menu = al_load_bitmap("./assets/images/menu.png");
    clouds = al_load_bitmap("./assets/images/clouds.png");

    if (!map || !snake || !fruit || !game_background || !game_menu || !clouds)
    {
        cout << "ERRO! Nao foi possivel carregar assets.\n";
        al_destroy_display(display);
        al_destroy_timer(timer);
        return false;
    }

    // carrega sons
    songs.menu = al_load_sample("./assets/music/background/menu.ogg");
    songs.playing = al_load_sample("./assets/music/background/playing.ogg");
    songs.game_over = al_load_sample("assets/music/background/game_over.ogg");

    // carrega efeitos
    effects.food = al_load_sample("./assets/music/effects/food.ogg");
    effects.special = al_load_sample("./assets/music/effects/food.ogg");
    effects.spoiled_food = al_load_sample("./assets/music/effects/spoiled_food.ogg");
    effects.lost_game = al_load_sample("./assets/music/effects/game_over.ogg");

    if (!songs.menu || !songs.playing || !songs.game_over || !effects.food ||
        !effects.special || !effects.spoiled_food || !effects.lost_game)
    {
        cout << "ERRO! Nao foi possivel carregar sons.\n";

        al_destroy_bitmap(game_background);
        al_destroy_bitmap(game_menu);

        al_destroy_bitmap(fruit);
        al_destroy_bitmap(snake);
        al_destroy_bitmap(map);
        al_destroy_display(display);
        al_destroy_timer(timer);

        return false;
    }

    // cria instancias de sons
    songs.menu_instance = al_create_sample_instance(songs.menu);
    songs.playing_instance = al_create_sample_instance(songs.playing);
    songs.game_over_instance = al_create_sample_instance(songs.game_over);

    // cria instancias de efeitos
    effects.food_instance = al_create_sample_instance(effects.food);
    effects.special_instance = al_create_sample_instance(effects.special);
    effects.spoiled_food_instance = al_create_sample_instance(effects.spoiled_food);
    effects.lost_game_instance = al_create_sample_instance(effects.lost_game);

    if (!effects.food_instance || !effects.special_instance ||
        !effects.spoiled_food_instance || !effects.lost_game_instance ||
        !songs.menu_instance || !songs.playing_instance || !songs.game_over_instance)
    {
        cout << "ERRO! Não foi possivel criar samples instances.\n";

        al_destroy_bitmap(game_background);
        al_destroy_bitmap(game_menu);

        // al_destroy_sample(morder_game_effect);
        // al_destroy_sample(background_menu_music);
        al_destroy_bitmap(fruit);
        al_destroy_bitmap(snake);
        al_destroy_bitmap(map);
        al_destroy_display(display);
        al_destroy_timer(timer);

        return false;
    }

    // Confugurar audios
    al_set_sample_instance_playmode(songs.menu_instance, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_playmode(songs.playing_instance, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_playmode(songs.game_over_instance, ALLEGRO_PLAYMODE_LOOP);

    al_set_sample_instance_playmode(effects.food_instance, ALLEGRO_PLAYMODE_ONCE);
    al_set_sample_instance_playmode(effects.spoiled_food_instance, ALLEGRO_PLAYMODE_ONCE);
    al_set_sample_instance_playmode(effects.special_instance, ALLEGRO_PLAYMODE_ONCE);
    al_set_sample_instance_playmode(effects.lost_game_instance, ALLEGRO_PLAYMODE_ONCE);

    // definir mixador
    al_attach_sample_instance_to_mixer(songs.menu_instance, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(songs.playing_instance, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(songs.game_over_instance, al_get_default_mixer());

    al_attach_sample_instance_to_mixer(effects.food_instance, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(effects.spoiled_food_instance, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(effects.special_instance, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(effects.lost_game_instance, al_get_default_mixer());

    // carrega textos
    game_font = al_load_font("./assets/fonts/game.ttf", 36, 0);
    gameover_font = al_load_font("./assets/fonts/game.ttf", 55, 0);
    if (!game_font || !gameover_font)
    {
        cout << "ERRO! Nao foi possivel carregar fontes\n";
    }
    // Cria a fila de eventos
    event_queue = al_create_event_queue();
    if (!event_queue)
    {
        cout << "ERRO! Nao foi possivel criar a fila de eventos.\n";

        al_destroy_bitmap(game_background);
        al_destroy_bitmap(game_menu);

        /*al_destroy_sample_instance(morder_game_effect_instance);
        al_destroy_sample_instance(background_menu_music_instance);
        al_destroy_sample(morder_game_effect);
        al_destroy_sample(background_menu_music);*/
        al_destroy_bitmap(fruit);
        al_destroy_bitmap(snake);
        al_destroy_bitmap(map);
        al_destroy_display(display);
        al_destroy_timer(timer);

        return false;
    }

    // registra os eventos
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());

    // inicia o timer
    al_start_timer(timer);

    return true;
}

bool init_menu()
{
    // desenha o plano de fundo padrão
    al_draw_bitmap(game_background, 0, 0, 0);
    // Desenha o menu
    al_draw_bitmap(game_menu, (game_parameters.SCREEN_W - 202) * 0.5, (game_parameters.SCREEN_H - 364) * 0.5, 0);
    al_flip_display();

    // inicia menu music
    al_play_sample_instance(songs.menu_instance);

    return true;
}
void generate_fruit()
{
    srand(time(0));

    // sorteia a fruta
    fruit_index = rand() % 3;

    do
    {
        // sorteia aleatoriamente a linha e coluna da fruta
        fruit_line = rand() % 21;
        fruit_collum = rand() % 29;
    } while (MAP_REF[fruit_line][fruit_collum] == '1' || MAP_REF[fruit_line][fruit_collum] == '4');

    MAP_REF[fruit_line][fruit_collum] = '3';
}

bool init_game()
{
    // inicia menu music
    al_play_sample_instance(songs.playing_instance);
    move_up = false;
    move_down = false;
    move_left = false;
    move_right = false;

    position = 0;

    redraw = true;

    snake_size = 4;
    i = 11;
    j = 11;

    for (int i = 0; i < 22; i++)
    {
        for (int j = 0; j < 30; j++)
        {
            // Limpa referenciais da cobra
            if (MAP_REF[i][j] == '4' || MAP_REF[i][j] == '3')
            {
                MAP_REF[i][j] = '0';
            }

            // reinicia matrizes
            MOVEMENT[i][j] = 0;
            INDEX_SNAKE[i][j] = 0;
        }
    }
    // gera a fruta inicial
    generate_fruit();

    return true;
}

void move_snake()
{
    position++;
    MOVEMENT[i][j] = position;

    // verifica se é area livre para andar
    if (move_up && MAP_REF[i][j] != '1')
    {
        i--;
        INDEX_SNAKE[i][j] = 1;
    }
    // pode ser parede ou a propria cobra, morre
    if (move_up && MAP_REF[i][j] == '1')
    {
        lost_game = true;
    }

    // verifica se é area livre para andar
    if (move_down && MAP_REF[i][j] != '1')
    {
        i++;
        INDEX_SNAKE[i][j] = 2;
    }
    // pode ser parede ou a propria cobra, morre
    if (move_down && MAP_REF[i][j] == '1')
    {
        lost_game = true;
    }

    // verifica se é area livre para andar
    if (move_left && MAP_REF[i][j] != '1')
    {
        j--;
        INDEX_SNAKE[i][j] = 3;
    }
    // pode ser parede ou a propria cobra, morre
    if (move_left && MAP_REF[i][j] == '1')
    {
        lost_game = true;
    }

    // verifica se é area livre para andar
    if (move_right && MAP_REF[i][j] != '1')
    {
        j++;
        INDEX_SNAKE[i][j] = 4;
    }
    // pode ser parede ou a propria cobra, morre
    if (move_right && MAP_REF[i][j] == '1')
    {
        lost_game = true;
    }

    // verifica se a cobra pegou fruta
    if ((move_up || move_down || move_right || move_left) && MAP_REF[i][j] == '3')
    {
        // ativa som de mordida
        al_play_sample_instance(effects.food_instance);
        // aumenta tamanho da cobra
        snake_size++;
        points += 1;
        // reseta posicao que a fruta estava
        MAP_REF[fruit_line][fruit_collum] = '0';
        // gera nova fruta
        generate_fruit();
    }

    // verifica se a cobra bateu nela mesma
    if ((move_up || move_down || move_left || move_right) && MAP_REF[i][j] == '4')
    {
        lost_game = true;
    }

    // solicita redesenho de jogo
    redraw = true;
}

void direction_snake(int keycode)
{
    switch (keycode)
    {
    case ALLEGRO_KEY_UP:
        move_up = true;
        move_down = false;
        move_right = false;
        move_left = false;
        break;

    case ALLEGRO_KEY_DOWN:
        move_up = false;
        move_down = true;
        move_right = false;
        move_left = false;
        break;

    case ALLEGRO_KEY_LEFT:
        move_up = false;
        move_down = false;
        move_right = false;
        move_left = true;
        break;

    case ALLEGRO_KEY_RIGHT:
        move_up = false;
        move_down = false;
        move_right = true;
        move_left = false;
        break;
    }
}

void others_keycode(int keycode)
{
    switch (keycode)
    {
    case ALLEGRO_KEY_ESCAPE:
        game_running = false;
        menu_running = false;
        gameover_running = false;
        finish_game = true;
        break;

    case ALLEGRO_KEY_ENTER:
        if (gameover_running)
        {
            gameover_running = false;
            menu_running = false;
            game_running = true;
            lost_game = false;
        }
        break;
    }
}

void check_button_click(int a, int b)
{
    // verifica se é dentro do menu
    if (menu_running && a >= 426 && a <= 571)
    {
        // jogar
        if (b >= 255 && b <= 381)
        {
            menu_running = false;
            game_running = true;
        }
        // opções
        else if (b >= 402 && b <= 443)
        {
        }
        // sobre
        else if (b >= 567 && b <= 507)
        {
        }
    }
}

void check_event(ALLEGRO_EVENT ev)
{
    // verifica qual foi o evento acionado e chama a função correspondente.
    if (ev.type == ALLEGRO_EVENT_TIMER)
    {
        move_snake();
    }

    else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
    {
        direction_snake(ev.keyboard.keycode);
    }

    else if (ev.type == ALLEGRO_EVENT_KEY_UP)
    {
        others_keycode(ev.keyboard.keycode);
    }

    else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
    {
        game_running = false;
        menu_running = false;
        gameover_running = false;
        finish_game = true;
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
    {
        mouse_x = ev.mouse.x;
        mouse_y = ev.mouse.y;
    }
    else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
    {
        check_button_click(mouse_x, mouse_y);
        cout << mouse_x << "  " << mouse_y << endl;
    }
}

void draw_snake(int ii, int jj, int counter)
{
    // curvas
    if (MAP_REF[ii + 1][jj] == '4' && MAP_REF[ii][jj - 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 3, 26 * 0, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (MAP_REF[ii - 1][jj] == '4' && MAP_REF[ii][jj - 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 0, 26 * 0, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (MAP_REF[ii + 1][jj] == '4' && MAP_REF[ii][jj + 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 2, 26 * 2, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (MAP_REF[ii - 1][jj] == '4' && MAP_REF[ii][jj + 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 3, 26 * 2, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    // cabeça
    else if (counter == 0 && INDEX_SNAKE[ii][jj] == 1 && MAP_REF[ii + 1][jj] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 3, 26 * 1, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (counter == snake_size && INDEX_SNAKE[ii][jj] == 2 && MAP_REF[ii - 1][jj] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 4, 26 * 1, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (counter == 0 && INDEX_SNAKE[ii][jj] == 3 && MAP_REF[ii][jj + 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 2, 26 * 1, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (counter == snake_size && INDEX_SNAKE[ii][jj] == 4 && MAP_REF[ii][jj - 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 4, 26 * 0, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    // rabo
    else if (counter == snake_size && INDEX_SNAKE[ii][jj] == 1 && MAP_REF[ii - 1][jj] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 0, 26 * 2, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (counter == 0 && INDEX_SNAKE[ii][jj] == 2 && MAP_REF[ii + 1][jj] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 1, 26 * 1, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (counter == snake_size && INDEX_SNAKE[ii][jj] == 3 && MAP_REF[ii][jj - 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 2, 26 * 0, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (counter == 0 && INDEX_SNAKE[ii][jj] == 4 && MAP_REF[ii][jj + 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 1, 26 * 0, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    // meio
    else if (INDEX_SNAKE[ii][jj] == 1 || INDEX_SNAKE[ii][jj] == 2 && (MAP_REF[ii + 1][jj] == '4' || MAP_REF[ii - 1][jj] == '4') && counter > 0 && counter < snake_size)
    {
        al_draw_bitmap_region(snake, 26 * 0, 26 * 1, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
    else if (INDEX_SNAKE[ii][jj] == 3 || INDEX_SNAKE[ii][jj] == 4 && (MAP_REF[ii][jj + 1] == '4' || MAP_REF[ii][jj - 1] == '4') && counter > 0 && counter < snake_size)
    {
        al_draw_bitmap_region(snake, 26 * 1, 26 * 2, 26, 26, jj * game_parameters.SIZE_REF + 110, ii * game_parameters.SIZE_REF + 92, 0);
    }
}

void redraw_game()
{
    // contador de partes da cobra
    int counter = 0;

    // plano de fundo
    al_draw_bitmap(game_background, 0, 0, 0);

    // desenha o mapa
    al_draw_bitmap(map, (game_parameters.SCREEN_W - 780) * 0.5, (game_parameters.SCREEN_H - 664) * 0.5, 0);

    // desenha a fruta
    al_draw_bitmap_region(fruit, 26 * fruit_index, 0, 26, 26, fruit_collum * game_parameters.SIZE_REF + 115, fruit_line * game_parameters.SIZE_REF + 92, 0);

    // escreve placar

    al_draw_textf(game_font, al_map_rgb(255, 255, 255), 226, 32, 0, "%.3d", points);
    // desenha a cobra
    for (int i = 0; i < 22; i++)
    {
        for (int j = 0; j < 30; j++)
        {
            // Limpa referenciais da cobra
            if (MAP_REF[i][j] == '4')
            {
                MAP_REF[i][j] = '0';
            }

            if (MOVEMENT[i][j] > 0 && MOVEMENT[i][j] >= position - snake_size)
            {
                // adiciona referencia
                MAP_REF[i][j] = '4';
                // Desenha a cobra
                draw_snake(i, j, counter);
                // incrementa contador
                counter++;
            }
        }
    }
}

void is_game_lost()
{
    if (lost_game)
    {
        al_stop_sample_instance(songs.playing_instance);
        al_play_sample_instance(effects.lost_game_instance);
        game_running = false;
        gameover_running = true;
    }
}

void run_game()
{
    // Cria evento local allegro
    ALLEGRO_EVENT ev;

    // aguarda algum evento
    al_wait_for_event(event_queue, &ev);

    // verifica qual evento foi disparado
    check_event(ev);

    // redesenha o jogo
    if (redraw && al_is_event_queue_empty(event_queue))
    {
        redraw_game();
    }

    // envia mudancas ao display
    al_flip_display();

    //  verifica se é o fim do jogo
    is_game_lost();
}

void destroy_game()
{
    al_stop_sample_instance(songs.playing_instance);
}

void draw_clouds()
{
}

void run_menu()
{
    // desenha nuvens de fundo
    draw_clouds();

    // Cria evento local allegro
    ALLEGRO_EVENT ev;

    // aguarda algum evento
    al_wait_for_event(event_queue, &ev);

    // verifica qual evento foi disparado
    check_event(ev);
}

void destroy_menu()
{
    // para musica de fundo
    al_stop_sample_instance(songs.menu_instance);
}

bool init_gameover()
{
    al_create_bitmap(game_parameters.SCREEN_W, game_parameters.SCREEN_H);

    al_draw_textf(game_font, al_map_rgb(0, 0, 0), 501, 331, 0, "VOCE PERDEU!");

    // enviar atualizações
    al_flip_display();

    // inicia game over music
    al_play_sample_instance(songs.game_over_instance);

    return true;
}

void run_gameover()
{
    // Cria evento local allegro
    ALLEGRO_EVENT ev;

    // aguarda algum evento
    al_wait_for_event(event_queue, &ev);

    // verifica qual evento foi disparado
    check_event(ev);
}


void destroy_gameover(){
    al_stop_sample_instance(songs.game_over_instance);
}
int main()
{

    // inicialização global
    if (!init())
    {
        return -1;
    }

    do
    {
        // inicializacao de menu
        if (!init_menu())
        {
            return -1;
        }

        // loop do menu
        while (menu_running)
        {
            run_menu();
        }

        // destroi o menu
        destroy_menu();

        // inicializacao de jogo
        if (!init_game())
        {
            return -1;
        }

        // executa o jogo
        while (game_running)
        {
            run_game();
        }

        // finaliza o jogo
        destroy_game();

        // inicializacao de menu
        if (!init_gameover())
        {
            return -1;
        }

        // loop do menu
        while (gameover_running)
        {
            run_gameover();
        }

        destroy_gameover();
    } while (!finish_game);

    return 0;
}
