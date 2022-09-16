#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "mapGenerator.h"
#include "global.h"
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t lock;
sem_t semaph;

int monster0Movements[500];
int monster0MovementsIndex = 0;

int monster1Movements[500];
int monster1MovementsIndex = 0;

int heroMovements[500];
int heroMovementsIndex = 0;

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

void * monsterLife(void * m){
    struct monster * mon = (struct monster *) m;
    int freeNSize = 0;
    int freeNeighbours[4];
    //printf("\nMonster %d is alive\n", mon->id);
    //while(mon->hp != 0){
    int r; 

    while(mon->hp != 0){
        //printf("\nLooking for free neighbours of %d in %d\n", mon->id, mon->location);
        r = rand() % 100;
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

        //* .Todo esto es temporal
            
            
            //pthread_mutex_lock(&lock);
            //getMapDetails();
            //pthread_mutex_unlock(&lock);

            //sleep(25);

            //sem_post(&semaph);
            sleep((rand()%2)+1);
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

int openChest(struct hero *h){
    if (getRoomType(h->location) == 1 && getRoomChestState(h->location) == 1) {  
        int r = rand() % 2;
        if (r == 0) {
            h->atk = h->atk + 1;
            printf("Hero opened a chest and got +1 attack, actual attack: %d\n", h->atk);
        } else {
            h->hp = h->hp + 1;
            printf("Hero has %d hp\n", h->hp);
        }
        setRoomChestState(h->location, 0);
        return 1;
    } else {
        return 0;
    }
}

void* heroLife(void* h)
{
    struct hero * hero = (struct hero *) h;
    int freeNSize = 0;
    int freeNeighbours[4];
    //printf("\nHero is alive\n");
    //while(her->hp != 0){

    /* IMPORTANTE FUNCIONES
    getNeighbour(int roomID, int direction)
    getRoomPointerByID(int roomID)
    isDoorOpen(int roomID, int direction)
    */

   printf("Holaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");

    while(hero->hp != 0){

        char input = getchar();
        while ('\n' != getchar());

        if(input == 'w'){
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
        }else if(input == ' '){
            printf("Hero is attacking\n");
        }else if(input == 'e'){
            openChest(hero);
        }

        pthread_mutex_lock(&lock);
        drawTemporalMap();
        pthread_mutex_unlock(&lock);

        printf("Hero has %d hp\n", hero->hp);

        printf("Monster 0 path:\n");
        printArray(monster0Movements, monster0MovementsIndex);   
        printf("Monster 1 path:\n");
        printArray(monster1Movements, monster1MovementsIndex);
        printf("Hero path:\n");
        printArray(heroMovements, heroMovementsIndex);

        sleep(0.1);


        // // ---------------------------------------------------------
    }
    
    printf("Hero is dead\n");

    pthread_mutex_lock(&lock); //Aqui se libera la habitación en la que el monstruo murio
    setOccupied(hero->location, 0);
    setMonsterID(hero->location, -1);
    pthread_mutex_unlock(&lock);

    sleep(3);
    pthread_exit(0);
}

void* observerHpHero(void* h) {
    struct hero * hero = (struct hero *) h;
    while(hero->hp != 0){
        sleep(1);
        // printf("Hero has %d hp\n", hero->hp);
    }
    printf("Hero is dead\n");
    pthread_exit(0);
}

int main(){

    srand(time(NULL));

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

    int start=generateMap(N);
    
    while(start==-1){
        start=generateMap(N);
        printf("Reintentando generar mapa...\n");
    }

    pthread_mutex_init(&lock, NULL);

    int monsterCount = 2;

    struct hero * player = (struct hero *) malloc(sizeof(struct hero));
    player->hp = 5;
    player->atk = 1;
    player->location = start;
    setHeroInRoom(start, 1);

    getMapDetails();
    return 0;

    pthread_t * heroThread = (pthread_t *) malloc(sizeof(pthread_t));
    pthread_t * heroHp = (pthread_t *) malloc(sizeof(pthread_t));

    //getRoomDetailsByID(start);
    //sem_init(&semaph,0,monsterCount);

    pthread_t * monsterThreads;
    monsterThreads = (pthread_t *)malloc(sizeof(pthread_t)*monsterCount);

    struct monster monsters[monsterCount];

    //printf("Looking for free rooms...\n");
    //getMapDetails();

    for(int m; m < monsterCount; m++){
        //printf("Monster %d is alive OUT\n", m);
        monsters[m].id = m;
        monsters[m].hp = 3;
        monsters[m].atk = 1;
        monsters[m].state = IDLE;
        monsters[m].location = getFreeRooms();
        printf("Room %d is occupied by monster %d\n", monsters[m].location, monsters[m].id);
        //getRoomDetailsByID(monsters[m].location);
        setOccupied(monsters[m].location, 1);
        setMonsterID(monsters[m].location, monsters[m].id);

        pthread_create(&monsterThreads[m], NULL, monsterLife, &monsters[m]);
        
    }

    pthread_create(&heroThread, NULL, heroLife, player);
    pthread_create(&heroHp, NULL, observerHpHero, player);

    for (int i = 0; i < monsterCount; i++)
    {
        pthread_join(monsterThreads[i], NULL);
    }

    pthread_join(heroHp, NULL);

    printf("Game Over\n");

    pthread_mutex_destroy(&lock);

    //sem_destroy(&semaph);
    
    getMapDetails();

    return 0;
}