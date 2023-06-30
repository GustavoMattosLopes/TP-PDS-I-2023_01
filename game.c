#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#define NUM_PRATOS 8    // Quantidade máxima de pratos que aparecerão ao longo do jogo

const float FPS = 100;  // Variável global constante que definirá quantidade de frames por segundo (taxa de atualização da tela)

const int SCREEN_W = 960;
const int SCREEN_H = 540;   // Variáveis globais constantes que definirão o tamanho da tela gerada em pixels

const float JOGADOR_W = 75;
const float JOGADOR_H = 105;    // Variáveis globais constantes que definirão o tamanho da sprite do jogador em pixels

const float PRATO_W = 60;
const float PRATO_H = 10; // Variáveis globais constantes que definirão o tamanho dos pratos em pixels

const float DISTANCE_BETWEEN = 105; // Variável global constante que definirá distância entre os pratos/postes
const float FIRST_POLE_X = 110; // Variável global constante que definirá coordenada x do primeiro poste
int POLE_SIZE = 6; // Variável global que definirá a espessura (em pixels) de cada poste

typedef struct Jogador {
    int index;  // Determina se é Jogador 1 ou Jogador 2
	float x;    // Posição Horizontal do jogador
	int equilibrando;   // Booleano para indicar se o jogador está ou não equilibrando algum prato
	int mov_esq, mov_dir;   // Booleanos para indicar se o jogador está ou não se movimentando para algum lado
	float vel; // Velocidade de movimento do jogador

} Jogador;  // Estrutura criada para armazenar informações relativas ao jogador

typedef struct Prato {

	float x;    // Posição Horizontal de cada prato
	double energia;  // 0 (prato equilibrado) - 255 (máxima energia, prestes a cair) 
	float tempoParaAparecer; // Tempo para cada prato aparecer durante a execução do jogo (em segundos)
    int aparecendo; // Booleano para indicar se o prato está ou não aparecendo no jogo

} Prato;  // Estrutura criada para armazenar informações relativas aos pratos

void draw_beginning(ALLEGRO_BITMAP *, ALLEGRO_FONT *);
void draw_scenario(ALLEGRO_BITMAP *, ALLEGRO_TIMER *, ALLEGRO_FONT *);

void modify_poles(Prato *, Jogador);

void initialize_player(Jogador *);
void draw_player(Jogador, ALLEGRO_BITMAP *);
void update_player(Jogador *);

float gener_time_for_plate(int);
void initialize_plates(Prato *);
void draw_plates(Prato *, int *, ALLEGRO_TIMER *, ALLEGRO_FONT *, ALLEGRO_BITMAP *);
void update_plates(Prato *, ALLEGRO_TIMER *, Jogador, int);

long long int record(long long int, int);
void draw_record_screen(long long int, long long int, ALLEGRO_FONT *);

void powerup_apparition(Jogador *, Prato *, int *, int);
void powerup_activated(int *, Jogador *, int, Jogador *, Prato *, int *);
void powerup_disabled(int *, Jogador *, int, Jogador *, Prato *, int *);


