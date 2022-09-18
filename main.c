#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "mapGenerator.h"
#include "global.h"

#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

//pthread_mutex_t lock;
sem_t semaph;
struct monster *monsters;
struct hero *player;
int winner = 0;

int closeWindow = 0;

int monster0Movements[500];
int monster0MovementsIndex = 0;

int monster1Movements[500];
int monster1MovementsIndex = 0;

int heroMovements[500];
int heroMovementsIndex = 0;

int SCREEN_W;
int SCREEN_H;
int CELL;

unsigned short *get_screen_size(void)
{
    static unsigned short size[2];
    char *array[8];
    char screen_size[64];
    char* token = NULL;

    FILE *cmd = popen("xdpyinfo | awk '/dimensions/ {print $2}'", "r");

    if (!cmd)
        return 0;

    while (fgets(screen_size, sizeof(screen_size), cmd) != NULL);
    pclose(cmd);

    token = strtok(screen_size, "x\n");

    if (!token)
        return 0;

    for (unsigned short i = 0; token != NULL; ++i) {
        array[i] = token;
        token = strtok(NULL, "x\n");
    }
    size[0] = atoi(array[0]);
    size[1] = atoi(array[1]);
    size[2] = -1;

    return size;
}

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

int getNextRoomForMonster(int id, int monsterID)
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
                pthread_mutex_lock(&lock); //Aqui el hilo monstruo obtiene el estado de una habitacion de manera sincronizada y revisa si esta ocupada o no
                tempNeighbour = getRoomPointerByID(neighbourID); 
                if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Start && tempNeighbour->type != Goal && tempNeighbour->type != Wall){
                    canProceedToThisRoom = 1;
                    setOccupied(id, 0);
                    setOccupied(neighbourID, 1);
                    setMonsterID(id, -1);
                    setMonsterID(neighbourID, monsterID);
                    //getRoomDetailsByID(id);
                    //getRoomDetailsByID(neighbourID);
                    //printf("\nMonster %d moved from room %d to room %d\n", monsterID, id, neighbourID);
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

int openChest(struct hero *h){
    if (getRoomType(h->location) == 1 && getRoomChestState(h->location) == 1) {  
        int r = rand() % 2;
        if (r == 0) {
            printf("before Hero opened a chest and got +1 attack, actual attack: %d\n", h->atk);
            h->atk = h->atk + 1;
            printf("after Hero opened a chest and got +1 attack, actual attack: %d\n", h->atk);
        } else {
            h->hp = h->hp + 1;
            printf("Hero has %d hp\n", h->hp);
        }
        setRoomChestState(h->location, 0);
        return 1;
    } else {
        printf("There is no chest in this room\n");
        return 0;
    }
}

void * monsterLife(void * m){
    struct monster * mon = (struct monster *) m;
    int freeNSize = 0;
    int freeNeighbours[4];
    int action;
    //printf("\nMonster %d is alive\n", mon->id);
    //while(mon->hp != 0){

    printf("Monster %d is alive\n", mon->id);

    while(mon->hp > 0 && player->hp > 0){
        //printf("\nLooking for free neighbours of %d in %d\n", mon->id, mon->location);
        if (getRoomPointerByID(mon->location)->isHeroInRoom == 1) {
            action = rand() % 2;
        } else {
            action = 0;
        }

        if (action){
            printf("Monster is attacking\n");
            // printf("Hola21\n");
            pthread_mutex_lock(&lock);
            attackHero(mon, player);
            pthread_mutex_unlock(&lock);
            // printf("Hola22\n");
        } else {
            int loc = getNextRoomForMonster(mon->location, mon->id);
            if(loc!=-1){ //Si hay una habitacion libre
                monsterMove(mon, loc);
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
        }
        

        //sem_post(&semaph);
        sleep((rand()%2)+1);
        if(current_game_state == GAME_STATE_OVER || current_game_state == GAME_STATE_VICTORY 
            || closeWindow == 1){
            pthread_exit(0);
        }
        // ---------------------------------------------------------
    }
    
    printf("Monster %d is dead\n", mon->id);

    pthread_mutex_lock(&lock); //Aqui se libera la habitación en la que el monstruo murio
    setOccupied(mon->location, 0);
    setMonsterID(mon->location, -1);
    pthread_mutex_unlock(&lock);

    sleep(3);
    pthread_exit(0);
}

void* heroLife(void* h)
{
    struct hero * hero = (struct hero *) h;
    int freeNSize = 0;
    int freeNeighbours[4];

    while(hero->hp > 0){

        if (getRoomPointerByID(hero->location)->type == Goal) {
            winner = 1;
            printf("Hero has won\n");
            sleep(3);
            hero->hp = 0;
            pthread_exit(0);
        } else {
            char input = getchar();
            while ('\n' != getchar());

            /*if(input == 'w'){
                if (isDoorOpen(hero->location, 0))
                {
                    int neighbourID = getNeighbour(hero->location, 0);
                    struct room *tempNeighbour = getRoomPointerByID(neighbourID);

                    pthread_mutex_lock(&lock);
                    if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall){
                        setHeroInRoom(hero->location, 0);
                        setHeroInRoom(neighbourID, 1);
                        heroMove(hero, neighbourID);

                        heroMovements[heroMovementsIndex] = neighbourID;
                        heroMovementsIndex++;

                        printf("Hero moved to room %d\n", hero->location);
                    }else{
                        printf("Hero can't move to room %d\n", neighbourID);
                    }
                    pthread_mutex_unlock(&lock);
                }else{
                    printf("Door is closed\n");
                }
            }else if(input == 'a'){
                if (isDoorOpen(hero->location, 3))
                {
                    int neighbourID = getNeighbour(hero->location, 3);
                    struct room *tempNeighbour = getRoomPointerByID(neighbourID);

                    pthread_mutex_lock(&lock);
                    if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall){
                        setHeroInRoom(hero->location, 0);
                        setHeroInRoom(neighbourID, 1);
                        heroMove(hero, neighbourID);
                        printf("Hero moved to room %d\n", hero->location);
                    }else{
                        printf("Hero can't move to room %d\n", neighbourID);
                    }
                    pthread_mutex_unlock(&lock);
                }else{
                    printf("Door is closed\n");
                }
            }else if(input == 's'){
                if (isDoorOpen(hero->location, 1))
                {
                    int neighbourID = getNeighbour(hero->location, 1);
                    struct room *tempNeighbour = getRoomPointerByID(neighbourID);

                    pthread_mutex_lock(&lock);
                    if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall){
                        setHeroInRoom(hero->location, 0);
                        setHeroInRoom(neighbourID, 1);
                        heroMove(hero, neighbourID);
                        printf("Hero moved to room %d\n", hero->location);
                    }else{
                        printf("Hero can't move to room %d\n", neighbourID);
                    }
                    pthread_mutex_unlock(&lock);
                }else{
                    printf("Door is closed\n");
                }
            }else if(input == 'd'){
                if (isDoorOpen(hero->location, 2))
                {
                    int neighbourID = getNeighbour(hero->location, 2);
                    struct room *tempNeighbour = getRoomPointerByID(neighbourID);

                    pthread_mutex_lock(&lock);
                    if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall){
                        setHeroInRoom(hero->location, 0);
                        setHeroInRoom(neighbourID, 1);
                        heroMove(hero, neighbourID);
                        printf("Hero moved to room %d\n", hero->location);
                    }else{
                        printf("Hero can't move to room %d\n", neighbourID);
                    }
                    pthread_mutex_unlock(&lock);
                }else{
                    printf("Door is closed\n");
                }
            }
            else */if(input == ' '){
                printf("Hero is attacking\n");
                // printf("Hola1\n");
                pthread_mutex_lock(&lock);
                int monsterID = getRoomPointerByID(hero->location)->monsterInRoomID;
                if (monsterID != -1)
                {
                    // printf("Hola2\n");
                    struct monster *monster = &monsters[monsterID];
                    if (monster->state == IDLE){
                        attackMonster(hero, monster);
                        // printf("Hola3\n");
                    } else {
                        printf("Monster is not idle\n");
                    }
                }
                pthread_mutex_unlock(&lock);
                // printf("Hola4\n");
                
            }/*else if(input == 'e'){
                openChest(hero);
            }*/

            sleep(0.1);
        } 
        // // ---------------------------------------------------------
    }
    
    printf("Hero is dead\n");
    pthread_exit(0);
}

int main(){

    srand(time(NULL));
	struct room *room_to_render;
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

    switch (eleccion){
        case 1:
            N = EASY;
            break;
        case 2:
            N = MEDIUM;
            break;
        case 3:
            N = HARD;
            break;
    }

    unsigned short *screen_size = get_screen_size();
    SCREEN_W = screen_size[0]-50;
    SCREEN_H = screen_size[1]-65;
    CELL = 120;
    
    int startLocation=generateMap(N);
    
    while(startLocation==-1){
        startLocation=generateMap(N);
        printf("Reintentando generar mapa...\n");
    }

    pthread_mutex_init(&lock, NULL);

    player = (struct hero *) malloc(sizeof(struct hero));
    player->hp = 5;
    player->atk = 1;
    player->location = startLocation;
    setHeroInRoom(startLocation, 1);

    int monsterCount = 2;
    monsters = (struct monster *)malloc(sizeof(struct monster)*monsterCount);
    pthread_t * heroThread = (pthread_t *) malloc(sizeof(pthread_t));

    pthread_t * monsterThreads;
    monsterThreads = (pthread_t *)malloc(sizeof(pthread_t)*monsterCount);

    for(int m = 0; m < monsterCount; m++){
        monsters[m].id = m;
        monsters[m].hp = 3;
        monsters[m].atk = 1;
        monsters[m].state = IDLE;
        monsters[m].location = getFreeRooms();
        //printf("Room %d is occupied by monster %d\n", monsters[m].location, monsters[m].id);
        setOccupied(monsters[m].location, 1);
        setMonsterID(monsters[m].location, monsters[m].id);

        pthread_create(&monsterThreads[m], NULL, monsterLife, &monsters[m]);
    }

    pthread_create(&heroThread, NULL, heroLife, player);
    
    //--------------------------------------------------------------------------------------------------------------
    //    INICIA CÓDIGO SDL
    //--------------------------------------------------------------------------------------------------------------

    // retorna 0 si SDL se inicializa con éxito, diferente de cero en caso contrario
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("Error inicializando SDL: %s\n", SDL_GetError());
	}

    // crea una ventana
	SDL_Window* win = SDL_CreateWindow("GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, 0);

	// activa el programa que controla el hardware gráfico y establece las flags
	Uint32 render_flags = SDL_RENDERER_ACCELERATED;

	// crea el renderizador
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);

    //Texto
    if (TTF_Init() < 0) {
	    printf("No se inicializó SDL TTF: %s\n", SDL_GetError());
	    exit(1);
    }

    char hero_life_text_string[3];
    char hero_attack_text_string[3];

    TTF_Font* sans = TTF_OpenFont("fonts/sans.ttf", 24);
	if ( !sans ) {
		printf("Error al cargar la fuente.\n");
		exit(1);
	}

    SDL_Color font_color = {255, 255, 255}; 

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
    SDL_Surface* open_chest;
    SDL_Surface* closed_chest;
    SDL_Surface* trap;
    SDL_Surface* hero_life_text;
    SDL_Surface* hero_life_icon;
    SDL_Surface* hero_attack_text;
    SDL_Surface* hero_attack_icon;

	// ubicación de las imágenes
	hero = IMG_Load("img/hero.png");
	wall = IMG_Load("img/wall.jpg");
	room = SDL_LoadBMP("img/room.bmp");
	door = IMG_Load("img/door.png");
    start = IMG_Load("img/start.png");
    goal = IMG_Load("img/goal.png");
    game_over = IMG_Load("img/gameOver.jpg");
    you_win = IMG_Load("img/win.jpg");
    monster = IMG_Load("img/monster.jpg");
    open_chest = IMG_Load("img/open_chest.png");
    closed_chest = IMG_Load("img/closed_chest.png");
    trap = IMG_Load("img/trap.png");
    hero_life_icon = IMG_Load("img/heart.png");
    hero_attack_icon = IMG_Load("img/attack.png");

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
    SDL_Texture* tex_open_chest = SDL_CreateTextureFromSurface(rend, open_chest);
    SDL_Texture* tex_closed_chest = SDL_CreateTextureFromSurface(rend, closed_chest);
    SDL_Texture* tex_trap = SDL_CreateTextureFromSurface(rend, trap);
    SDL_Texture* tex_hero_life_icon = SDL_CreateTextureFromSurface(rend, hero_life_icon);
    SDL_Texture* tex_hero_attack_icon = SDL_CreateTextureFromSurface(rend, hero_attack_icon);

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
    SDL_FreeSurface(open_chest);
    SDL_FreeSurface(closed_chest);
    SDL_FreeSurface(trap);
    SDL_FreeSurface(hero_life_icon);
    SDL_FreeSurface(hero_attack_icon);

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
    SDL_Rect dest_open_chest;
    SDL_Rect dest_closed_chest;
    SDL_Rect dest_trap;
    SDL_Rect dest_hero_life_text;
    SDL_Rect dest_hero_life_icon;
    SDL_Rect dest_hero_attack_text;
    SDL_Rect dest_hero_attack_icon;

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
    SDL_QueryTexture(tex_open_chest, NULL, NULL, &dest_open_chest.w, &dest_open_chest.h);
    SDL_QueryTexture(tex_closed_chest, NULL, NULL, &dest_closed_chest.w, &dest_closed_chest.h);
    SDL_QueryTexture(tex_trap, NULL, NULL, &dest_trap.w, &dest_trap.h);
    SDL_QueryTexture(tex_hero_life_icon, NULL, NULL, &dest_hero_life_icon.w, &dest_hero_life_icon.h);
    SDL_QueryTexture(tex_hero_attack_icon, NULL, NULL, &dest_hero_attack_icon.w, &dest_hero_attack_icon.h);

	// ajusta el ancho y alto de los sprites

	dest_hero.w = CELL;
	dest_hero.h = CELL;

	dest_wall.w = CELL;
	dest_wall.h = CELL;

	dest_room.w = SCREEN_W;
	dest_room.h = SCREEN_H;

	dest_door.w = CELL;
	dest_door.h = CELL;

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

    dest_open_chest.w = CELL;
    dest_open_chest.h = CELL;
    dest_closed_chest.w = CELL;
    dest_closed_chest.h = CELL;
    dest_trap.w = CELL;
    dest_trap.h = CELL;

    dest_hero_life_text.x = 60;  
    dest_hero_life_text.y = 20; 
    dest_hero_life_text.w = 35; 
    dest_hero_life_text.h = 35;

    dest_hero_life_icon.x = 20;  
    dest_hero_life_icon.y = 20; 
    dest_hero_life_icon.w = 35;
    dest_hero_life_icon.h = 35;

    dest_hero_attack_text.x = 170;  
    dest_hero_attack_text.y = 20; 
    dest_hero_attack_text.w = 35; 
    dest_hero_attack_text.h = 35;

    dest_hero_attack_icon.x = 130;  
    dest_hero_attack_icon.y = 20; 
    dest_hero_attack_icon.w = 35;
    dest_hero_attack_icon.h = 35;

    current_room_id = player->location;

	// establece la posición inicial en x del sprite
	dest_hero.x = SCREEN_W/2-SCREEN_W/4;

	// establece la posición inicial en y del sprite
	dest_hero.y = SCREEN_H/2-SCREEN_H/4;

    dest_game_over.x =  SCREEN_W / 3;
    dest_game_over.y = SCREEN_H / 4;

    dest_you_win.x =  SCREEN_W / 3;
    dest_you_win.y = SCREEN_H / 4;

	// controla el ciclo de animación
	

	// velocidad del sprite
	//int speed = 300;

    SDL_Point mousePosition;
  

	// ciclo de animación
	while (!closeWindow) {
		SDL_Event event;

        current_room_player = getRoomPointerByID(current_room_id);
        if(current_room_player->type == Goal){
            current_game_state = GAME_STATE_VICTORY;
            //closeWindow = 1;
        }
        if(player->hp <= 0){
            current_game_state = GAME_STATE_OVER;
        }

		// administración de eventos
		while (SDL_PollEvent(&event)) {
			switch (event.type) {

			case SDL_QUIT:
				// manejando el botón de cerrar
				closeWindow = 1;
				break;

			case SDL_KEYDOWN:
				// API de teclado para presionar teclas
                if(current_game_state == GAME_STATE_RUN){

                    switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                    case SDL_SCANCODE_UP:
                        if (isDoorOpen(current_room_id, 0))
                        {
                            int neighbourID = getNeighbour(current_room_id, 0);
                            struct room *tempNeighbour = getRoomPointerByID(neighbourID);

                            //pthread_mutex_lock(&lock);
                            if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall){
                                pthread_mutex_lock(&lock);
                                setHeroInRoom(current_room_id, 0);
                                setHeroInRoom(neighbourID, 1);
                                heroMove(player, neighbourID);
                                pthread_mutex_unlock(&lock);
                                heroMovements[heroMovementsIndex] = neighbourID;
                                heroMovementsIndex++;
                                current_room_id = neighbourID;
                                //dest_hero.y -= CELL;

                                printf("Hero moved to room %d\n", player->location);
                            }else{
                                printf("Hero can't move to room %d\n", neighbourID);
                            }
                            //pthread_mutex_unlock(&lock);
                        }else{
                            printf("Door is closed\n");
                        }
                        break;
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:
                        if (isDoorOpen(current_room_id, 3))
                        {
                            int neighbourID = getNeighbour(current_room_id, 3);
                            struct room *tempNeighbour = getRoomPointerByID(neighbourID);

                            //pthread_mutex_lock(&lock);
                            if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall){
                                pthread_mutex_lock(&lock);
                                setHeroInRoom(current_room_id, 0);
                                setHeroInRoom(neighbourID, 1);
                                heroMove(player, neighbourID);
                                pthread_mutex_unlock(&lock);
                                heroMovements[heroMovementsIndex] = neighbourID;
                                heroMovementsIndex++;
                                current_room_id = neighbourID;
                                //dest_hero.x -= CELL;

                                printf("Hero moved to room %d\n", player->location);
                            }else{
                                printf("Hero can't move to room %d\n", neighbourID);
                            }
                            //pthread_mutex_unlock(&lock);
                        }else{
                            printf("Door is closed\n");
                        }
                        break;
                    case SDL_SCANCODE_S:
                    case SDL_SCANCODE_DOWN:
                        if (isDoorOpen(current_room_id, 1))
                        {
                            int neighbourID = getNeighbour(current_room_id, 1);
                            struct room *tempNeighbour = getRoomPointerByID(neighbourID);

                            //pthread_mutex_lock(&lock);
                            if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall){
                                pthread_mutex_lock(&lock);
                                setHeroInRoom(current_room_id, 0);
                                setHeroInRoom(neighbourID, 1);
                                heroMove(player, neighbourID);
                                pthread_mutex_unlock(&lock);
                                heroMovements[heroMovementsIndex] = neighbourID;
                                heroMovementsIndex++;
                                current_room_id = neighbourID;
                                //dest_hero.y += CELL;

                                printf("Hero moved to room %d\n", player->location);
                            }else{
                                printf("Hero can't move to room %d\n", neighbourID);
                            }
                            //pthread_mutex_unlock(&lock);
                        }else{
                            printf("Door is closed\n");
                        }
                        break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT:
                        if (isDoorOpen(current_room_id, 2))
                        {
                            int neighbourID = getNeighbour(current_room_id, 2);
                            struct room *tempNeighbour = getRoomPointerByID(neighbourID);

                            //pthread_mutex_lock(&lock);
                            if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall){
                                pthread_mutex_lock(&lock);
                                setHeroInRoom(current_room_id, 0);
                                setHeroInRoom(neighbourID, 1);
                                heroMove(player, neighbourID);
                                pthread_mutex_unlock(&lock);
                                heroMovements[heroMovementsIndex] = neighbourID;
                                heroMovementsIndex++;
                                current_room_id = neighbourID;
                                //dest_hero.x += CELL;

                                printf("Hero moved to room %d\n", player->location);
                            }else{
                                printf("Hero can't move to room %d\n", neighbourID);
                            }
                            //pthread_mutex_unlock(&lock);
                        }else{
                            printf("Door is closed\n");
                        }
                        break;
                    case SDL_SCANCODE_E:
                        openChest(player);
                        break;
                    case SDL_SCANCODE_SPACE:
                        printf("space pressed \n");
                        printf("Hero is attacking, atk: %d\n", player->atk);
                        printf("Hola1\n");
                        pthread_mutex_lock(&lock);
                        int monsterID = getRoomPointerByID(current_room_id)->monsterInRoomID;
                        if (monsterID != -1)
                        {
                            printf("Hola2\n");
                            struct monster *monster = &monsters[monsterID];
                            if (monster->state == IDLE){
                                attackMonster(player, monster);
                                printf("Hola3\n");
                            } else {
                                printf("Monster is not idle\n");
                            }
                        }
                        pthread_mutex_unlock(&lock);
                        break;
                    default:
                        break;
                    }
                }
                default:
                    break;
			}
		}

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
                SDL_RenderCopy(rend, tex_room, NULL, &dest_room);
                for (int k = 0; k < 4; k++) {
                    if(current_room_player->doors[k].state == 1) {
                        switch(current_room_player->doors[k].cardinal) {
                            // North = 0, South = 1, East = 2, West = 3
                            case 0:
                                dest_door.x = SCREEN_W/2; 
                                dest_door.y = 0;
                                break;
                            case 1:
                                dest_door.x = SCREEN_W/2;  
                                dest_door.y = SCREEN_H-dest_door.h;  
                                break;
                            case 2:
                                dest_door.x = SCREEN_W-dest_door.w;  
                                dest_door.y = SCREEN_H/2;  
                                break;
                            case 3:
                                dest_door.x = 0; 
                                dest_door.y = SCREEN_H/2; 
                                break;
                            default:
                                break;
                        }
                        SDL_RenderCopy(rend, tex_door, NULL, &dest_door);
                    }
                }
                if(current_room_player->occupiedByMonster) {
                    dest_monster.x = (SCREEN_W/2-CELL)+SCREEN_W/4;
                    dest_monster.y = SCREEN_H/2-SCREEN_H/4;
                    SDL_RenderCopy(rend, tex_monster, NULL, &dest_monster);
                }
                if(current_room_player->type == Treasure && current_room_player->treasure){
                    dest_closed_chest.x = (SCREEN_W/2-CELL)+SCREEN_W/4;
                    dest_closed_chest.y = (SCREEN_H/2-CELL)+SCREEN_H/4;
                    SDL_RenderCopy(rend, tex_closed_chest, NULL, &dest_closed_chest);
                }
                if(current_room_player->type == Treasure && !current_room_player->treasure){
                    dest_open_chest.x = (SCREEN_W/2-CELL)+SCREEN_W/4;
                    dest_open_chest.y = (SCREEN_H/2-CELL)+SCREEN_H/4;
                    SDL_RenderCopy(rend, tex_open_chest, NULL, &dest_open_chest);
                }
                if (current_room_player->type == Trap && current_room_player->activated_trap == 1) {
                    dest_trap.x = SCREEN_W/2-SCREEN_W/4;
                    dest_trap.y = (SCREEN_H/2-CELL)+SCREEN_H/4;
                    SDL_RenderCopy(rend, tex_trap, NULL, &dest_trap);
                }
                SDL_RenderCopy(rend, tex_hero, NULL, &dest_hero);
                
                // Vidas del héroe
                sprintf(hero_life_text_string, "%d", player->hp);
                hero_life_text = TTF_RenderText_Solid(sans, hero_life_text_string, font_color);
                SDL_Texture* tex_hero_life_text = SDL_CreateTextureFromSurface(rend, hero_life_text);

                SDL_RenderCopy(rend, tex_hero_life_text, NULL, &dest_hero_life_text);
                SDL_RenderCopy(rend, tex_hero_life_icon, NULL, &dest_hero_life_icon);
                SDL_DestroyTexture(tex_hero_life_text);
                SDL_FreeSurface(hero_life_text);

                // Ataques disponibles del héroe
                sprintf(hero_attack_text_string, "%d", player->atk);
                hero_attack_text = TTF_RenderText_Solid(sans, hero_attack_text_string, font_color);
                SDL_Texture* tex_hero_attack_text = SDL_CreateTextureFromSurface(rend, hero_attack_text);

                SDL_RenderCopy(rend, tex_hero_attack_text, NULL, &dest_hero_attack_text);
                SDL_RenderCopy(rend, tex_hero_attack_icon, NULL, &dest_hero_attack_icon);
                SDL_DestroyTexture(tex_hero_attack_text);
                SDL_FreeSurface(hero_attack_text);
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
    SDL_DestroyTexture(tex_open_chest);
    SDL_DestroyTexture(tex_closed_chest);
    SDL_DestroyTexture(tex_trap);

	// destruye el renderizador
	SDL_DestroyRenderer(rend);

	// destruye la ventana
	SDL_DestroyWindow(win);
	
	// cierra SDL
	SDL_Quit();

    for(int i = 0; i < monsterCount; i++){
        pthread_join(monsterThreads[i], NULL);
    }

    printf("All monsters are dead\n");

    printf("HeroHP is dead\n");
    pthread_cancel(heroThread);

    pthread_join(heroThread, NULL);

    printf("HeroThread is dead\n");

    pthread_mutex_destroy(&lock);
    
    return 0;
}