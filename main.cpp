#include <iostream>
#include <ctime>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_opengl.h>

using namespace std;

// Definicao de FPS
const double FPS = 15;
// Definicao de Altura
const int SCREEN_H = 572;
// Definicao de largura
const int SCREEN_W = 572;
// Definicao do tamanho referencia
const int SIZE_REF = 26;

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

// fila de eventos
ALLEGRO_EVENT_QUEUE *event_queue = NULL;

// direcoes
bool move_up, move_down, move_left, move_right;

// matriz referencia
char MAP_REF[22][22] =
    {
        "111111111111111111111",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "000000000000000000000",
        "111111111111111111111",
};

// contador de posicao atual da cobra
int position = 0;

// frag termina o jogo
bool game_running = true;

// frag redesenha o jogo
bool redraw = true;

// posicao da fruta no mapa
int fruit_line, fruit_collum;

// matriz referencia "onde cobra passou"
int MOVEMENT[22][22] = {0};

// matriz referencia partes da cobra
int INDEX_SNAKE[22][22] = {0};

// tamanho inicial da cobra
int snake_size = 4;

// index da fruta atual
int fruit_index = 0;

// Posição da cobra na matriz
int i = 11, j = 11;

// posicao da cobra no display
int posy = i * SIZE_REF;
int posx = i * SIZE_REF;

// frag morreu
bool lost_game = false;

bool init_game()
{

    // verifica se e possivel carregar o allegro
    if (!al_init() || !al_init_image_addon())
    {
        cout << "ERRO! Nao foi possivel carregar os arquivos necessarios.\n";
        return false;
    }

    // verifica se os hardware necessario está disponível.
    if (!al_install_keyboard() || !al_install_mouse())
    {
        cout << "ERRO! O hardware necessario nao esta disponivel.\n";
        return false;
    }

    // Inicializa o timer do jogo.
    timer = al_create_timer(1.0 / FPS);
    if (!timer)
    {
        cout << "ERRO! Nao foi possivel inicializar o temporarizador. \n";
        return false;
    }

    // Inicializa display
    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display)
    {
        cout << "ERRO! Nao foi possivel inicializar o display.\n";
        al_destroy_timer(timer);
        return false;
    }

    // carrega o mapa
    map = al_load_bitmap("./assets/images/map.png");
    if (!map)
    {
        cout << "ERRO! Nao foi possivel carregar o mapa.\n";
        al_destroy_display(display);
        al_destroy_timer(timer);
        return false;
    }

    // desenha o mapa
    al_draw_bitmap(map, 0, 0, 0);

    // carrega sprite com partes da cobra
    snake = al_load_bitmap("./assets/images/snake.png");

    if (!snake)
    {
        cout << "ERRO! Nao foi possivel carregar personagem.\n";
        al_destroy_bitmap(map);
        al_destroy_display(display);
        al_destroy_timer(timer);
        return false;
    }

    // carrega frutas do jogo
    fruit = al_load_bitmap("./assets/images/fruits.png");

    if (!fruit)
    {
        cout << "ERRO! Nao foi possivel carregar frutas.\n";
        al_destroy_bitmap(snake);
        al_destroy_bitmap(map);
        al_destroy_display(display);
        al_destroy_timer(timer);
        return false;
    }

    // Cria a fila de eventos
    event_queue = al_create_event_queue();
    if (!event_queue)
    {
        cout << "ERRO! Nao foi possivel criar a fila de eventos.\n";

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

    // reinicio de cores
    al_clear_to_color(al_map_rgb(0, 0, 0));

    // envia mudancas para a interface
    al_flip_display();

    // inicia o timer
    al_start_timer(timer);

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
        fruit_collum = rand() % 21;
    } while (MAP_REF[fruit_line][fruit_collum] == '1' || MAP_REF[fruit_line][fruit_collum] == '4');

    MAP_REF[fruit_line][fruit_collum] = '3';
}