int main()
{
    srand(time(NULL));

    ALLEGRO_DISPLAY *display;   // Ponteiro para uma estrutura "ALLEGRO_DISPLAY", que representa a tela a ser gerada
    ALLEGRO_EVENT_QUEUE *event_queue;   // Ponteiro para uma estrutura do Allegro, que representa a lista de eventos do programa
	ALLEGRO_TIMER *timer;   // Ponteiro para uma estrutura do Allegro, que representa um contador de tempo

	if(!al_init())  // Função para a inicialização do allegro
    {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

    timer = al_create_timer(1.0 / FPS); // Cria um contador (temporizador) que incrementa uma unidade a cada 1.0/FPS segundos
    if(!timer) 
    {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
    
    //OBS: Permitem criar elementos visuais básicos; Funções de desenho para primitivas geométricas simples
    if(!al_init_primitives_addon()) // Função para inicializar o módulo de primitivas do Allegro
    {
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }

	if(!al_init_image_addon())  // Função para inicializar o módulo que permite carregar imagens no jogo
    {
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}

	display = al_create_display(SCREEN_W, SCREEN_H);    // Função para criar uma tela de acordo com as dimensões propostas em pixels
	if(!display)
    {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}
    al_set_window_title(display, "Dancing Plates"); // Função para definir o título da janela de execução do jogo

	al_init_font_addon(); // Função para inicializar o módulo Allegro que carrega as fontes
	if(!al_init_ttf_addon()) // Função que inicializa o modulo Allegro que entende arquivos tff de fontes
    {
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}

    ALLEGRO_FONT *size_32 = al_load_font("./assets/arial.ttf", 32, 1); // Carrega o arquivo arial.ttf
	if(size_32 == NULL) {
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}

	if(!al_install_keyboard()) // Função para instalar o teclado
    {
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}

    if (!al_install_mouse()) // Função para instalar o mouse
    {
        fprintf(stderr, "failed to install mouse!\n");
        return -1;
    }

	event_queue = al_create_event_queue();  // Função para criar uma fila de eventos e atribuir as informações da fila criada
	if(!event_queue)
    {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
	}
        // Registra na fila os eventos de tipo tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	    // Registra na fila os eventos de tempo (ex: quando o tempo altera de t para t+1)
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	    // Registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
    	// Registra na fila os eventos de mouse (ex: clique na tela)
    al_register_event_source(event_queue, al_get_mouse_event_source());

    ALLEGRO_BITMAP *cover = al_load_bitmap("./assets/capa.png"); // Função para carregar a imagem que será usada como fundo na tela inicial
    if (!cover) 
    {
        fprintf(stderr, "failed to load cover image!\n");
        return -1;
    }

    ALLEGRO_BITMAP *background = al_load_bitmap("./assets/fundo.png"); // Função para carregar a imagem que será usada como fundo na tela inicial
    if (!background) 
    {
        fprintf(stderr, "failed to load background image!\n");
        return -1;
    }

    ALLEGRO_BITMAP *icon = al_load_bitmap("./assets/icon.png"); // Função para carregar o ícone da janela
    if (!icon)
    {
        fprintf(stderr, "failed to load icon!\n");
        return -1;
    }
    al_set_display_icon(display, icon);

    ALLEGRO_BITMAP *player1 = al_load_bitmap("./assets/player1.png");
    ALLEGRO_BITMAP *player2 = al_load_bitmap("./assets/player2.png");   // Função para carregar a sprite dos jogadores
    if (!player1 || !player2)
    {
        fprintf(stderr, "failed to load player image!\n");
        return -1;
    }

    // Funções para inicializar o módulo de áudio do Allegro
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(1);

    ALLEGRO_SAMPLE *song = al_load_sample("./assets/song.wav");
    if (!song)
    {
        fprintf(stderr, "failed to load song!\n");
        return -1;
    }

    int start = 1, playing2 = 0;
    while(start)
    {
        draw_beginning(cover, size_32);

        al_flip_display();  // Função para atualizar a tela e exibí-la com tudo o que foi desenhado

        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event); // Espera por um evento e o armazena na variável "event"
        if(event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            int X = event.mouse.x;
            int Y = event.mouse.y;
            if (X >= 650 && X <= 890    &&     Y >= 125 && Y <= 180)
            {
                start = 0;
            }
            else if (X >= 650 && X <= 890    &&     Y >= 235 && Y <= 290)
            {
                start = 0;
                playing2 = 1;
            }
        }
        else if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			return 0;
    
    }
    al_destroy_bitmap(cover);

    Jogador jogador, jogador2;
    initialize_player(&jogador); // Passagem por referência para alterar dados de jogador

    if(playing2)
    {
        initialize_player(&jogador2);
        jogador.x = SCREEN_W / 3;
        jogador2.x = 2 * SCREEN_W / 3;
        jogador2.index = 2;
    }

    Prato pratos[NUM_PRATOS];
	initialize_plates(pratos);

    al_start_timer(timer);  // Inicializa o temporizador

    int tempo_powerup = 15+rand()%20, check_powerup = 0, freeze = 0;

    int playing = 1;
    while(playing)
    {
        ALLEGRO_SAMPLE_ID IDCurrentSong;
        al_play_sample(song, 0.5, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &IDCurrentSong); 
                            // Números: volume, posição estéreo (central), velocidade de reprodução 

        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event); // Espera por um evento e o armazena na variável "event"

            // Se o tipo do evento ocorrido for um evento do temporizador, ou seja, se o tempo passou de t para t+1
        if(event.type == ALLEGRO_EVENT_TIMER)
        {
            draw_scenario(background, timer, size_32);
            modify_poles(pratos, jogador);

            update_player(&jogador);
            draw_player(jogador, player1);
            
            if(playing2)
            {
                update_player(&jogador2);
                draw_player(jogador2, player2);
                modify_poles(pratos, jogador2);
                update_plates(pratos, timer, jogador2, freeze);
            }

            update_plates(pratos, timer, jogador, freeze);
            draw_plates(pratos, &playing, timer, size_32, background);

            int seconds = al_get_timer_count(timer)/(int)FPS;

            int how_long, which_plate;
            if(seconds >= tempo_powerup && !check_powerup)
            {
                which_plate =  tempo_powerup%(NUM_PRATOS-2)+1; // Não aparecerá nos pratos das extremidades (1-6)

                powerup_apparition(&jogador, pratos, &check_powerup, which_plate);
                if(playing2)
                powerup_apparition(&jogador2, pratos, &check_powerup, which_plate);

                if(check_powerup)
                {
                    how_long = seconds+10;
                    powerup_activated(&check_powerup, &jogador, playing2, &jogador2, pratos, &freeze);
                }
            }
            else if(check_powerup)
            {
                char * powerups[] = {"SPEED++", "THICKNESS++", "FREEZE!"};
                al_draw_text(size_32, al_map_rgb(254, 254, 60), pratos[which_plate].x, JOGADOR_H-50, ALLEGRO_ALIGN_CENTER, powerups[check_powerup-1]);

                if(seconds == how_long)
                {
                    tempo_powerup += seconds;
                    powerup_disabled(&check_powerup, &jogador, playing2, &jogador2, pratos, &freeze);
                }
            }

            al_flip_display();  // Função para atualizar a tela e exibí-la com tudo o que foi desenhado/alterado (conforme FPS)

            if(al_get_timer_count(timer)%(int)FPS == 0)
            	printf("%d seconds\n", seconds);
        }
            // Se o tipo do evento ocorrido for do tipo do evento de fechamento da tela (código relativo) 
        else if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
			playing = 0;
        }
            // Se o tipo do evento ocorrido for uma tecla sendo acionada/pressionada
		else if(event.type == ALLEGRO_EVENT_KEY_DOWN) {

			if(event.keyboard.keycode == ALLEGRO_KEY_A) {
				jogador.mov_esq = 1;
			}
			else if(event.keyboard.keycode == ALLEGRO_KEY_D) {
				jogador.mov_dir = 1;
			}
            else if(event.keyboard.keycode == ALLEGRO_KEY_SPACE){
                jogador.equilibrando = 1;
            }
            else if(playing2 && event.keyboard.keycode == ALLEGRO_KEY_LEFT){
                jogador2.mov_esq = 1;
            }
            else if(playing2 && event.keyboard.keycode == ALLEGRO_KEY_RIGHT){
                jogador2.mov_dir = 1;
            }
            else if(playing2 && event.keyboard.keycode == ALLEGRO_KEY_ENTER){
                jogador2.equilibrando = 1;
            }  
		}
		    // Se o tipo do evento ocorrido for uma tecla sendo liberada/solta
		else if(event.type == ALLEGRO_EVENT_KEY_UP) {

			if(event.keyboard.keycode == ALLEGRO_KEY_A) {
				jogador.mov_esq = 0;
			}
			else if(event.keyboard.keycode == ALLEGRO_KEY_D) {
				jogador.mov_dir = 0;
			}
            else if(event.keyboard.keycode == ALLEGRO_KEY_SPACE){
                jogador.equilibrando = 0;
            }
            else if(playing2 && event.keyboard.keycode == ALLEGRO_KEY_LEFT){
                jogador2.mov_esq = 0;
            }
            else if(playing2 && event.keyboard.keycode == ALLEGRO_KEY_RIGHT){
                jogador2.mov_dir = 0;
            }
            else if(playing2 && event.keyboard.keycode == ALLEGRO_KEY_ENTER){
                jogador2.equilibrando = 0;
            }
		}
    }
    al_destroy_sample(song);
    
    long long int pontos = (long long int) al_get_timer_count(timer);
    long long int recorde = record(pontos, playing2);

    while(1)
    {
        draw_record_screen(pontos, recorde, size_32);
        al_flip_display();  // Função para atualizar a tela e exibí-la com tudo o que foi desenhado

        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);
        if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			break;
    }

        // Funções para procedimentos de fim de jogo (encerra o temporizador, fecha a tela, limpa a memória)
	al_destroy_display(display);
    al_destroy_bitmap(background);
    al_destroy_bitmap(icon);
    al_destroy_bitmap(player1);
    al_destroy_bitmap(player2);
	al_destroy_event_queue(event_queue);
    al_destroy_font(size_32);

    return 0;
}


