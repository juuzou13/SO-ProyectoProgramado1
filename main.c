#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define true 1
#define false 0

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define DEFAULT "\x1b[0m"

#define EASY 10
#define MEDIUM 20
#define HARD 30

enum CardinalPoint
{
    North,
    South,
    East,
    West
}; // North = 0, South = 1, East = 2, West = 3
enum DoorState
{
    Closed,
    Open
}; // Open = 1, Closed = 0
enum RoomType
{
    Normal,
    Treasure,
    Trap,
    Start,
    Goal,
    Wall
}; // Normal = 0, Treasure = 1, Trap = 2, Start = 3, Goal = 4, Wall = 5

struct door
{
    enum CardinalPoint cardinal;
    enum DoorState state;
};

struct room
{
    int id;
    int type; // 0=Normal, 1=Start, 2=Goal
    struct door doors[4];
    int openDoorsLeft; // Number of open doors

    int discovered;
    int occupied;

    int treasure;
    int trap;
    int monster;
};

//---Variables gloables---

short N;
struct room **game_map;

//------------------------

/*
    Retorna el id de la habitacion en la direccion cardinal ingresada, de la habitacion ingresada en currID
    Retorna -1 si no hay habitacion en esa direccion
*/

int getNeighbour(int currID, enum CardinalPoint direction)
{
    switch (direction)
    {
    case North:
        return currID - N > 0 ? currID - N : -1;
    case South:
        return currID + N <= N * N ? currID + N : -1;
    case East:
        return currID % N != 0 ? currID + 1 : -1;
    case West:
        return currID % N != 1 ? currID - 1 : -1;
    }
}

// Crea 4 posibles puertas para cada habitacion, todas cerradas
void createDoors(struct door doors[4])
{
    for (int i = 0; i < 4; i++)
    {
        doors[i].cardinal = i;
        doors[i].state = Closed;
    }
}

char *getCardinalName(int cardinal)
{
    switch (cardinal)
    {
    case 0:
        return "Norte";
        break;
    case 1:
        return "Sur";
        break;
    case 2:
        return "Este";
        break;
    case 3:
        return "Oeste";
        break;
    }
}

// Imprime los datos de una habitacion
void getRoomDetails(struct room *room)
{
    printf("\n");
    printf("Room id: %d\n", room->id);
    printf("Room north neighbour: %d\n", getNeighbour(room->id, North));
    printf("Room south neighbour: %d\n", getNeighbour(room->id, South));
    printf("Room east neighbour: %d\n", getNeighbour(room->id, East));
    printf("Room west neighbour: %d\n", getNeighbour(room->id, West));

    char tipo[10];

    switch (room->type)
    {
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
    case Wall:
        strcpy(tipo, "Wall");
        break;
    }

    printf("Room type: %s\n", tipo);
    printf("Room doors: \n");

    for (int k = 0; k < 4; k++)
    {
        printf("  Door %d) Cardinal: %s, State: %d\n", k + 1, getCardinalName(room->doors[k].cardinal), room->doors[k].state);
    }

    printf("Room open doors left: %d\n", room->openDoorsLeft);
    printf("Room discovered: %d\n", room->discovered);
    printf("Room occupied: %d\n", room->occupied);
    printf("Room treasure: %d\n", room->treasure);
    printf("Room monster: %d\n", room->monster);
    printf("\n");
}

// Imprime los datos de todas las habitaciones
void getMapDetails()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("Matriz en posicion [%d][%d]\n", i, j);
            getRoomDetails(&game_map[i][j]);
        }
    }
}

// Imprime los datos de una habitacion especifica
void getRoomDetailsByID(int id)
{
    if (id > N * N || id < 1)
    {
        printf("No existe habitacion con dicho id.\n");
    }
    else
    {
        int i = (id - 1) / N;
        int j = (id - 1) % N;
        getRoomDetails(&game_map[i][j]);
    }
}