void move_snake()
{
    position++;
    MOVEMENT[i][j] = position;

    // verifica se é area livre para andar
    if (move_up && MAP_REF[i][j] != '1')
    {
        i--;
        posy = i * SIZE_REF;
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
        posy = i * SIZE_REF;
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
        posx = j * SIZE_REF;
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
        posx = j * SIZE_REF;
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
        // aumenta tamanho da cobra
        snake_size++;
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
        break;
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
    }

    else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
    {
    }
}

void draw_snake(int ii, int jj, int counter)
{
    // curvas
    if (MAP_REF[ii + 1][jj] == '4' && MAP_REF[ii][jj - 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 3, 26 * 0, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (MAP_REF[ii - 1][jj] == '4' && MAP_REF[ii][jj - 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 0, 26 * 0, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (MAP_REF[ii + 1][jj] == '4' && MAP_REF[ii][jj + 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 2, 26 * 2, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (MAP_REF[ii - 1][jj] == '4' && MAP_REF[ii][jj + 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 3, 26 * 2, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    // cabeça
    else if (counter == 0 && INDEX_SNAKE[ii][jj] == 1 && MAP_REF[ii + 1][jj] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 3, 26 * 1, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (counter == snake_size && INDEX_SNAKE[ii][jj] == 2 && MAP_REF[ii - 1][jj] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 4, 26 * 1, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (counter == 0 && INDEX_SNAKE[ii][jj] == 3 && MAP_REF[ii][jj + 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 2, 26 * 1, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (counter == snake_size && INDEX_SNAKE[ii][jj] == 4 && MAP_REF[ii][jj - 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 4, 26 * 0, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    // rabo
    else if (counter == snake_size && INDEX_SNAKE[ii][jj] == 1 && MAP_REF[ii - 1][jj] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 0, 26 * 2, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (counter == 0 && INDEX_SNAKE[ii][jj] == 2 && MAP_REF[ii + 1][jj] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 1, 26 * 1, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (counter == snake_size && INDEX_SNAKE[ii][jj] == 3 && MAP_REF[ii][jj - 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 2, 26 * 0, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (counter == 0 && INDEX_SNAKE[ii][jj] == 4 && MAP_REF[ii][jj + 1] == '4')
    {
        al_draw_bitmap_region(snake, 26 * 1, 26 * 0, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    // meio
    else if (INDEX_SNAKE[ii][jj] == 1 || INDEX_SNAKE[ii][jj] == 2 && (MAP_REF[ii + 1][jj] == '4' || MAP_REF[ii - 1][jj] == '4') && counter > 0 && counter < snake_size)
    {
        al_draw_bitmap_region(snake, 26 * 0, 26 * 1, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
    else if (INDEX_SNAKE[ii][jj] == 3 || INDEX_SNAKE[ii][jj] == 4 && (MAP_REF[ii][jj + 1] == '4' || MAP_REF[ii][jj - 1] == '4') && counter > 0 && counter < snake_size)
    {
        al_draw_bitmap_region(snake, 26 * 1, 26 * 2, 26, 26, jj * SIZE_REF, ii * SIZE_REF, 0);
    }
}

void redraw_game()
{
    // contador de partes da cobra
    int counter = 0;

    // desenha o mapa
    al_draw_bitmap(map, 0, 0, 0);

    // desenha a fruta
    al_draw_bitmap_region(fruit, 26 * fruit_index, 0, 26, 26, fruit_collum * SIZE_REF, fruit_line * SIZE_REF, 0);

    // desenha a cobra
    for (int i = 0; i < 22; i++)
    {
        for (int j = 0; j < 22; j++)
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
        game_running = false;
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

    // verifica se é o fim do jogo
    is_game_lost();
}

void finish_game()
{
    al_destroy_bitmap(fruit);
    al_destroy_bitmap(snake);
    al_destroy_bitmap(map);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
}

int main()
{
    // carrega o jogo
    if (!init_game())
    {
        return 0;
    }

    // inicializa variaveis de direcao
    move_up = false;
    move_down = false;
    move_left = false;
    move_right = false;

    // gera a fruta inicial
    generate_fruit();

    // executa o jogo
    while (game_running)
    {
        run_game();
    }

    // finaliza o jogo
    finish_game();

    return 0;
}