void draw_beginning(ALLEGRO_BITMAP *background, ALLEGRO_FONT *font)
{
    al_clear_to_color(al_map_rgb(0, 0, 0)); 
        
    al_draw_bitmap(background, 0, 0, 0); // Desenha o fundo na posição (0, 0)

    al_draw_filled_rounded_rectangle(650, 125, 
                                         890, 180,
                                         20, 20, 
                                         al_map_rgb(239, 184, 16));
    al_draw_text(font, al_map_rgb(0, 0, 0), 770, 135, ALLEGRO_ALIGN_CENTER, "1 Player");
    al_draw_rounded_rectangle(650, 125, 
                              890, 180,
                              20, 20, 
                              al_map_rgb(0, 0, 0), 2);

    al_draw_filled_rounded_rectangle(650, 235, 
                                         890, 290,
                                         20, 20, 
                                         al_map_rgb(239, 184, 16));
    al_draw_text(font, al_map_rgb(0, 0, 0), 770, 245, ALLEGRO_ALIGN_CENTER, "2 Players");
    al_draw_rounded_rectangle(650, 235, 
                              890, 290,
                              20, 20, 
                              al_map_rgb(0, 0, 0), 2);
}


void draw_scenario(ALLEGRO_BITMAP *background, ALLEGRO_TIMER *timer, ALLEGRO_FONT *font)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));   // Função para limpar o fundo da tela (retirar elementos outrora desenhados)

    al_draw_bitmap(background, 0, 0, 0);

    char texto[50]; 
    sprintf(texto, "%lld", al_get_timer_count(timer)); // Criar string formatada
    al_draw_text(font, al_map_rgb(0, 223, 15), 50, 50, ALLEGRO_ALIGN_CENTER, texto);

    for (int i = 0; i < NUM_PRATOS; i++)
    {
        al_draw_filled_rectangle(FIRST_POLE_X + DISTANCE_BETWEEN*i, JOGADOR_H,
                                 FIRST_POLE_X+POLE_SIZE + DISTANCE_BETWEEN*i, SCREEN_H - JOGADOR_H,
                                 al_map_rgb(255, 255, 255));
    }
}


