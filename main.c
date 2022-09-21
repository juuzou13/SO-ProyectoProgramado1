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

pthread_mutex_t lock;
pthread_mutex_t attackingLock;
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
    char *token = NULL;

    FILE *cmd = popen("xdpyinfo | awk '/dimensions/ {print $2}'", "r");

    if (!cmd)
        return 0;

    while (fgets(screen_size, sizeof(screen_size), cmd) != NULL)
        ;
    pclose(cmd);

    token = strtok(screen_size, "x\n");

    if (!token)
        return 0;

    for (unsigned short i = 0; token != NULL; ++i)
    {
        array[i] = token;
        token = strtok(NULL, "x\n");
    }
    size[0] = atoi(array[0]);
    size[1] = atoi(array[1]);
    size[2] = -1;

    return size;
}

void randomDestination(int *destinations)
{
    int a[4] = {0, 1, 2, 3};

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
    pthread_mutex_t currLock;
    for (int i = 0; i < 4; i++)
    {
        canProceedToThisRoom = 0;
        currentIndex = destinations[i];
        neighbourID = getNeighbour(id, currentIndex);
        if (neighbourID != -1)
        {
            if (tempRoom->doors[currentIndex].state == Open)
            {

                tempNeighbour = getRoomPointerByID(neighbourID);
                currLock = tempNeighbour->room_lock;
                pthread_mutex_lock(&currLock);

                if (tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Start && tempNeighbour->type != Goal && tempNeighbour->type != Wall)
                {
                    canProceedToThisRoom = 1;
                    setOccupied(id, 0);
                    setOccupied(neighbourID, 1);
                    setMonsterID(id, -1);
                    setMonsterID(neighbourID, monsterID);
                }
                pthread_mutex_unlock(&currLock);
            }

            if (canProceedToThisRoom)
            {
                return neighbourID;
            }
        }
    }
    return -1;
}

int openChest(struct hero *h)
{
    if (getRoomType(h->location) == 1 && getRoomChestState(h->location) == 1)
    {
        int r = rand() % 2;
        if (r == 0)
        {
            printf("before Hero opened a chest and got +1 attack, actual attack: %d\n", h->atk);
            h->atk = h->atk + 1;
            printf("after Hero opened a chest and got +1 attack, actual attack: %d\n", h->atk);
        }
        else
        {
            h->hp = h->hp + 1;
            printf("Hero has %d hp\n", h->hp);
        }
        setRoomChestState(h->location, 0);
        return 1;
    }
    else
    {
        printf("There is no chest in this room\n");
        return 0;
    }
}

void *monsterLife(void *m)
{
    struct monster *mon = (struct monster *)m;
    int freeNSize = 0;
    int freeNeighbours[4];
    int action;
    pthread_mutex_t currHeroLock;

    printf("Monster %d is alive\n", mon->id);

    while (mon->hp > 0 && player->hp > 0)
    {

        if (getRoomPointerByID(mon->location)->isHeroInRoom == 1)
        {
            action = rand() % 2;
        }
        else
        {
            action = 0;
        }

        currHeroLock = player->heroLock;

        if (action)
        {
            printf("Monster is attacking\n");

            pthread_mutex_lock(&currHeroLock);
            attackHero(mon, player);
            pthread_mutex_unlock(&currHeroLock);
        }
        else
        {
            int loc = getNextRoomForMonster(mon->location, mon->id);
            if (loc != -1)
            {
                monsterMove(mon, loc);

                if (mon->id == 0)
                {
                    monster0Movements[monster0MovementsIndex] = loc;
                    monster0MovementsIndex++;
                }
                else if (mon->id == 1)
                {
                    monster1Movements[monster1MovementsIndex] = loc;
                    monster1MovementsIndex++;
                }
            }
            else
            {
                changeMonsterState(mon, IDLE);
            }
        }

        sleep((rand() % 2) + 1);
        if (current_game_state == GAME_STATE_OVER || current_game_state == GAME_STATE_VICTORY || closeWindow == 1)
        {
            pthread_exit(0);
        }
    }

    printf("Monster %d is dead\n", mon->id);

    pthread_mutex_t currLock = getRoomPointerByID(mon->location)->room_lock;

    pthread_mutex_lock(&currLock);
    setOccupied(mon->location, 0);
    setMonsterID(mon->location, -1);
    pthread_mutex_unlock(&currLock);

    sleep(3);
    pthread_exit(0);
}

