#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "mapGenerator.h"
#include "monster.h"
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

pthread_mutex_t lock;
sem_t semaph;

int monster0Movements[500];
int monster0MovementsIndex = 0;

int monster1Movements[500];
int monster1MovementsIndex = 0;

void randomDestination(int * destinations) {
    int a[4] = {0,1,2,3};

    for (int i = 0; i < 4; i++)
    {
        int j = rand() % 4;
        int temp = a[i];
        a[i] = a[j];
        a[j] = temp;
    }

    memcpy(destinations, a, sizeof(a));
}

int getNextRoomForMonster(int id)
{
    int currentIndex = 0;
    int canProceedToThisRoom = 0;

    struct room *tempRoom = getRoomPointerByID(id);
    struct room *tempNeighbour;
    int destinations[4];

    randomDestination(destinations);
            
    int neighbourID;
    for (int i = 0; i < 4; i++)
    {
        canProceedToThisRoom = 0;
        currentIndex = destinations[i];
        neighbourID = getNeighbour(id, currentIndex); //Se obtiene el numero de cuarto candidato a entrar
        if (neighbourID != -1)
        {
            if (tempRoom->doors[currentIndex].state == Open)
            {
                //! Region critica
                pthread_mutex_lock(&lock); //Aqui el hilo monstruo obtiene el estado de una habitacion de manera sincornizada y revisa si esta ocupada o no
                tempNeighbour = getRoomPointerByID(neighbourID); 
                if(tempNeighbour->occupied == 0 && tempNeighbour->type != Start && tempNeighbour->type != Goal && tempNeighbour->type != Wall){
                    canProceedToThisRoom = 1;
                }
                pthread_mutex_unlock(&lock);
            }
            
            if(canProceedToThisRoom)
            {
                return neighbourID;
            }
            
        }
    }
    return -1;

}

void * monsterLife(void * m){
    struct monster * mon = (struct monster *) m;
    int freeNSize = 0;
    int freeNeighbours[4];
    //printf("\nMonster %d is alive\n", mon->id);
    //while(mon->hp != 0){
    int r; 

    while(mon->hp > 0){
        //printf("\nLooking for free neighbours of %d in %d\n", mon->id, mon->location);
        r = rand() % 100;
        int loc = getNextRoomForMonster(mon->location);

        if(loc!=-1){ //Si hay una habitacion libre
            setOccupied(mon->location, 0);
            monsterMove(mon, loc);
            setOccupied(loc, 1);
            //Esto es temporal
            if(mon->id==0){
                monster0Movements[monster0MovementsIndex] = loc;
                monster0MovementsIndex++;
            }else if(mon->id==1){
                monster1Movements[monster1MovementsIndex] = loc;
                monster1MovementsIndex++;
            }
            //-----------------------------------------------------
            //printf("Found (%d), a free neighbour of monster %d, located in %d\n", loc, mon->id, mon->location);

        }else{ //Si no hay habitaciones libres, el monstruo se queda quieto
            changeMonsterState(mon, IDLE);
            //printf("No free neighbours found for monster %d, located in %d\n", mon->id, mon->location);
        }       

        // .Todo esto es temporal
            /*pthread_mutex_lock(&lock);
            system("clear");
            //drawTemporalMap();
            pthread_mutex_unlock(&lock);*/

            /*printf("Monster 0 path:\n");
            printArray(monster0Movements, monster0MovementsIndex);   
            printf("Monster 1 path:\n");
            printArray(monster1Movements, monster1MovementsIndex);*/
            //sem_post(&semaph);
            sleep((rand()%3)+1);
        // ---------------------------------------------------------
        if(current_game_state == GAME_STATE_OVER || current_game_state == GAME_STATE_VICTORY){
            pthread_exit(0);
        }
    }
    
    printf("Monster %d is dead\n", mon->id);
    sleep(3);
    pthread_exit(0);
}