void modify_poles(Prato *p, Jogador j)
{
    for (int i = 0; i < NUM_PRATOS; i++)
    {
        int x = rand()%2, y = rand()%2, z = rand()%2;
        if(p[i].aparecendo && j.equilibrando &&  !(j.mov_dir || j.mov_esq) &&
        j.x >= FIRST_POLE_X + DISTANCE_BETWEEN*i && j.x <= FIRST_POLE_X+POLE_SIZE + DISTANCE_BETWEEN*i)
        {
            al_draw_filled_rectangle(FIRST_POLE_X + DISTANCE_BETWEEN*i, JOGADOR_H,               // Ponto ESQ. SUP.
                                     FIRST_POLE_X+POLE_SIZE + DISTANCE_BETWEEN*i, SCREEN_H - JOGADOR_H,    // Ponto DIR. INF.
                                     al_map_rgb(255 * x, 255 * y, 255 * z));
        }
    }
}


void initialize_player(Jogador *j)
{
    j->index = 1;
    j->x = SCREEN_W / 2;
    j->equilibrando = 0;
    j->mov_esq = 0;
    j->mov_dir = 0;
    j->vel = 1;
}


void draw_player(Jogador j, ALLEGRO_BITMAP *player)
{  
    al_draw_bitmap(player, j.x - JOGADOR_W + 12, SCREEN_H - JOGADOR_H, 0);
    /* A imagem do jogador será desenhada a partir da extremidade superior à esquerda cujas coordenadas são:
        - x: de modo que j.x esteja centralizado na mão do personagem (12 pixels à esquerda da extremidade direita)
        - y: de modo que o personagem seja todo desenhado na tela (da altura do jogador até a extremidade inferior)
    */
}


