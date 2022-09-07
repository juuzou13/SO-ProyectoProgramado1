#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define true 1
#define false 0

enum CardinalPoint {North, South, East, West}; //North = 0, South = 1, East = 2, West = 3
enum DoorState {Closed, Open}; //Open = 1, Closed = 0
enum RoomType {Normal, Treasure, Trap, Start, Goal, Nothing}; //Normal = 0, Treasure = 1, Trap = 2, Start = 3, Goal = 4, Nothing = 5

struct door
{
    enum CardinalPoint cardinal;
    enum DoorState state;
};

struct room
{
    int id;
    int type; //0=Normal, 1=Start, 2=Goal
    struct door doors[4];
    int openDoorsLeft; //Number of open doors

    int discovered;
    int occupied;
    
    int treasure;
    int monster;
};

//---Variables gloables---

short N;
struct room **game_map;
struct room *game_rooms;

//------------------------

/*
    Retorna el id de la habitacion en la direccion cardinal ingresada, de la habitacion ingresada en currID
    Retorna -1 si no hay habitacion en esa direccion
*/

int getNeighbour(int currID, enum CardinalPoint direction){
    switch(direction){
        case North:
            return currID - N > 0? currID - N : -1;
        case South:
            return currID + N <= N*N? currID + N : -1;
        case East:
            return currID % N != 0? currID + 1 : -1;
        case West:
            return currID % N != 1? currID - 1 : -1;
    }
}

//Crea 4 posibles puertas para cada habitacion, todas cerradas
void createDoors(struct door doors[4]){
    for(int i = 0; i < 4; i++){
        doors[i].cardinal = i;
        doors[i].state = Closed;
    }
}

//Imprime los datos de una habitacion
void getRoomDetails(struct room *room){
    printf("\n");
    printf("Room id: %d\n", room->id);
    printf("Room north neighbour: %d\n", getNeighbour(room->id, North));
    printf("Room south neighbour: %d\n", getNeighbour(room->id, South));
    printf("Room east neighbour: %d\n", getNeighbour(room->id, East));
    printf("Room west neighbour: %d\n", getNeighbour(room->id, West));
    
    char tipo[10];
    
    switch(room->type){
        case Normal:
            strcpy(tipo, "Normal");
            break;
        case Treasure:
            strcpy(tipo, "Treasure");
            break;
        case Trap:
            strcpy(tipo, "Trap");
            break;
        case Start:
            strcpy(tipo, "Start");
            break;
        case Goal:
            strcpy(tipo, "Goal");
            break;
        case Nothing:
            strcpy(tipo, "Nothing");
            break;
    }

    printf("Room type: %s\n", tipo);
    printf("Room doors: \n");

    for(int k = 0; k < 4; k++){
        printf("  Door %d) Cardinal: %d, State: %d\n", k+1, room->doors[k].cardinal, room->doors[k].state);
    }

    printf("Room open doors left: %d\n", room->openDoorsLeft);
    printf("Room discovered: %d\n", room->discovered);
    printf("Room occupied: %d\n", room->occupied);
    printf("Room treasure: %d\n", room->treasure);
    printf("Room monster: %d\n", room->monster);
    printf("\n");
}

//Imprime los datos de todas las habitaciones
void getMapDetails(){
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("Matriz en posicion [%d][%d]\n", i, j);
            getRoomDetails(&game_map[i][j]);
        }
    }
}

//Imprime los datos de una habitacion especifica
void getRoomDetailsByID(int id){
    if(id > N*N || id < 1){
        printf("No existe habitacion con dicho id.\n");    
    }else{
        int i = (id-1) / N;
        int j = (id-1) % N;
        getRoomDetails(&game_map[i][j]);
    }
}


void printArray(int *array, int size){
    for(int i = 0; i < size; i++){
        printf("%d ", array[i]);
    }
    printf("\n");
}

int isGoal(int id){
    int i = (id-1) / N;
    int j = (id-1) % N;
    return game_map[i][j].type == Goal;
}

