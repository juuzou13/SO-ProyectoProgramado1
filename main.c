#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "mapGenerator.h"
#include "monster.h"
#include <pthread.h>
#include <semaphore.h>


pthread_mutex_t lock;
sem_t semaph;
int **map;

void allocate(){
    map = (int **)malloc(sizeof(int *) * N);
    for(int i = 0; i < N; i++){
        map[i] = (int *)malloc(sizeof(int) * N);
    }
}


void * monsterLife(void * m){
    struct monster * mon = (struct monster *) m;
    int freeNSize = 0;
    int freeNeighbours[4];
    //printf("\nMonster %d is alive\n", mon->id);
    //while(mon->hp != 0){

    int r;

    while(1){
        //printf("\nLooking for free neighbours of %d in %d\n", mon->id, mon->location);
        r = rand() % 100;
        if(r < 8){
            
            //pthread_mutex_lock(&lock);
            sem_wait(&semaph);

            printf("\n\n-------------------------------------------------------------------------\n\n");
            printf("Looking for free neighbours of %d in %d...\n\n", mon->id, mon->location);

            int loc = getFreeNeighboursNoStartGoal(mon->location);

            if(loc!=-1){
                setOccupied(mon->location, 0);
                printf("Found (%d), a free neighbour of monster %d, located in %d\n", loc, mon->id, mon->location);
                monsterMove(mon, loc);
                setOccupied(loc, 1);
            }else{
                printf("No free neighbours found for monster %d, located in %d\n", mon->id, mon->location);
            }

            //
            sleep(1);
            printf("\n\n-------------------------------------------------------------------------\n\n");

            drawTemporalMap();
            sem_post(&semaph);            
            //pthread_mutex_unlock(&lock);
        }
        
    }

    pthread_exit(NULL);

        
    //}
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
    
    while(generateMap(N)==-1){
        printf("Reintentando generar mapa...\n");
    }

    //allocate();
    /*

    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            struct room * r = &game_map[i][j];
            getRoomDetailsByID(r->id);
        }
        printf("\n");
    }
    sleep(3);

    */


    //getMapDetails();

    //pthread_mutex_init(&lock, NULL);

    int monsterCount = N/2;

    sem_init(&semaph,0,N/2);


    pthread_t * monsterThreads;
    monsterThreads = (pthread_t *)malloc(sizeof(pthread_t)*monsterCount);

    struct monster monsters[monsterCount];

    int freeRooms[N];
    int freeRoomsSize = 0;

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

    //pthread_mutex_destroy(&lock);

    sem_destroy(&semaph);
    
    return 0;
}

/*
for(int i = 0; i < N*N; i++){
    getRoomDetails(&gameRooms[i]);
}
*/
/*
for(int i = 0; i < N; i++){
    free(game_map[i]);
}
*/