void update_player(Jogador *j)
{
    if(j->mov_esq)  // Se há movimento para a esquerda (1 = T, 0 = F)
    {
        if((j->x - (JOGADOR_W-12) - j->vel) > 0)
        j->x -= j->vel;
    }
    if(j->mov_dir)  // Se há movimento para a direita (1 = T, 0 = F)
    {
        if((j->x + 12 + j->vel) < SCREEN_W)
        j->x += j->vel;
    }
}


float gener_time_for_plate(int i)
{
    int ls[] = {30, 20, 10, 5, 0, 10, 15, 25};
    return ls[i]+rand()%6;
}


void initialize_plates(Prato *p)
{
    for(int i=0; i < NUM_PRATOS; i++)
    {
		p[i].x = FIRST_POLE_X + DISTANCE_BETWEEN*i + POLE_SIZE/2;
		p[i].tempoParaAparecer = gener_time_for_plate(i);
        p[i].aparecendo = 0;
		p[i].energia = 0;
	}
}


void draw_plates(Prato *p, int *playing, ALLEGRO_TIMER *timer, ALLEGRO_FONT *font, ALLEGRO_BITMAP *background)
{
    for(int i = 0; i < NUM_PRATOS; i++)
    {
        float pY_pratos = JOGADOR_H;
        if(p[i].aparecendo && p[i].energia <= 255)
        {
            al_draw_filled_rectangle(p[i].x - PRATO_W/2, pY_pratos - PRATO_H,
                                     p[i].x + PRATO_W/2, pY_pratos,
                                     al_map_rgb(255, 255-p[i].energia, 255-p[i].energia));
        }
        else if(p[i].energia > 255)
        {
            al_stop_timer(timer);
            while(pY_pratos < SCREEN_H)
            {
                draw_scenario(background, timer, font);
                pY_pratos += 86;
                al_draw_filled_rectangle(p[i].x - PRATO_W/2, pY_pratos - PRATO_H,
                                        p[i].x + PRATO_W/2, pY_pratos,
                                        al_map_rgb(255, 0, 0));
                al_flip_display();
                al_rest(0.1);
            }
            *playing = 0;
            break;
        }                     
    }
}


void update_plates(Prato *p, ALLEGRO_TIMER * t, Jogador j, int freeze)
{
    for(int i = 0; i < NUM_PRATOS; i++)
    {
        if(!p[i].aparecendo && al_get_timer_count(t)/(int)FPS >= p[i].tempoParaAparecer)
        {
            p[i].aparecendo = 1;
        }
        else if (p[i].aparecendo && j.equilibrando && !(j.mov_dir || j.mov_esq) &&
        j.x >= FIRST_POLE_X + DISTANCE_BETWEEN*i && j.x <= FIRST_POLE_X+POLE_SIZE + DISTANCE_BETWEEN*i)
        {
            if(p[i].energia >= 2.04)
                p[i].energia -= 2.04;
        }
        else if(!freeze && p[i].aparecendo && al_get_timer_count(t)%(int)FPS == 0 && j.index == 1)
//Atualiza energia dos pratos se powerup-freeze não estiver ativo, se o prato está aparecendo, a cada segundo e somente para um jogador
        {
            p[i].energia += 10.2; // (255/10.2) = 25, ou seja, o prato está configurado para cair passados 25s
        }
    }
}