void *heroLife(void *h)
{
    struct hero *hero = (struct hero *)h;
    int freeNSize = 0;
    int freeNeighbours[4];

    while (hero->hp > 0)
    {

        

        if (getRoomPointerByID(hero->location)->type == Goal)
        {
            winner = 1;
            printf("Hero has won\n");
            sleep(3);
            hero->hp = 0;
            pthread_exit(0);
        }
        //system("clear");
        //drawTemporalMap();
        
    }

    printf("Hero is dead\n");
    pthread_exit(0);
}

void safelyMoveHero(int roomToMove)
{
    int neighbourID = getNeighbour(current_room_id, roomToMove);

    struct room *tempNeighbour = getRoomPointerByID(neighbourID);
    pthread_mutex_t currRoomLock = tempNeighbour->room_lock;

    pthread_mutex_lock(&currRoomLock);

    if (tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Wall)
    {
        setHeroInRoom(current_room_id, 0);
        setHeroInRoom(neighbourID, 1);
        heroMove(player, neighbourID);
        sleep(0.1);

        heroMovements[heroMovementsIndex] = neighbourID;
        heroMovementsIndex++;
        current_room_id = neighbourID;

        printf("Hero moved to room %d\n", player->location);
    }
    else
    {
        if(tempNeighbour->type == Wall){
            printf("Hero can't move, there is a wall\n");
        }
        else{
            printf("Hero can't move to this room, there is a monster\n");
        }
    }

    pthread_mutex_unlock(&currRoomLock);
}

int slashed;
int prevHealth;

