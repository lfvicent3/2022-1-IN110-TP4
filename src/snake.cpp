#include <iostream>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <time.h>

using namespace std;

const float FPS = 15;
const int SCREEN_W = 572;
const int SCREEN_H = 572;
const int QUAD_SIZE = 26;

// matriz definindo mapa do jogo: 1 representa paredes, 0 representa corredor
char MAPA[22][22] =
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

int andou[22][22] = {0};
int passo = 0;

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_BITMAP *mapa = NULL;
ALLEGRO_BITMAP *quad = NULL;
ALLEGRO_BITMAP *fruta = NULL;

int i = 15, j = 12; // posicao inicial da Snake na matriz
int q = 26;         // tamanho de cada celula no mapa
int posy = i * q;   // posicao da Snake no display
int posx = j * q;

bool redraw = true;
bool sair = false;
bool cima, baixo, esq, dir;
bool morreu1 = false;
int tam = 20;
int c, l;

void morreu()
{
    display = al_create_display(SCREEN_W, SCREEN_H);
}

void gera_frutinha()
{

    srand(time(0));
    do
    {

        c = rand() % 20;
        l = rand() % 20;

    } while (MAPA[l][c] == '1');

    MAPA[l][c] = '3';
}

int inicializa()
{
    if (!al_init())
    {
        cout << "Falha ao carregar Allegro" << endl;
        return 0;
    }

    if (!al_install_keyboard())
    {
        cout << "Falha ao inicializar o teclado" << endl;
        return 0;
    }

    timer = al_create_timer(1.0 / FPS);
    if (!timer)
    {
        cout << "Falha ao inicializar o temporizador" << endl;
        return 0;
    }

    if (!al_init_image_addon())
    {
        cout << "Falha ao iniciar al_init_image_addon!" << endl;
        return 0;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display)
    {
        cout << "Falha ao inicializar a tela" << endl;
        al_destroy_timer(timer);
        return 0;
    }

    mapa = al_load_bitmap("map.png");
    if (!mapa)
    {
        cout << "Falha ao carregar o mapa!" << endl;
        al_destroy_display(display);
        return 0;
    }
    al_draw_bitmap(mapa, 0, 0, 0);

    quad = al_create_bitmap(QUAD_SIZE, QUAD_SIZE);
    fruta = al_create_bitmap(QUAD_SIZE, QUAD_SIZE);

    if (!quad)
    {
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    if (!fruta)
    {
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    al_set_target_bitmap(quad);                       // muda destino dos desenhos para o bitmap quad
    al_clear_to_color(al_map_rgb(255, 0, 0));         // limpa e colore de magenta
    al_set_target_bitmap(al_get_backbuffer(display)); // muda de volta o destino dos desenhos para o display

    al_set_target_bitmap(fruta);                      // muda destino dos desenhos para o bitmap quad
    al_clear_to_color(al_map_rgb(0, 0, 255));         // limpa e colore de magenta
    al_set_target_bitmap(al_get_backbuffer(display)); // muda de volta o destino dos desenhos para o display

    event_queue = al_create_event_queue();
    if (!event_queue)
    {
        cout << "Falha ao criar a fila de eventos" << endl;
        al_destroy_bitmap(quad);
        al_destroy_bitmap(fruta);
        al_destroy_display(display);
        al_destroy_timer(timer);
        return 0;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_start_timer(timer);

    return 1;
}

int main(int argc, char **argv)
{
    if (!inicializa())
        return -1;

    cima = false;
    baixo = false;
    esq = false;
    dir = false;

    gera_frutinha();

    while (!sair)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            passo++;
            andou[i][j] = passo;

            if (cima && MAPA[i][j] != '1')
            {
                i--;
                posy = i * q;
            }
            else if (cima && MAPA[i][j] == '1')
            {
                morreu1 = true;
            }

            if (baixo && MAPA[i][j] != '1')
            {
                i++;
                posy = i * q;
            }
            else if (baixo && MAPA[i][j] == '1')
            {
                morreu1 = true;
            }

            if (esq && MAPA[i][j] != '1')
            {
                j--;
                posx = j * q;
            }

            if (dir && MAPA[i][j] != '1')
            {
                j++;
                posx = j * q;
            }

            if ((cima || baixo || esq || dir) && MAPA[i][j] == '3')
            {
                tam++;
                MAPA[l][c] = '0';
                gera_frutinha();
            }
            if ((cima || baixo || esq || dir) && MAPA[i][j] == '4')
            {
                morreu1 = true;
            }

            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch (ev.keyboard.keycode)
            {
            case ALLEGRO_KEY_UP: // setinha pra cima
                cima = true;     // esta indo pra cima
                baixo = false;
                esq = false;
                dir = false;
                break;

            case ALLEGRO_KEY_DOWN: // setinha pra baixo
                cima = false;
                baixo = true;
                esq = false;
                dir = false;
                break;

            case ALLEGRO_KEY_LEFT: // setinha pra esquerda
                cima = false;
                baixo = false;
                esq = true;
                dir = false;
                break;

            case ALLEGRO_KEY_RIGHT: // setinha pra direita
                cima = false;
                baixo = false;
                esq = false;
                dir = true;
                break;
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP)
        {
            switch (ev.keyboard.keycode)
            {
            case ALLEGRO_KEY_ESCAPE: // tecla ESC
                sair = true;
                break;
            }
        }

        if (redraw && al_is_event_queue_empty(event_queue))
        {
            redraw = false;

            al_clear_to_color(al_map_rgb(0, 0, 0));

            al_draw_bitmap(mapa, 0, 0, 0);
            al_draw_bitmap(fruta, c * q, l * q, 0);

            for (int i = 0; i < 22; i++)
            {
                for (int j = 0; j < 22; j++)
                {
                    if (MAPA[i][j] == '4')
                    {
                        MAPA[i][j] = '0';
                    }
                    if (andou[i][j] > 0 && andou[i][j] >= passo - tam)
                    {                                          // se passou aqui nos ultimos 4 passos
                        al_draw_bitmap(quad, j * q, i * q, 0); // desenha quadrado
                        MAPA[i][j] = '4';
                    }
                }
            }

            if (morreu1)
            {
                morreu();
                sair = true;
            }
            al_flip_display();
        }
    }

    al_destroy_bitmap(quad);
    al_destroy_bitmap(mapa);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}