long long int record(long long int points, int playing2)
{
    long long int record;
    FILE *fp;
    fp = fopen("record.bin", "rb");
    if(playing2)
        fseek(fp, sizeof(long long int), SEEK_SET);
    fread(&record, sizeof(long long int), 1, fp);
    fclose(fp);

    if(points > record && !playing2)
    {
        long long int temp;
        fp = fopen("record.bin", "rb");
        fseek(fp, sizeof(long long int), SEEK_SET);
        fread(&temp, sizeof(long long int), 1, fp);
        fclose(fp);

        fp = fopen("record.bin", "wb");
        fwrite(&points, sizeof(long long int), 1, fp);
        fwrite(&temp, sizeof(long long int), 1, fp);
        fclose(fp);

        printf("New Record: %lld\n", points);
        return points;
    }
    else if(points > record && playing2)
    {
        long long int temp;
        fp = fopen("record.bin", "rb");
        fread(&temp, sizeof(long long int), 1, fp);
        fclose(fp);

        fp = fopen("record.bin", "wb");
        fwrite(&temp, sizeof(long long int), 1, fp);
        fwrite(&points, sizeof(long long int), 1, fp);
        fclose(fp);

        printf("New Record: %lld\n", points);
        return points;
    }
    else
    {
        printf("Record: %lld\nPoints: %lld\n", record, points);
        return record;
    }
}


void draw_record_screen(long long int points, long long int record, ALLEGRO_FONT *font)
{
    al_clear_to_color(al_map_rgb(243, 243, 243));
    if(points == record)
    {
        char texto[50]; 
        sprintf(texto, "Novo Recorde: %lld", record); //Criar string formatada

        al_draw_text(font, al_map_rgb(0, 0, 0), SCREEN_W/2, SCREEN_H/2, ALLEGRO_ALIGN_CENTER, "Game Over!");
        al_draw_text(font, al_map_rgb(0, 255, 0), SCREEN_W/2, SCREEN_H/2+50, ALLEGRO_ALIGN_CENTER, texto);
    }
    else
    {
        char texto1[50], texto2[50];
        sprintf(texto1, "Pontuação atual: %lld", points);
        sprintf(texto2, "Recorde vigente: %lld", record);

        al_draw_text(font, al_map_rgb(255, 0, 0), SCREEN_W/2, SCREEN_H/2-50, ALLEGRO_ALIGN_CENTER, texto1);
        al_draw_text(font, al_map_rgb(0, 0, 0), SCREEN_W/2, SCREEN_H/2, ALLEGRO_ALIGN_CENTER, "Game Over!");
        al_draw_text(font, al_map_rgb(0, 255, 0), SCREEN_W/2, SCREEN_H/2+50, ALLEGRO_ALIGN_CENTER, texto2);
    }   
}


void powerup_apparition(Jogador *j, Prato *p, int *check, int index)
{
    if(p[index].aparecendo)
    {
        al_draw_filled_circle(p[index].x, JOGADOR_H-50, 20, 
                              al_map_rgb(239, 184, 16));
    }

    if(j->equilibrando && !(j->mov_dir || j->mov_esq) &&
    j->x >= FIRST_POLE_X + DISTANCE_BETWEEN*index && j->x <= FIRST_POLE_X+POLE_SIZE + DISTANCE_BETWEEN*index)
    {
        *check = rand()%3+1;
    }
}


void powerup_activated(int *check, Jogador *j, int playing2, Jogador *j2, Prato *p, int *freeze)
{
    if(*check == 1)
    {
        j->vel = 2;
        if(playing2)
        j2->vel = 2;
    }
    else if(*check == 2)
    {
        POLE_SIZE = 10;
        for(int i = 0; i < NUM_PRATOS; i++)
            p[i].x = FIRST_POLE_X + DISTANCE_BETWEEN*i + POLE_SIZE/2; // Centralizando os pratos nos postes em suas novas medidas
    }
    else if(*check == 3)
    {
        *freeze = 1;
    }
}


void powerup_disabled(int *check, Jogador *j, int playing2, Jogador *j2, Prato *p, int *freeze)
{
    if(*check == 1)
    {
        j->vel = 1;
        if(playing2)
        j2->vel = 1;
    }
    else if(*check == 2)
    {
        POLE_SIZE = 6;
        for(int i = 0; i < NUM_PRATOS; i++)
            p[i].x = FIRST_POLE_X + DISTANCE_BETWEEN*i + POLE_SIZE/2; // Centralizando novamente os pratos nos postes
    }
    else if(*check == 3)
    {
        *freeze = 0;
    }

    *check = 0;
}               