struct room *getRoomPointerByID(int id)
{
    if (id > N * N || id < 1)
    {
        printf("No existe habitacion con dicho id.\n");
    }
    else
    {
        int i = (id - 1) / N;
        int j = (id - 1) % N;
        return &game_map[i][j];
    }
}

void printArray(int *array, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

void printArrayFrom(int *array, int index, int size)
{
    for (int i = index; i < size; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

int isGoal(int id)
{
    int i = (id - 1) / N;
    int j = (id - 1) % N;
    return game_map[i][j].type == Goal;
}

int isInArray(int *array, int size, int element)
{
    for (int i = 0; i < size; i++)
    {
        if (array[i] == element)
        {
            return true;
        }
    }
    return false;
}

// Dibuja el mapa con ASCII mostrando paredes y puertas abiertas
void drawTemporalMap()
{
    printf("\n");
    char openDoors[5] = {' ', ' ', ' ', ' ', '\0'};
    struct room *room;

    char *roomColor;

    for (int i = 0; i < N; i++)
    {

        for (int j = 0; j < N; j++)
        {

            room = &game_map[i][j];

            for (int k = 0; k < 4; k++)
            {
                if (room->type == Wall)
                {
                    for (int k = 0; k < 4; k++)
                    {
                        openDoors[k] = '//';
                    }
                }
                else
                {
                    if (room->doors[k].state == Open)
                    {
                        switch (room->doors[k].cardinal)
                        {
                        case North:
                            openDoors[0] = '^';
                            break;
                        case South:
                            openDoors[1] = 'v';
                            break;
                        case East:
                            openDoors[2] = '>';
                            break;
                        case West:
                            openDoors[3] = '<';
                            break;
                        }
                    }
                    else
                    {
                        openDoors[k] = ' ';
                    }
                }
            }
            char *spaces = "";
            char *spaces2 = "  ";

            int q = room->id;
            if (q < 10)
            {
                spaces2 = "  ";
            }
            else if (q < 100)
            {
                spaces2 = " ";
            }
            else if (q < 1000)
            {
                spaces2 = "";
            }

            if (room->type == Start)
            {
                roomColor = GREEN;
                printf("%s|" GREEN "%d" DEFAULT ") %s%s", spaces, room->id, openDoors, spaces2);
            }
            else if (room->type == Goal)
            {
                roomColor = RED;
                printf("%s|" RED "%d" DEFAULT ") %s%s", spaces, room->id, openDoors, spaces2);
            }
            else
            {
                printf("%s|" DEFAULT "%d" DEFAULT ") %s%s", spaces, room->id, openDoors, spaces2);
            }

            for (int k = 0; k < 4; k++)
            {
                openDoors[k] = ' ';
            }

            if (room->id % N == 0)
            {
                printf("%s|", spaces);
            }
        }
        printf("\n");
    }
    printf("\n");
}

int getMaxNeighbours(int id)
{

    if (id == 1 || id == N || id == N * N || id == N * N - N + 1)
    {
        return 2;
    }

    return 3;
}



void connectRooms(int *roomsArray, int *directionsArray, int size)
{
    int currentRoomId = 0;
    int nextRoom = 0;
    struct room *currentRoomPtr;
    // Print roomsArray
    // printf("Rooms array: ");
    // printArray(roomsArray, size);

    int oppositeDoor = -1;

    int oppositeTable[4] = {1, 0, 3, 2};

    for (int i = 0; i < size - 1; i++)
    {
        currentRoomId = roomsArray[i];
        openDoor(currentRoomId, directionsArray[i]);
        if (oppositeDoor != -1)
        {
            openDoor(currentRoomId, oppositeDoor);
        }
        oppositeDoor = oppositeTable[directionsArray[i]];
        //printf(GREEN "Abriendo puerta %s de la habitacion %d\n" DEFAULT, getCardinalName(directionsArray[i]), currentRoomId);
    }

    currentRoomPtr = getRoomPointerByID(roomsArray[size - 1]);
    currentRoomPtr->doors[oppositeDoor].state = Open;
    currentRoomPtr->openDoorsLeft = currentRoomPtr->openDoorsLeft - 1;

    // getMapDetails();
    // getMapDetails();
    // drawTemporalMap();
}

void getUnvisitedeighbors(int *roomsList, int size, int *unvisitedO, int *unvisitedSizeO, int *unvisitedDirectionsO, int *unvisitedDirectionsSizeO, int goalRoomID)
{
    int neighbor;
    int unvisited[N * N];
    int unvisitedSize = 0;
    int unvisitedDirections[N * N];
    int unvisitedDirectionsSize = 0;
    int oppositeTable[4] = {1, 0, 3, 2};

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            neighbor = getNeighbour(roomsList[i], j);
            if (roomsList[i] != goalRoomID && neighbor != -1 && !isInArray(roomsList, size, neighbor) && !isInArray(unvisited, unvisitedSize, neighbor))
            {
                unvisited[unvisitedSize++] = neighbor;
                unvisitedDirections[unvisitedDirectionsSize++] = oppositeTable[j];
            }
        }
    }

    *unvisitedSizeO = unvisitedSize;
    *unvisitedDirectionsSizeO = unvisitedDirectionsSize;
    memcpy(unvisitedO, unvisited, unvisitedSize * sizeof(int));
    memcpy(unvisitedDirectionsO, unvisitedDirections, unvisitedDirectionsSize * sizeof(int));
}

int openDoor(int roomID, int doorDirection)
{
    struct room *room = getRoomPointerByID(roomID);

    if (room->openDoorsLeft == 0)
    {
        printf("Cant open any more doors in %d\n", roomID);
        return false;
    }

    room->doors[doorDirection].state = Open;
    room->openDoorsLeft = room->openDoorsLeft - 1;

    return true;
}

int closeDoor(int roomID, int doorDirection)
{
    struct room *room = getRoomPointerByID(roomID);

    if (room->openDoorsLeft == 3)
    {
        printf("Cant close anymore doors in %d\n", roomID);
        return false;
    }

    room->doors[doorDirection].state = Closed;
    //room->openDoorsLeft = room->openDoorsLeft + 1;

    // printf("Door opened in room %d, direction %d\n");

    return true;
}

void getClosedDoorsFromRoom(int ID)
{
    struct room *currRoom = getRoomPointerByID(ID);

    for (int i = 0; i < 4; i++)
    {
        if (currRoom->doors[i].state == Closed)
        {
            printf("Door %d is closed in %d\n", i, ID);
        }
    }
}

int getRandomRoomType()
{
    int probability = rand() % 10; // Genera un numero aleatorio entre 0 y 9

    if (probability < 3)
    { // 30% de probabilidad de ser una habitacion normal
        return Normal;
    }
    else if (probability > 6)
    { // 30% de probabilidad de ser una habitacion con tesoro
        return Treasure;
    }
    else
    { // 40% de probabilidad de ser una habitacion con trampa
        return Trap;
    }
}

void createMap(int startRoomID, int total)
{
    int idList[total];
    int id;

    for (int i = 0; i < N; i++)
    {
        game_map[i] = (int *)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++)
        {
            struct room *room = (struct room *)malloc(sizeof(struct room));

            // Calcula un id desde 1,2,3..N*N, con la i y la j
            id = (j + 1) + N * i;

            room->id = (j + 1) + N * i;
            idList[id - 1] = (j + 1) + N * i; // Guarda el id en la lista de ids

            int probability = rand() % 10; // Genera un numero aleatorio entre 0 y 9

            if (probability < 3)
            { // 30% de probabilidad de ser una habitacion normal
                room->type = Normal;
            }
            else if (probability > 6)
            { // 30% de probabilidad de ser una habitacion con tesoro
                room->type = Treasure;
                room->treasure = 1;
            }
            else
            { // 40% de probabilidad de ser una habitacion con trampa
                room->type = Trap;
                room->trap = 1;
            }

            // room->type = room->id == startRoomID ? Start : room->id == goalRoomID ? Goal: room->type;
            room->type = room->id == startRoomID ? Start : Wall;
            createDoors(room->doors);
            room->openDoorsLeft = getMaxNeighbours(room->id); // Cada habitacion puede tener hasta 3 puertas abiertas
            room->discovered = false;
            room->occupied = room->type == Start ? true : false; // La habitacion de inicio siempre esta ocupada, las demas no
            room->treasure = false;
            room->monster = false;

            // game_rooms[id-1] = *room; //Guarda la habitacion en la lista de habitaciones
            game_map[i][j] = *room; // Guarda la habitacion en la matriz (mapa del juego)
        }
    }
}