int main(){

    srand(time(NULL));
	int next_room_id;
    struct room *current_room_player;
	struct room *next_room_player;

    int eleccion = 0;
    int input_valid = 0;
    char input[50];

    while (!input_valid)
    {
        printf("\n");
        printf("----------------------------------");
        printf("\n");
        printf("\n");
        printf("Escoja Dificultad");
        printf("\n\n");
        printf("1. Facil");
        printf("\n");
        printf("2. Medio");
        printf("\n");
        printf("3. Dificil");
        printf("\n");

        printf("\n");
        printf("----------------------------------");
        printf("\n");

        printf("\n");
        printf("Eleccion: ");
        scanf("%s", &input);

        printf("\n");
        printf("----------------------------------");
        printf("\n");

        // printf("Input %s\n", input);

        input_valid = atoi(input);

        if (input_valid && (atoi(input) == atof(input)))
        {
            eleccion = atoi(input);

            if (eleccion >= 1 && 3 >= eleccion)
            {
                break;
            }
        }
        printf(RED "\nError: Ingrese una dificultad valida.\n" DEFAULT);
        input_valid = 0;
    }

    switch (eleccion)
    {
    case 1:
        N = EASY;
        DOOR = 10;
        break;
    case 2:
        N = MEDIUM;
        DOOR = 22;
        break;
    case 3:
        N = HARD;
        DOOR = 23;
        break;
    }
    CELL = SCREEN/N;
    
    while(generateMap(N)==-1){
        printf("Reintentando generar mapa...\n");
    }

    pthread_mutex_init(&lock, NULL);

    int monsterCount = N/2;

    pthread_t * monsterThreads;
    monsterThreads = (pthread_t *)malloc(sizeof(pthread_t)*monsterCount);

    struct monster monsters[monsterCount];

    for(int m = 0; m < monsterCount; m++){
        monsters[m].id = m;
        monsters[m].hp = 3;
        monsters[m].atk = 1;
        monsters[m].state = IDLE;
        monsters[m].location = getFreeRooms();
        //printf("Room %d is occupied by monster %d\n", monsters[m].location, monsters[m].id);
        setOccupied(monsters[m].location, 1);
        pthread_create(&monsterThreads[m], NULL, monsterLife, &monsters[m]);
    }

    /*--------------------------------------------------------------------------------------------------------------
        INICIA CÓDIGO SDL
    --------------------------------------------------------------------------------------------------------------*/

    // retorna 0 si SDL se inicializa con éxito, diferente de cero en caso contrario
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("Error inicializando SDL: %s\n", SDL_GetError());
	}

    // crea una ventana
	SDL_Window* win = SDL_CreateWindow("GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN, SCREEN, 0);

	// activa el programa que controla el hardware gráfico y establece las flags
	Uint32 render_flags = SDL_RENDERER_ACCELERATED;

	// crea el renderizador
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);

	// crea una superficie para cargar la imagen en memoria principal
	SDL_Surface* hero;
	SDL_Surface* wall;
	SDL_Surface* room;
	SDL_Surface* door;
    SDL_Surface* start;
	SDL_Surface* goal;
    SDL_Surface* game_over;
    SDL_Surface* you_win;
    SDL_Surface* monster;

	// ubicación de las imágenes
	hero = IMG_Load("hero.png");
	wall = IMG_Load("wall.jpg");
	room = IMG_Load("room.jpg");
	door = IMG_Load("wall.jpg");
    start = IMG_Load("start.png");
    goal = IMG_Load("goal.png");
    game_over = IMG_Load("gameOver.jpg");
    you_win = IMG_Load("win.jpg");
    monster = IMG_Load("monster.jpg");

	// carga la imagen en la memoria del hardware gráfico
	SDL_Texture* tex_hero = SDL_CreateTextureFromSurface(rend, hero);
	SDL_Texture* tex_wall = SDL_CreateTextureFromSurface(rend, wall);
	SDL_Texture* tex_room = SDL_CreateTextureFromSurface(rend, room);
	SDL_Texture* tex_door = SDL_CreateTextureFromSurface(rend, door);
    SDL_Texture* tex_start = SDL_CreateTextureFromSurface(rend, start);
	SDL_Texture* tex_goal = SDL_CreateTextureFromSurface(rend, goal);
    SDL_Texture* tex_game_over = SDL_CreateTextureFromSurface(rend, game_over);
    SDL_Texture* tex_win = SDL_CreateTextureFromSurface(rend, you_win);
    SDL_Texture* tex_monster = SDL_CreateTextureFromSurface(rend, monster);

	// limpia la memoria principal
	SDL_FreeSurface(hero);
	SDL_FreeSurface(wall);
	SDL_FreeSurface(room);
	SDL_FreeSurface(door);
    SDL_FreeSurface(start);
	SDL_FreeSurface(goal);
    SDL_FreeSurface(game_over);
    SDL_FreeSurface(you_win);
    SDL_FreeSurface(monster);

	// permite controlar la posición de los sprites en pantalla
	SDL_Rect dest_hero;
	SDL_Rect dest_wall;
	SDL_Rect dest_room;
	SDL_Rect dest_door;
    SDL_Rect dest_start;
	SDL_Rect dest_goal;
    SDL_Rect dest_game_over;
    SDL_Rect dest_you_win;
    SDL_Rect dest_monster;

	// conecta las texturas con dest para controlar su posición
	SDL_QueryTexture(tex_hero, NULL, NULL, &dest_hero.w, &dest_hero.h);
	SDL_QueryTexture(tex_wall, NULL, NULL, &dest_wall.w, &dest_wall.h);
	SDL_QueryTexture(tex_room, NULL, NULL, &dest_room.w, &dest_room.h);
	SDL_QueryTexture(tex_door, NULL, NULL, &dest_door.w, &dest_door.h);
    SDL_QueryTexture(tex_start, NULL, NULL, &dest_start.w, &dest_start.h);
    SDL_QueryTexture(tex_goal, NULL, NULL, &dest_goal.w, &dest_goal.h);
    SDL_QueryTexture(tex_game_over, NULL, NULL, &dest_game_over.w, &dest_game_over.h);
    SDL_QueryTexture(tex_win, NULL, NULL, &dest_you_win.w, &dest_you_win.h);
    SDL_QueryTexture(tex_monster, NULL, NULL, &dest_monster.w, &dest_monster.h);

	// ajusta el ancho y alto de los sprites

	dest_hero.w = CELL;
	dest_hero.h = CELL;

	dest_wall.w = CELL;
	dest_wall.h = CELL;

	dest_room.w = SCREEN;
	dest_room.h = SCREEN;

	dest_door.w /= DOOR;
	dest_door.h /= DOOR;

    dest_start.w = CELL;
    dest_start.h = CELL;

    dest_goal.w = CELL;
    dest_goal.h = CELL;

    dest_you_win.w /= 5;
    dest_you_win.h /= 5;

    //dest_game_over.w = 1;
    //dest_game_over.h  = 1;

    dest_monster.w = CELL;
    dest_monster.h = CELL;

	// establece la posición inicial en x del sprite
	dest_hero.x = start_i*CELL;

	// establece la posición inicial en y del sprite
	dest_hero.y = start_j*CELL;

    dest_game_over.x =  SCREEN / 3;
    dest_game_over.y = SCREEN / 4;

    dest_you_win.x =  SCREEN / 3;
    dest_you_win.y = SCREEN / 4;

	// controla el ciclo de animación
	int close = 0;

	// velocidad del sprite
	//int speed = 300;

    SDL_Point mousePosition;
  

	// ciclo de animación
	while (!close) {
		SDL_Event event;

        current_room_player = getRoomPointerByID(current_room_id);
        if(current_room_player->type == Goal){
            current_game_state = GAME_STATE_VICTORY;
            //close = 1;
        }

		// administración de eventos
		while (SDL_PollEvent(&event)) {
			switch (event.type) {

			case SDL_QUIT:
				// manejando el botón de cerrar
				close = 1;
				break;

			case SDL_KEYDOWN:
				// API de teclado para presionar teclas
                if(current_game_state == GAME_STATE_RUN){

                    switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                    case SDL_SCANCODE_UP:
                        next_room_id = getNeighbour(current_room_id, North);
                        if(next_room_id != -1) {
                            next_room_player = getRoomPointerByID(next_room_id);
                            if(next_room_player->type != Wall) {
                                current_room_player = getRoomPointerByID(current_room_id);
                                if(current_room_player->doors[North].state == 1) {
                                    current_room_player->occupied = false;
                                    next_room_player->occupied = true;
                                    current_room_id = next_room_id;
                                    //dest_hero.y -= CELL;
                                }
                                
                            }
                        }
                        break;
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:
                        next_room_id = getNeighbour(current_room_id, West);
                        if(next_room_id != -1) {
                            next_room_player = getRoomPointerByID(next_room_id);
                            if(next_room_player->type != Wall) {
                                current_room_player = getRoomPointerByID(current_room_id);
                                if(current_room_player->doors[West].state == 1) {
                                    current_room_player->occupied = false;
                                    next_room_player->occupied = true;
                                    current_room_id = next_room_id;
                                    //dest_hero.x -= CELL;
                                }
                            }
                        }
                        break;
                    case SDL_SCANCODE_S:
                    case SDL_SCANCODE_DOWN:
                        next_room_id = getNeighbour(current_room_id, South);
                        if(next_room_id != -1) {
                            next_room_player = getRoomPointerByID(next_room_id);
                            if(next_room_player->type != Wall) {
                                current_room_player = getRoomPointerByID(current_room_id);
                                if(current_room_player->doors[South].state == 1) {
                                    current_room_player->occupied = false;
                                    next_room_player->occupied = true;
                                    current_room_id = next_room_id;
                                    //dest_hero.y += CELL;
                                }
                            }
                        }
                        break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT:
                        next_room_id = getNeighbour(current_room_id, East);
                        if(next_room_id != -1) {
                            next_room_player = getRoomPointerByID(next_room_id);
                            if(next_room_player->type != Wall) {
                                current_room_player = getRoomPointerByID(current_room_id);
                                if(current_room_player->doors[East].state == 1) {
                                    current_room_player->occupied = false;
                                    next_room_player->occupied = true;
                                    current_room_id = next_room_id;
                                    //dest_hero.x += CELL;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
                default:
                    break;
			}
		}

		// límite derecho
		if (dest_hero.x + dest_hero.w > SCREEN)
			dest_hero.x = SCREEN - dest_hero.w;

		// límite izquierdo
		if (dest_hero.x < 0)
			dest_hero.x = 0;

		// límite inferior
		if (dest_hero.y + dest_hero.h > SCREEN)
			dest_hero.y = SCREEN - dest_hero.h;

		// límite superior
		if (dest_hero.y < 0)
			dest_hero.y = 0;

		// limpia la pantalla
		SDL_RenderClear(rend);

        switch(current_game_state){
            case GAME_STATE_OVER:
                SDL_RenderCopy(rend, tex_game_over, NULL, &dest_game_over);
                break;
            case GAME_STATE_VICTORY: 
                SDL_RenderCopy(rend, tex_win, NULL, &dest_you_win);
                break;
            case GAME_STATE_RUN:
                /*if(room_to_render->type == Start) {
                    dest_start.x = j*CELL;
                    dest_start.y = i*CELL;
                    SDL_RenderCopy(rend, tex_start, NULL, &dest_start);
                } else if(room_to_render->type == Goal) {
                    dest_goal.x = j*CELL;
                    dest_goal.y = i*CELL;
                    SDL_RenderCopy(rend, tex_goal, NULL, &dest_goal);
                } else {*/
                    //dest_room.x = j*CELL;
                    //dest_room.y = i*CELL;
                    SDL_RenderCopy(rend, tex_room, NULL, &dest_room);
                    for (int k = 0; k < 4; k++) {
                        if(current_room_player->doors[k].state == 1) {
                            switch(current_room_player->doors[k].cardinal) {
                                // North = 0, South = 1, East = 2, West = 3
                                case 0:
                                    dest_door.x = SCREEN/2; 
                                    dest_door.y = 0;
                                    break;
                                case 1:
                                    dest_door.x = SCREEN/2;  
                                    dest_door.y = SCREEN-dest_door.h;  
                                    break;
                                case 2:
                                    dest_door.x = SCREEN-dest_door.w;  
                                    dest_door.y = SCREEN/2;  
                                    break;
                                case 3:
                                    dest_door.x = 0; 
                                    dest_door.y = SCREEN/2; 
                                    break;
                                default:
                                    break;
                            }
                            SDL_RenderCopy(rend, tex_door, NULL, &dest_door);
                        }
                    }
                    if(current_room_player->occupied) {
                        dest_monster.x = 2*CELL;
                        dest_monster.y = 4*CELL;
                        SDL_RenderCopy(rend, tex_monster, NULL, &dest_monster);
                    }
                //}
                SDL_RenderCopy(rend, tex_hero, NULL, &dest_hero);
                break;
            default:
                break;
        }

		// activa los buffers dobles para renderizado múltiple
		SDL_RenderPresent(rend);

		// calcula 60 fotogramas por segundo
		SDL_Delay(1000 / 60);
	}

	// destruye las texturas
	SDL_DestroyTexture(tex_hero);
	SDL_DestroyTexture(tex_wall);
    SDL_DestroyTexture(tex_room);
	SDL_DestroyTexture(tex_door);
    SDL_DestroyTexture(tex_start);
	SDL_DestroyTexture(tex_goal);
    SDL_DestroyTexture(tex_game_over);
    SDL_DestroyTexture(tex_win);
    SDL_DestroyTexture(tex_monster);

	// destruye el renderizador
	SDL_DestroyRenderer(rend);

	// destruye la ventana
	SDL_DestroyWindow(win);
	
	// cierra SDL
	SDL_Quit();

    for(int i = 0; i < monsterCount; i++){
        pthread_join(monsterThreads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    
    return 0;
}