int main()
{

    srand(time(NULL));
    slashed = 0;
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
        break;
    case 2:
        N = MEDIUM;
        break;
    case 3:
        N = HARD;
        break;
    }

    unsigned short *screen_size = get_screen_size();
    SCREEN_W = screen_size[0] * 0.75;
    SCREEN_H = screen_size[1] * 0.75;
    CELL = SCREEN_W/16;

    int startLocation = generateMap(N);

    while (startLocation == -1)
    {
        startLocation = generateMap(N);
        printf("Reintentando generar mapa...\n");
    }

    pthread_mutex_init(&lock, NULL);

    player = (struct hero *)malloc(sizeof(struct hero));
    player->hp = 5;
    player->atk = 1;
    player->location = startLocation;
    setHeroInRoom(startLocation, 1);
    prevHealth = player->hp;

    int monsterCount = N / 2;
    monsters = (struct monster *)malloc(sizeof(struct monster) * monsterCount);
    pthread_t *heroThread = (pthread_t *)malloc(sizeof(pthread_t));

    pthread_t *monsterThreads;
    monsterThreads = (pthread_t *)malloc(sizeof(pthread_t) * monsterCount);

    for (int m = 0; m < monsterCount; m++)
    {
        monsters[m].id = m;
        monsters[m].hp = 3;
        monsters[m].atk = 1;
        monsters[m].state = IDLE;
        monsters[m].location = getFreeRooms();

        setOccupied(monsters[m].location, 1);
        setMonsterID(monsters[m].location, monsters[m].id);

        pthread_create(&monsterThreads[m], NULL, monsterLife, &monsters[m]);
    }

    pthread_create(&heroThread, NULL, heroLife, player);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("Error inicializando SDL: %s\n", SDL_GetError());
    }

    SDL_Window *win = SDL_CreateWindow("GAME", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, 0);

    Uint32 render_flags = SDL_RENDERER_ACCELERATED;

    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, render_flags);

    if (TTF_Init() < 0)
    {
        printf("No se inicializó SDL TTF: %s\n", SDL_GetError());
        exit(1);
    }

    char hero_life_text_string[3];
    char hero_attack_text_string[3];

    TTF_Font *sans = TTF_OpenFont("fonts/sans.ttf", 24);
    if (!sans)
    {
        printf("Error al cargar la fuente.\n");
        exit(1);
    }

    SDL_Color font_color = {255, 255, 255};

    SDL_Surface *hero;
    SDL_Surface *heroDMG;

    SDL_Surface *wall;
    SDL_Surface *room;

    SDL_Surface *door;
    SDL_Surface *doorN;
    SDL_Surface *doorS;
    SDL_Surface *doorE;
    SDL_Surface *doorW;

    SDL_Surface *slash;
    
    SDL_Surface *start;
    SDL_Surface *goal;
    SDL_Surface *game_over;
    SDL_Surface *you_win;
    SDL_Surface *monster;
    SDL_Surface *open_chest;
    SDL_Surface *closed_chest;
    SDL_Surface *trap;
    SDL_Surface *hero_life_text;
    SDL_Surface *hero_life_icon;
    SDL_Surface *hero_attack_text;
    SDL_Surface *hero_attack_icon;

    hero = IMG_Load("img/knight.png");
    heroDMG = IMG_Load("img/redKnight.png");
    wall = IMG_Load("img/wall.jpg");
    room = IMG_Load("img/dungeonfloor.png");

    door = IMG_Load("img/door.png");

    doorN = IMG_Load("img/N.jpg");
    doorS = IMG_Load("img/S.jpg");
    doorE = IMG_Load("img/E.jpg");
    doorW = IMG_Load("img/W.jpg");

    slash = IMG_Load("img/slash.png");

    start = IMG_Load("img/start.png");
    goal = IMG_Load("img/goal.png");
    game_over = IMG_Load("img/gameOver.jpg");
    you_win = IMG_Load("img/win.jpg");
    monster = IMG_Load("img/skeleton.jpg");
    open_chest = IMG_Load("img/open_chest.png");
    closed_chest = IMG_Load("img/closed_chest.png");
    trap = IMG_Load("img/fire.png");
    hero_life_icon = IMG_Load("img/heart.png");
    hero_attack_icon = IMG_Load("img/attack.png");

    SDL_Texture *tex_hero = SDL_CreateTextureFromSurface(rend, hero);
    SDL_Texture *tex_heroDMG = SDL_CreateTextureFromSurface(rend, heroDMG);


    SDL_Texture *tex_wall = SDL_CreateTextureFromSurface(rend, wall);
    SDL_Texture *tex_room = SDL_CreateTextureFromSurface(rend, room);
    SDL_Texture *tex_door = SDL_CreateTextureFromSurface(rend, door);

    SDL_Texture *tex_doorN = SDL_CreateTextureFromSurface(rend, doorN);
    SDL_Texture *tex_doorS = SDL_CreateTextureFromSurface(rend, doorS);
    SDL_Texture *tex_doorE = SDL_CreateTextureFromSurface(rend, doorE);
    SDL_Texture *tex_doorW = SDL_CreateTextureFromSurface(rend, doorW);

    SDL_Texture *tex_slash = SDL_CreateTextureFromSurface(rend, slash);

    SDL_Texture *tex_start = SDL_CreateTextureFromSurface(rend, start);
    SDL_Texture *tex_goal = SDL_CreateTextureFromSurface(rend, goal);
    SDL_Texture *tex_game_over = SDL_CreateTextureFromSurface(rend, game_over);
    SDL_Texture *tex_win = SDL_CreateTextureFromSurface(rend, you_win);
    SDL_Texture *tex_monster = SDL_CreateTextureFromSurface(rend, monster);
    SDL_Texture *tex_open_chest = SDL_CreateTextureFromSurface(rend, open_chest);
    SDL_Texture *tex_closed_chest = SDL_CreateTextureFromSurface(rend, closed_chest);
    SDL_Texture *tex_trap = SDL_CreateTextureFromSurface(rend, trap);
    SDL_Texture *tex_hero_life_icon = SDL_CreateTextureFromSurface(rend, hero_life_icon);
    SDL_Texture *tex_hero_attack_icon = SDL_CreateTextureFromSurface(rend, hero_attack_icon);

    SDL_FreeSurface(hero);
    SDL_FreeSurface(heroDMG);
    SDL_FreeSurface(wall);
    SDL_FreeSurface(room);

    SDL_FreeSurface(door);
    SDL_FreeSurface(doorN);
    SDL_FreeSurface(doorS);
    SDL_FreeSurface(doorE);
    SDL_FreeSurface(doorW);

    SDL_FreeSurface(slash);

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

    SDL_Rect dest_hero;

    SDL_Rect dest_wall;
    SDL_Rect dest_room;
    SDL_Rect dest_door;

    SDL_Rect dest_doorN;
    SDL_Rect dest_doorS;

    SDL_Rect dest_doorE;
    SDL_Rect dest_doorW;

    SDL_Rect dest_slash;

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

    SDL_QueryTexture(tex_hero, NULL, NULL, &dest_hero.w, &dest_hero.h);
    SDL_QueryTexture(tex_heroDMG, NULL, NULL, &dest_hero.w, &dest_hero.h);

    SDL_QueryTexture(tex_wall, NULL, NULL, &dest_wall.w, &dest_wall.h);

    SDL_QueryTexture(tex_room, NULL, NULL, &dest_room.w, &dest_room.h);

    SDL_QueryTexture(tex_doorN, NULL, NULL, &dest_door.w, &dest_door.h);
    SDL_QueryTexture(tex_doorS, NULL, NULL, &dest_door.w, &dest_door.h);
    SDL_QueryTexture(tex_doorE, NULL, NULL, &dest_door.w, &dest_door.h);
    SDL_QueryTexture(tex_doorW, NULL, NULL, &dest_door.w, &dest_door.h);

    SDL_QueryTexture(tex_door, NULL, NULL, &dest_door.w, &dest_door.h);

    SDL_QueryTexture(tex_slash, NULL, NULL, &dest_slash.w, &dest_slash.h);



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

    dest_hero.w = CELL*1.5;
    dest_hero.h = CELL*1.5;

    dest_wall.w = CELL;
    dest_wall.h = CELL;

    dest_room.w = SCREEN_W;
    dest_room.h = SCREEN_H;

    dest_door.w = CELL;
    dest_door.h = CELL;



    //------------------------  DOORS  ----------------------------

    // dest_doorN.w = CELL;
    // dest_doorN.h = CELL;

    // dest_doorS.w = CELL;
    // dest_doorS.h = CELL;

    // dest_doorE.w = CELL;
    // dest_doorE.h = CELL;

    // dest_doorW.w = CELL;
    // dest_doorW.h = CELL;
    
    //---------------------------------------------------------------

    dest_start.w = CELL;
    dest_start.h = CELL;

    dest_goal.w = CELL;
    dest_goal.h = CELL;

    dest_you_win.w /= 5;
    dest_you_win.h /= 5;

    dest_monster.w = CELL*1.3;
    dest_monster.h = CELL*1.3;

    dest_slash.w = CELL;
    dest_slash.h = CELL;



    dest_open_chest.w = CELL;
    dest_open_chest.h = CELL;
    dest_closed_chest.w = CELL;
    dest_closed_chest.h = CELL;

    dest_trap.w = CELL*1.5;
    dest_trap.h = CELL*1.5;

    int indicators_offset = CELL/4;
    int icons_size = CELL/2;
    int icons_y = CELL/4;

    dest_hero_life_icon.x = icons_size;
    dest_hero_life_icon.y = icons_y;
    dest_hero_life_icon.w = icons_size;
    dest_hero_life_icon.h = icons_size;

    dest_hero_life_text.x = dest_hero_life_icon.x + dest_hero_life_icon.w + indicators_offset;
    dest_hero_life_text.y = icons_y;
    dest_hero_life_text.w = icons_size;
    dest_hero_life_text.h = icons_size;

    dest_hero_attack_icon.x = dest_hero_life_text.x + dest_hero_life_text.w + indicators_offset;
    dest_hero_attack_icon.y = icons_y;
    dest_hero_attack_icon.w = icons_size;
    dest_hero_attack_icon.h = icons_size;

    dest_hero_attack_text.x = dest_hero_attack_icon.x + dest_hero_attack_icon.w + indicators_offset;
    dest_hero_attack_text.y = icons_y;
    dest_hero_attack_text.w = icons_size;
    dest_hero_attack_text.h = icons_size;

    

    current_room_id = player->location;

    dest_hero.x = SCREEN_W / 2 - dest_hero.w/2;

    dest_hero.y = SCREEN_H / 2 - dest_hero.h/2;

    dest_game_over.x = SCREEN_W / 2 - dest_game_over.w/2;
    dest_game_over.y = SCREEN_H / 2 - dest_game_over.h/2;

    dest_you_win.x = SCREEN_W / 2 - dest_you_win.w/2;
    dest_you_win.y = SCREEN_H / 2 - dest_you_win.h/2;

    SDL_Point mousePosition;

    while (!closeWindow)
    {
        SDL_Event event;

        current_room_player = getRoomPointerByID(current_room_id);
        if (current_room_player->type == Goal)
        {
            current_game_state = GAME_STATE_VICTORY;
        }
        if (player->hp <= 0)
        {
            current_game_state = GAME_STATE_OVER;
        }

        SDL_RenderClear(rend);
        if(current_game_state==GAME_STATE_RUN){
            SDL_RenderCopy(rend, tex_room, NULL, &dest_room);
        }


        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {

            case SDL_QUIT:

                closeWindow = 1;
                break;

            case SDL_KEYDOWN:

                if (current_game_state == GAME_STATE_RUN)
                {

                    switch (event.key.keysym.scancode)
                    {
                    case SDL_SCANCODE_W:
                    case SDL_SCANCODE_UP:
                        if (isDoorOpen(current_room_id, 0))
                        {
                            safelyMoveHero(0);
                        }
                        else
                        {
                            printf("North door is closed\n");
                        }
                        break;
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:

                        if (isDoorOpen(current_room_id, 3))
                        {
                            safelyMoveHero(3);
                        }
                        else
                        {
                            printf("West door is closed\n");
                        }
                        
                        break;
                    case SDL_SCANCODE_S:
                    case SDL_SCANCODE_DOWN:
                        if (isDoorOpen(current_room_id, 1))
                        {
                            safelyMoveHero(1);
                        }
                        else
                        {
                            printf("South door is closed\n");
                        }
                        break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT:
                        if (isDoorOpen(current_room_id, 2))
                        {
                            safelyMoveHero(2);
                        }
                        else
                        {
                            printf("East door is closed\n");
                        }
                        
                        break;
                    case SDL_SCANCODE_E:
                        openChest(player);
                        break;
                    case SDL_SCANCODE_SPACE:
                        printf("space pressed \n");
                        printf("Hero is attacking, atk: %d\n", player->atk);

                        struct room *current_room = getRoomPointerByID(current_room_id);
                        attackingLock = current_room->room_lock;

                        dest_slash.x = dest_hero.x + CELL;
                        dest_slash.y = dest_hero.y;

                        SDL_RenderCopy(rend, tex_slash, NULL, &dest_slash);

                        //pthread_mutex_t roomMutex = current_room->room_lock;
                        pthread_mutex_lock(&attackingLock);
                        int monsterID = current_room->monsterInRoomID;

                        if (monsterID != -1)
                        {
                            struct monster *monster = &monsters[monsterID];
                            if (monster->state == IDLE)
                            {
                                attackMonster(player, monster);
                                sleep(0.1);

                            }
                            else
                            {
                                printf("Monster is not idle\n");
                            }
                        }
                        else
                        {
                            printf("There is no monster in this room\n");
                        }
                        pthread_mutex_unlock(&attackingLock);
                        //SDL_DestroyTexture(tex_slash);

                        break;
                    default:
                        break;
                    }
                }
            default:
                break;
            }
        }



        //printf("Rend\n");
        
        switch (current_game_state)
        {
        case GAME_STATE_OVER:
            SDL_RenderCopy(rend, tex_game_over, NULL, &dest_game_over);
            break;
        case GAME_STATE_VICTORY:
            SDL_RenderCopy(rend, tex_win, NULL, &dest_you_win);
            break;
        case GAME_STATE_RUN:
            

            

            for (int k = 0; k < 4; k++)
            {
                if (current_room_player->doors[k].state == 1)
                {
                    switch (current_room_player->doors[k].cardinal)
                    {

                    case 0:
                        dest_door.x = SCREEN_W / 2 - dest_door.w / 2;
                        dest_door.y = 0;
                        SDL_RenderCopy(rend, tex_doorN, NULL, &dest_door);
                        break;
                    case 1:
                        dest_door.x = SCREEN_W / 2 - dest_door.w / 2;
                        dest_door.y = SCREEN_H - dest_door.h;
                        SDL_RenderCopy(rend, tex_doorS, NULL, &dest_door);
                        break;
                    case 2:
                        dest_door.x = SCREEN_W - dest_door.w;
                        dest_door.y = SCREEN_H / 2 - dest_door.h / 2;
                        SDL_RenderCopy(rend, tex_doorE, NULL, &dest_door);
                        break;
                    case 3:
                        dest_door.x = 0;
                        dest_door.y = SCREEN_H / 2 - dest_door.h / 2;
                        SDL_RenderCopy(rend, tex_doorW, NULL, &dest_door);
                        break;
                    default:
                        break;
                    }
                    
                }
            }
            if (current_room_player->occupiedByMonster)
            {
                dest_monster.x = dest_hero.x + CELL*2;
                dest_monster.y = dest_hero.y;

                SDL_RenderCopy(rend, tex_monster, NULL, &dest_monster);
            }

            if (current_room_player->type == Treasure && current_room_player->treasure)
            {
                dest_closed_chest.x = (SCREEN_W / 2 - CELL) + SCREEN_W / 4;
                dest_closed_chest.y = (SCREEN_H / 2 - CELL) + SCREEN_H / 4;
                SDL_RenderCopy(rend, tex_closed_chest, NULL, &dest_closed_chest);
            }
            if (current_room_player->type == Treasure && !current_room_player->treasure)
            {
                dest_open_chest.x = (SCREEN_W / 2 - CELL) + SCREEN_W / 4;
                dest_open_chest.y = (SCREEN_H / 2 - CELL) + SCREEN_H / 4;
                SDL_RenderCopy(rend, tex_open_chest, NULL, &dest_open_chest);
            }
            if (current_room_player->type == Trap && current_room_player->trap == 1)
            {
                dest_trap.x = SCREEN_W / 2 - SCREEN_W / 4;
                dest_trap.y = SCREEN_H / 2 - dest_trap.h / 2;
                SDL_RenderCopy(rend, tex_trap, NULL, &dest_trap);
            }
            if(player->hp < prevHealth){
                SDL_RenderCopy(rend, tex_heroDMG, NULL, &dest_hero);
                sleep(0.1);
                prevHealth = player->hp;
            }else{
                SDL_RenderCopy(rend, tex_hero, NULL, &dest_hero);
                prevHealth = player->hp;
            }

            sprintf(hero_life_text_string, "%d", player->hp);
            hero_life_text = TTF_RenderText_Solid(sans, hero_life_text_string, font_color);
            SDL_Texture *tex_hero_life_text = SDL_CreateTextureFromSurface(rend, hero_life_text);

            SDL_RenderCopy(rend, tex_hero_life_text, NULL, &dest_hero_life_text);
            SDL_RenderCopy(rend, tex_hero_life_icon, NULL, &dest_hero_life_icon);
            SDL_DestroyTexture(tex_hero_life_text);
            SDL_FreeSurface(hero_life_text);

            sprintf(hero_attack_text_string, "%d", player->atk);
            hero_attack_text = TTF_RenderText_Solid(sans, hero_attack_text_string, font_color);
            SDL_Texture *tex_hero_attack_text = SDL_CreateTextureFromSurface(rend, hero_attack_text);

            SDL_RenderCopy(rend, tex_hero_attack_text, NULL, &dest_hero_attack_text);
            SDL_RenderCopy(rend, tex_hero_attack_icon, NULL, &dest_hero_attack_icon);
            SDL_DestroyTexture(tex_hero_attack_text);
            SDL_FreeSurface(hero_attack_text);
            break;
        default:
            break;
        }

        SDL_RenderPresent(rend);

        SDL_Delay(100);
    }

    SDL_DestroyTexture(tex_hero);
    SDL_DestroyTexture(tex_heroDMG);
    SDL_DestroyTexture(tex_wall);
    SDL_DestroyTexture(tex_room);
    SDL_DestroyTexture(tex_door);

    SDL_DestroyTexture(tex_doorN);
    SDL_DestroyTexture(tex_doorS);
    SDL_DestroyTexture(tex_doorE);
    SDL_DestroyTexture(tex_doorW);


    SDL_DestroyTexture(tex_start);
    SDL_DestroyTexture(tex_goal);
    SDL_DestroyTexture(tex_game_over);
    SDL_DestroyTexture(tex_win);
    SDL_DestroyTexture(tex_monster);
    SDL_DestroyTexture(tex_slash);
    SDL_DestroyTexture(tex_open_chest);
    SDL_DestroyTexture(tex_closed_chest);
    SDL_DestroyTexture(tex_trap);

    SDL_DestroyRenderer(rend);



    for (int i = 0; i < monsterCount; i++)
    {
        pthread_join(monsterThreads[i], NULL);
    }

    printf("All monsters are dead\n");

    printf("HeroHP is dead\n");
    pthread_cancel(heroThread);

    pthread_join(heroThread, NULL);

    printf("HeroThread is dead\n");

    pthread_mutex_destroy(&lock);

    SDL_DestroyWindow(win);

    SDL_Quit();

    return 0;
}