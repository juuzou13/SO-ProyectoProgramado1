#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "mapGenerator.h"
#include "monster.h"
#include <pthread.h>
#include <semaphore.h>

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
                pthread_mutex_lock(&lock); //Aqui el hilo monstruo obtiene el estado de una habitacion de manera sincronizada y revisa si esta ocupada o no
                tempNeighbour = getRoomPointerByID(neighbourID); 
                if(tempNeighbour->occupiedByMonster == 0 && tempNeighbour->type != Start && tempNeighbour->type != Goal && tempNeighbour->type != Wall){
                    canProceedToThisRoom = 1;
                    setOccupied(id, 0);
                    setOccupied(neighbourID, 1);
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
            // setOccupied(mon->location, 0);
            monsterMove(mon, loc);
            // setOccupied(loc, 1);
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
            pthread_mutex_lock(&lock);
            system("clear");
            drawTemporalMap();
            pthread_mutex_unlock(&lock);

            printf("Monster 0 path:\n");
            printArray(monster0Movements, monster0MovementsIndex);   
            printf("Monster 1 path:\n");
            printArray(monster1Movements, monster1MovementsIndex);
            //sem_post(&semaph);
            sleep((rand()%2)+1);
        // ---------------------------------------------------------
    }
    
    printf("Monster %d is dead\n", mon->id);

    pthread_mutex_lock(&lock); //Aqui se libera la habitación en la que el monstruo murio
    setOccupied(mon->location, 0);
    pthread_mutex_unlock(&lock);

    sleep(3);
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
    
    while(generateMap(N)==-1){
        printf("Reintentando generar mapa...\n");
    }

    pthread_mutex_init(&lock, NULL);

    int monsterCount = 2;

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

        pthread_create(&monsterThreads[m], NULL, monsterLife, &monsters[m]);
        
    }

    for(int i = 0; i < monsterCount; i++){
        pthread_join(monsterThreads[i], NULL);
    }

    printf("Game Over\n");

    pthread_mutex_destroy(&lock);

    //sem_destroy(&semaph);
    
    return 0;
}