//Check if element is in array
int isInArray(int *array, int size, int element){
    for(int i = 0; i < size; i++){
        if(array[i] == element){
            return true;
        }
    }
    return false;
}

int main(){

    srand(time(NULL));

    printf("\n");
    printf("Ingrese N: ");
    scanf("%d", &N);
    printf("\n");

    if(N < 2){
        printf("N debe ser mayor o igual a 2.\n");
        return 0;
    }
    
    int total = N*N;

    game_map = (int **)malloc(N * sizeof(struct room *));
    game_rooms = (int *)malloc(N * sizeof(int));

    int startRoomID = (rand() % total) + 1;
    int goalRoomID = (rand() % total) + 1;

    while(startRoomID == goalRoomID){
        goalRoomID = (rand() % total) + 1;
    }

    int idList[total];

    int id;

    for (int i = 0; i < N; i++)
    {
        game_map[i] = (int *)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++)
        {
            struct room *room = (struct room *) malloc(sizeof(struct room));
            
            //Calcula un id desde 1,2,3..N*N, con la i y la j
            id = (j+1)+N*i;

            room->id = id;
            idList[id-1] = id;

            int probability = rand() % 10;

            if(probability < 3){// 30% de probabilidad de ser una habitacion j)+N*inormal
                room->type = Normal;
            }else if (probability > 6){// 30% de probabilidad de ser una habitacion con tesoro
                room->type = Treasure;
            }else{// 40% de probabilidad de ser una habitacion con trampa
                room->type = Trap;
            }

            room->type = room->id==startRoomID? Start : room->id==goalRoomID? Goal : room->type;
            createDoors(room->doors);
            room->openDoorsLeft = 3;
            room->discovered = false;
            room->occupied = room->type==Start? true : false;
            room->treasure = false;
            room->monster = false;

            game_rooms[id-1] = *room;
            game_map[i][j] = *room;
        }
    }

    //getMapDetails();
    //printArray(idList, total);

    int blockedIds[total];
    int blockedIdsSize = 0;

    int touredIds[total];
    int touredIdsSize = 0;

    int possibleDestinations[4];
    int possibleDestinationsSize = 0;

    touredIds[touredIdsSize++] = startRoomID;

    int currentNeighbour;

    int currentRoom;

    int chosenNextRoom;

    printf("\nCuarto incial: %d\n", startRoomID);
    printf("Cuarto final: %d\n", goalRoomID);
    
    while(touredIds[touredIdsSize-1]!=goalRoomID){
        currentRoom = touredIds[touredIdsSize-1];

        //printf("\n\nCurrent room: %d\n", currentRoom);

        for(int i = 0; i < 4; i++)
        {
            currentNeighbour = getNeighbour(currentRoom, i);

            if(!isInArray(touredIds, touredIdsSize, currentNeighbour) && !isInArray(blockedIds, blockedIdsSize, currentNeighbour)){
                if(currentNeighbour != -1){
                    if(currentRoom==startRoomID){
                        if(currentNeighbour!=goalRoomID){
                            possibleDestinations[possibleDestinationsSize++] = currentNeighbour;
                        }
                    }else{
                        possibleDestinations[possibleDestinationsSize++] = currentNeighbour;
                    }
                }
            } 
        }

        //printArray(possibleDestinations, possibleDestinationsSize);
        //printf("Possible destinations size: %d\n", possibleDestinationsSize);

        if(possibleDestinationsSize == 0){
            //printf("Blocked room: %d\n", currentRoom);
            blockedIds[blockedIdsSize++] = currentRoom;
            touredIdsSize--;
            possibleDestinationsSize = 0;
            //printArray(touredIds, touredIdsSize);
        }else{
            chosenNextRoom = possibleDestinations[rand() % possibleDestinationsSize];
            //printf("Cuarto elegido: %d\n", chosenNextRoom);
            touredIds[touredIdsSize++] = chosenNextRoom;
            possibleDestinationsSize = 0;
        }
        


    }

    printf("Camino seguro: ");
    printArray(touredIds, touredIdsSize);

    

    


    
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