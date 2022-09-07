#include <time.h>
#include <stdlib.h>
#include <stdio.h>

enum CardinalPoint {North, South, East, West}; //North = 0, South = 1, East = 2, West = 3
enum DoorState {Closed, Open}; //Open = 0, Closed = 1
enum RoomType {Normal, Treasure, Trap, Start, Goal, Nothing}; //Normal = 0, Treasure = 1, Trap = 2, Start = 3, Goal = 4

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
    
    int treasure; //0=No treasure, 1=Has treasure
    int monster; //0=No monster, 1=Has monster
};

//---Variables gloables---

short N;
struct room **game_map;

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

int main(){
    srand(time(NULL));
    //Crear matrices
    printf("Ingrese N: ");
    scanf("%d", &N);
    printf("\n");
    
    int total = N*N;
    game_map = (int **)malloc(N * sizeof(struct room *));

    int startRoomID = (rand() % total) + 1;
    int goalRoomID = (rand() % total) + 1;

    while(startRoomID == goalRoomID){
        goalRoomID = (rand() % total) + 1;
    }

    enum RoomType tipos [3] = {Normal, Treasure, Trap};

    for (int i = 0; i < N; i++)
    {
        game_map[i] = (int *)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++)
        {
            struct room *room = (struct room *) malloc(sizeof(struct room));
            
            room->id = (j+1)+N*i;

            int probability = rand() % 10;
            if(probability < 3){//30% de chance de ser una habitacion normal
                room->type = Normal;
            }else if (probability > 6){//30% de chance de ser una habitacion con tesoro
                room->type = Treasure;
            }else{//40% de chance de ser una habitacion con trampa
                room->type = Trap;
            }
            room->type = room->id==startRoomID? Start : room->id==goalRoomID? Goal : room->type;
            createDoors(room->doors);
            room->openDoorsLeft = 3;
            room->discovered = 0;
            room->occupied = room->type==Start? 1 : 0;
            room->treasure = 0;
            room->monster = 0;

            //gameRooms[(j)+N*i] = *room;
            game_map[i][j] = *room;
        }
        
    }

    /*
    for(int i = 0; i < N*N; i++){
        getRoomDetails(&gameRooms[i]);
    }
    */

    getMapDetails();
    //getRoomDetailsByID(1);

    /*
    for(int i = 0; i < N; i++){
        free(game_map[i]);
    }
    */
    
    return 0;
}