int generateMap()
{

    if (N < 2)
    {
        printf("N debe ser mayor o igual a 2.\n");
        return -1;
    }

    // -------------------- Modulo de creacion del mapa vacio -------------------------

    int total = N * N;

    game_map = (int **)malloc(N * sizeof(struct room *));

    int startRoomID = N % 2 == 0 ? total / 2 - N / 2 : total / 2 + 1; //(rand() % total) + 1; // Se obtiene una habitacion aleatoria para ser la de inicio
    int goalRoomID;

    createMap(startRoomID, total);

    // -------------------------------------------------------

    // -------------------- Modulo de generacion de un camino al final -------------------------

    int blockedIds[total];
    int blockedIdsSize = 0;

    int touredIds[total];
    int touredIdsSize = 0;

    int connections[total];
    int connectionsSize = 0;

    int possibleConnections[total];
    int possibleConnectionsSize = 0;

    int possibleDestinations[4];
    int possibleDestinationsSize = 0;

    touredIds[touredIdsSize++] = startRoomID;

    int currentNeighbour;
    int currentRoomId;
    int chosenNextRoom;

    int roomCount = N - 1;
    int deadEnds = rand() % (4);

    roomCount = roomCount - deadEnds;

    while (roomCount > 0)
    { // Mientras no se haya llegado a la habitacion final
        currentRoomId = touredIds[touredIdsSize - 1];

        for (int i = 0; i < 4; i++) // Se recorren las 4 posibles direcciones en las que se puede ir
        {
            currentNeighbour = getNeighbour(currentRoomId, i); // Se obtiene el vecino en la direccion i

            if (!isInArray(touredIds, touredIdsSize, currentNeighbour) && !isInArray(blockedIds, blockedIdsSize, currentNeighbour))
            { // Si el vecino no esta en los ids visitados y no esta en los ids bloqueados
                if (currentNeighbour != -1)
                { // Si el vecino existe
                    if (currentRoomId == startRoomID)
                    { // Si el cuarto actual es el inicial
                        if (currentNeighbour != goalRoomID)
                        {                                                                        // Si el vecino no es el cuarto final
                            possibleDestinations[possibleDestinationsSize++] = currentNeighbour; // Se agrega el vecino a los posibles destinos
                            possibleConnections[possibleConnectionsSize++] = i;                  // Se agrega la direccion en la que se puede ir al vecino a las conexiones
                        }
                    }
                    else
                    {
                        possibleDestinations[possibleDestinationsSize++] = currentNeighbour; // Se agrega el vecino a los posibles destinos
                        possibleConnections[possibleConnectionsSize++] = i;                  // Se agrega la direccion en la que se puede ir al vecino a las conexiones
                    }
                }
            }
        }

        if (possibleDestinationsSize == 0)
        {                                                 // Si no hay posibles destinos, se bloquea el cuarto actual
            blockedIds[blockedIdsSize++] = currentRoomId; // Agrega el cuarto a la lista de cuartos bloqueados, ya que no tiene vecinos disponibles
            touredIdsSize--;                              // Se retrocede un cuarto en la lista de cuartos visitados
            connectionsSize--;
            possibleConnectionsSize = 0;  // Se reinician las conexiones
            possibleDestinationsSize = 0; // Se reinicia la lista de posibles destinos
        }
        else
        {
            int random = rand() % possibleDestinationsSize;
            chosenNextRoom = possibleDestinations[random];                // Todos los posibles destinos tienen la misma probabilidad de ser elegidos
            touredIds[touredIdsSize++] = chosenNextRoom;                  // Se agrega el cuarto elegido a la lista de cuartos visitados
            connections[connectionsSize++] = possibleConnections[random]; // Se agrega la direccion en la que se puede ir al cuarto elegido a las conexiones
            possibleDestinationsSize = 0;                                 // Se reinicia la lista de posibles destinos
            possibleConnectionsSize = 0;                                  // Se reinician las conexiones
        }

        struct room *room = getRoomPointerByID(currentRoomId);

        if (room->type != Start)
        {
            room->type = getRandomRoomType();
        }

        roomCount--;
    }

    // -------------------------------------------------------

    // -------------------- Se define un cuarto final -------------------------

    struct room *goalRoom = getRoomPointerByID(touredIds[touredIdsSize - 1]);
    goalRoom->type = Goal;
    goalRoomID = goalRoom->id;

    // ------------------------------------------------------------------------

    // -------------------- Aqui se abren las puertas del laberinto en base al camino descubierto -------------------------

    printf("Abriendo puertas del laberinto...\n\n");

    int camino[total];
    int caminoSize = 0;

    memcpy(camino, touredIds, sizeof(touredIds));
    caminoSize = touredIdsSize;

    connectRooms(touredIds, connections, touredIdsSize);

    
    int possibleDoors[4];
    int possibleDoorsSize = 0;

    for (int i = 0; i < touredIdsSize; i++)
    {
        struct room *currRoom = getRoomPointerByID(touredIds[i]);
        struct room *neighbour;
        int neighbourID;
        int oppositeTable[4] = {1, 0, 3, 2};

        for (int j = 0; j < 4; j++)
        {
            neighbourID = getNeighbour(currRoom->id, j);
            if (neighbourID != -1)
            {
                neighbour = getRoomPointerByID(neighbourID);
                if (currRoom->doors[j].state == Closed && neighbour->type != Wall && neighbour->openDoorsLeft > 0)
                {
                    if (currRoom->type == Start)
                    {
                        if (neighbour->type != Goal)
                        {
                            possibleDoors[possibleDoorsSize++] = j;
                        }
                    }
                    else if (currRoom->type == Goal)
                    {
                        if (neighbour->type != Start)
                        {
                            possibleDoors[possibleDoorsSize++] = j;
                        }
                    }
                    else
                    {
                        possibleDoors[possibleDoorsSize++] = j;
                    }
                }
            }
        }

        
        int chance = rand() % 10;

        if (chance < 8)
        {
            possibleDoorsSize = 0;
        }
        

        for (int j = 0; j < possibleDoorsSize; j++)
        {
            neighbourID = getNeighbour(currRoom->id, possibleDoors[j]);
            if(openDoor(neighbourID, oppositeTable[possibleDoors[j]])){
                openDoor(currRoom->id, possibleDoors[j]);
            }else{
                printf("No se pudo abrir la puerta %d del cuarto %d\n", possibleDoors[j], currRoom->id);
                closeDoor(neighbourID, oppositeTable[possibleDoors[j]]);
            }
            
            //printf("Puerta abierta entre %d y %d\n", currRoom->id, neighbourID);
        }

        possibleDoorsSize = 0;
    }
    

    //drawTemporalMap();

    // -------------------- Modulo de creacion de callejones sin salida -------------------------

    printf("Creando callejones sin salida...\n\n");

    int univisted[total];
    int unvisitedSize = 0;

    int unvisitedDirections[total];
    int unvisitedDirectionsSize = 0;

    struct room *currentRoom;
    struct room *roomToConnect;

    getUnvisitedeighbors(touredIds, touredIdsSize, &univisted, &unvisitedSize, &unvisitedDirections, &unvisitedDirectionsSize, goalRoomID);

    int oppositeTable[4] = {1, 0, 3, 2};

    int roomToConnectId;
    int connectedRoomID;
    int directionToConnect;
    int oppositeDirection;

    int tries = unvisitedDirectionsSize;

    int visitedIndexes[unvisitedSize];
    int visitedIndexesSize = 0;

    int actualDeadEnds = 0;

    int opporunities = 500;

    while (0 < deadEnds || caminoSize < N)
    {
        int random = rand() % unvisitedSize;

        while(isInArray(visitedIndexes, visitedIndexesSize, random))
        {
            random = rand() % unvisitedSize;
        }

        connectedRoomID = univisted[random];
        directionToConnect = unvisitedDirections[random];

        roomToConnectId = getNeighbour(connectedRoomID, directionToConnect);
        oppositeDirection = oppositeTable[directionToConnect];

        currentRoom = getRoomPointerByID(connectedRoomID);
        roomToConnect = getRoomPointerByID(roomToConnectId);

        //printf(DEFAULT "Intentando conectar %d con %d\n" DEFAULT, connectedRoomID, roomToConnectId);
        int res = openDoor(roomToConnectId, oppositeDirection);
        if (res)
        {
            struct room *room = getRoomPointerByID(connectedRoomID);

            camino[caminoSize++] = connectedRoomID;

            room->type = getRandomRoomType();

            openDoor(connectedRoomID, directionToConnect);
            deadEnds--;
            //printf(GREEN "Se conecto %d con %d\n" DEFAULT, connectedRoomID, roomToConnectId);
            actualDeadEnds++;
        }
        else
        {
            //printf(RED "No se pudo conectar %d con %d\n" DEFAULT, connectedRoomID, roomToConnectId);
            // closeDoor(roomToConnectId, oppositeDirection);
            currentRoom->type = Wall;
        }

        visitedIndexes[visitedIndexesSize++] = random;

        if (visitedIndexesSize == unvisitedSize)
        {
            return -1;
        }


    }

    //getMapDetails(); // Con esto pueden obtener los detalles de cada habitacion,
                    // o si quieren con struct room *room = getRoomPointerByID(id); pueden obtener el puntero a la habitacion y acceder a sus atributos
                    // para obtener los datos de una habitacion desde la matriz de habitaciones pueden llamar a struct room *room = &gameMap[x][y]
    

    struct room *startRoom = getRoomPointerByID(startRoomID);
    startRoom->type = getRandomRoomType();
    startRoom->occupied=0;

    int found = 0;
    int d = 0;

    while(!found){
        int connectsToGoal = 0;
        int r = rand() % caminoSize;
        
        if(camino[r]!=goalRoomID){
            for(int j=0; j<4; j++){            
                d = getNeighbour(camino[r], j);
                if(d!=-1){
                    struct room * Nroom = getRoomPointerByID(getNeighbour(camino[r], j));
                    if(Nroom->type == Goal){
                        connectsToGoal = 1;
                    }
                }
            }
            printf("Habitacion %d conecta con la meta? %d\n", camino[r], connectsToGoal);
            if(!connectsToGoal){
                struct room * newStartroom = getRoomPointerByID(camino[r]);
                newStartroom->type = Start;
                newStartroom->occupied=1;
                found = 1;
            }
        }
    }

    drawTemporalMap(); // Con este pueden guiarse para ver como quedo el mapa y compararlo en SDL


    printf("Camino de habitaciones hasta el final (%d):\n", caminoSize-actualDeadEnds);
    printArray(camino, caminoSize-actualDeadEnds);
    printf("Callejones sin salida (%d):\n", actualDeadEnds);
    printArrayFrom(camino, caminoSize-actualDeadEnds, caminoSize);

    printf("Total Rooms: %d\n", caminoSize);

    return 1;
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
    return 0;
}